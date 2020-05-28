### 说明

网上的环境没有符号信息不能调试，自己checkout 到漏洞引入的补丁，按照提供的.config进行编译

commit：f1a2e44a3aeccb3ff18d3ccc0b0203e70b95bd92

但是生成的vmlinux找不到iretq 的rop gadget，所以在完成利用代码后，切到网上环境，用网上的环境rop链进行提权

网上环境：https://github.com/ret2p4nda/kernel-pwn/tree/master/linux-bpf-4.20-overflow