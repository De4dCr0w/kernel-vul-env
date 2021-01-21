#!/bin/bash

qemu-system-x86_64 \
    -m 512M \
    -kernel ./bzImage \
    -nographic \
    -append 'console=ttyS0 loglevel=3 oops=panic panic=1 kaslr' \
    -initrd ./rootfs.img \
    -monitor /dev/null \
    -smp cores=1,threads=2 \
    -cpu kvm64,smep,smap \
    #-s -S
    #--enable-kvm \

