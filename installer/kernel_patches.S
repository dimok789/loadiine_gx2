#if ((VER == 532) || (VER == 540))
#define BAT_SETUP_HOOK_ADDR         0xFFF1D638
#define PREP_TITLE_HOOK_ADDR        0xFFF18558
#define BAT_SET_NOP_ADDR_1          0xFFF06A14
#define BAT_SET_NOP_ADDR_2          0xFFF06AA0
#define BAT_SET_NOP_ADDR_3          0xFFF003C8
#define BAT_SET_NOP_ADDR_4          0xFFF003CC
#elif ((VER == 500) || (VER == 510))
    #define BAT_SETUP_HOOK_ADDR	0xFFF1D518
    #define PREP_TITLE_HOOK_ADDR    0xFFF18534
#elif VER == 410
    #define BAT_SETUP_HOOK_ADDR	0xFFF1AD00
    #define PREP_TITLE_HOOK_ADDR    0xFFF166DC
#else
    #error Please define valid values for kernel setup.
#endif

#ifdef USE_SD_LOADER
    #define BAT_SETUP_HOOK_ENTRY    0x00800000
#else
    #define BAT_SETUP_HOOK_ENTRY    (0x00800000 + 0x2000)
#endif

#define PREP_TITLE_HOOK_ENTRY       (BAT_SETUP_HOOK_ENTRY + 0x38)
#define BAT4U_VAL                   0x008000FF
#define BAT4L_VAL                   0x30800012

#define SET_R4_TO_ADDR(addr)        \
    lis r3, addr@h              ;   \
    ori r3, r3, addr@l          ;   \
    stw r4, 0(r3)               ;   \
    dcbf 0, r3                  ;   \
    icbi 0, r3                  ;

     .globl Syscall_0x36
Syscall_0x36:
    mflr r0
    stwu r1, -0x10(r1)
    stw r30, 0x4(r1)
    stw r31, 0x8(r1)
    mr r5, r0
    mr r6, r1
    li r0, 0x3600
    sc
    nop
    mr r0, r5
    mr r1, r6
    lwz r30, 0x04(r1)
    lwz r31, 0x08(r1)
    addi r1, r1, 0x10
    mtlr r0
    blr

    .globl KernelPatches
KernelPatches:
    # store the old DBAT0
    mfdbatu r30, 0
    mfdbatl r31, 0

    # memory barrier
    eieio
    isync

    # setup DBAT0 for access to kernel code memory
    lis r3, 0xFFF0
    ori r3, r3, 0x0002
    mtdbatu 0, r3
    lis r3, 0xFFF0
    ori r3, r3, 0x0032
    mtdbatl 0, r3

    # memory barrier
    eieio
    isync

    # SaveAndResetDataBATs_And_SRs hook setup, but could be any BAT function though
    # just chosen because its simple
    lis r3, BAT_SETUP_HOOK_ADDR@h
    ori r3, r3, BAT_SETUP_HOOK_ADDR@l

    # make the kernel setup our section in IBAT4 and
    # jump to our function to restore the replaced instructions
    lis r4, 0x3ce0      				#   lis r7, BAT4L_VAL@h
    ori r4, r4, BAT4L_VAL@h
    stw r4, 0x00(r3)
    lis r4, 0x60e7      				#   ori r7, r7, BAT4L_VAL@l
    ori r4, r4, BAT4L_VAL@l
    stw r4, 0x04(r3)
    lis r4, 0x7cf1      				#   mtspr 561, r7
    ori r4, r4, 0x8ba6
    stw r4, 0x08(r3)
    lis r4, 0x3ce0      				#   lis r7, BAT4U_VAL@h
    ori r4, r4, BAT4U_VAL@h
    stw r4, 0x0C(r3)
    lis r4, 0x60e7      				#   ori r7, r7, BAT4U_VAL@l
    ori r4, r4, BAT4U_VAL@l
    stw r4, 0x10(r3)
    lis r4, 0x7cf0      				#   mtspr 560, r7
    ori r4, r4, 0x8ba6
    stw r4, 0x14(r3)
    lis r4, 0x7c00      				#   eieio
    ori r4, r4, 0x06ac
    stw r4, 0x18(r3)
    lis r4, 0x4c00      				#   isync
    ori r4, r4, 0x012c
    stw r4, 0x1C(r3)
    lis r4, 0x7ce8     				    #   mflr r7
    ori r4, r4, 0x02a6
    stw r4, 0x20(r3)
    lis r4, (BAT_SETUP_HOOK_ENTRY | 0x48000003)@h      #   bla BAT_SETUP_HOOK_ENTRY
    ori r4, r4, (BAT_SETUP_HOOK_ENTRY | 0x48000003)@l
    stw r4, 0x24(r3)

    # flush and invalidate the replaced instructions
    lis r3, (BAT_SETUP_HOOK_ADDR & ~31)@h
    ori r3, r3, (BAT_SETUP_HOOK_ADDR & ~31)@l
    dcbf 0, r3
    icbi 0, r3
    lis r3, ((BAT_SETUP_HOOK_ADDR + 0x20) & ~31)@h
    ori r3, r3, ((BAT_SETUP_HOOK_ADDR + 0x20) & ~31)@l
    dcbf 0, r3
    icbi 0, r3
    sync

    # setup IBAT4 for core 1 at this position (not really required but wont hurt)
    # IBATL 4
    lis r3, BAT4L_VAL@h
    ori r3, r3, BAT4L_VAL@l
    mtspr 561, r3

    # IBATU 4
    lis r3, BAT4U_VAL@h
    ori r3, r3, BAT4U_VAL@l
    mtspr 560, r3

    # memory barrier
    eieio
    isync

    # Now lets hook the kernel function for result printing of PrepareTitle add jump to our function
    lis r4, (PREP_TITLE_HOOK_ENTRY | 0x48000003)@h
    ori r4, r4, (PREP_TITLE_HOOK_ENTRY | 0x48000003)@l
    SET_R4_TO_ADDR(PREP_TITLE_HOOK_ADDR)

    # write "nop" to some positions
    lis r4, 0x6000
    # nop on IBATU 4 and DBAT 5 set/reset
    SET_R4_TO_ADDR(BAT_SET_NOP_ADDR_1)
    SET_R4_TO_ADDR(BAT_SET_NOP_ADDR_2)
    SET_R4_TO_ADDR(BAT_SET_NOP_ADDR_3)
    SET_R4_TO_ADDR(BAT_SET_NOP_ADDR_4)

    # memory barrier
    eieio
    isync

    # restore DBAT 0 and return from interrupt
    mtdbatu 0, r30
    mtdbatl 0, r31

    # memory barrier
    eieio
    isync

    rfi

