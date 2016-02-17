# Syscalls for kernel that we use

     .globl SC0x36_KernelReadDBATs
SC0x36_KernelReadDBATs:
    li r0, 0x3600
    sc
    blr

     .globl SC0x37_KernelWriteDBATs
SC0x37_KernelWriteDBATs:
    li r0, 0x3700
    sc
    blr

     .globl SC0x25_KernelCopyData
SC0x25_KernelCopyData:
    li r0, 0x2500
    sc
    blr
