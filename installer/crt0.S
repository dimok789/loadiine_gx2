.extern __main
    .globl _start
_start:
    # load proper stack
    lis r1, 0x1ab5
    ori r1, r1, 0xd138
    # jump to our main
    bl __main
