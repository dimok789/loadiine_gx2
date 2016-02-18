# This stuff may need a change in different kernel versions
# This is only needed when launched directly through browser and not SD card.

.section ".kernel_code"
    .globl SaveAndResetDataBATs_And_SRs_hook
SaveAndResetDataBATs_And_SRs_hook:
    # setup CTR to the position we need to return to
    mflr r5
    mtctr r5
    # set link register to its original value
    mtlr r7
    # setup us a nice DBAT for our code data with same region as our code
    mfspr r5, 560
    mtspr 570, r5
    mfspr r5, 561
    mtspr 571, r5
    # restore the original kernel instructions that we replaced
    lwz r5, 0x34(r3)
    lwz r6, 0x38(r3)
    lwz r7, 0x3C(r3)
    lwz r8, 0x40(r3)
    lwz r9, 0x44(r3)
    lwz r10, 0x48(r3)
    lwz r11, 0x4C(r3)
    lwz r3, 0x50(r3)
    isync
    mtsr 7, r5
    # jump back to the position in kernel after our patch (from LR)
    bctr

.extern my_PrepareTitle
    .globl my_PrepareTitle_hook
my_PrepareTitle_hook:
    # store all registers on stack to avoid issues with the call to C functions
    stwu r1, -0x90(r1)
    # registers for our own usage
    # only need r31 and rest is from tests before, just leaving it for later tests
    stw r28, 0x20(r1)
    stw r29, 0x24(r1)
    stw r30, 0x28(r1)
    stw r31, 0x2C(r1)

    stw r3, 0x30(r1)
    stw r4, 0x34(r1)
    stw r5, 0x38(r1)
    stw r6, 0x3C(r1)
    stw r7, 0x40(r1)
    stw r8, 0x44(r1)
    stw r9, 0x48(r1)
    stw r10, 0x4C(r1)
    stw r11, 0x50(r1)
    stw r12, 0x54(r1)
    stw r13, 0x58(r1)
    stw r14, 0x5C(r1)
    stw r15, 0x60(r1)
    stw r16, 0x64(r1)
    stw r17, 0x68(r1)
    stw r18, 0x6C(r1)
    stw r19, 0x70(r1)
    stw r20, 0x74(r1)
    stw r21, 0x78(r1)
    stw r22, 0x7C(r1)

    # save original DBAT registers
    mfdbatu r28, 0
    mfdbatl r29, 0

    # setup access to our data memory range
    lis r3, 0xC000
    ori r3, r3, 0x1FFF
    mtdbatu 0, r3
    lis r3, 0x3000
    ori r3, r3, 0x0012
    mtdbatl 0, r3

    # memory barrier
    eieio
    isync

    # save the LR from where we came
    mflr r31

    # the cos.xml/app.xml structure is at the location 0x68 of r11
    # there are actually many places that can be hooked for it
    # e.g. 0xFFF16130 and r27 points to this structure
    addi r3, r11, 0x68

    bl my_PrepareTitle

    # restore original DBAT registers
    mtdbatu 0, r28
    mtdbatl 0, r29

    # memory barrier
    eieio
    isync

    # setup LR to jump back to kernel code
    mtlr r31

    # restore all original values of registers from stack
    lwz r28, 0x20(r1)
    lwz r29, 0x24(r1)
    lwz r30, 0x28(r1)
    lwz r31, 0x2C(r1)

    lwz r3, 0x30(r1)
    lwz r4, 0x34(r1)
    lwz r5, 0x38(r1)
    lwz r6, 0x3C(r1)
    lwz r7, 0x40(r1)
    lwz r8, 0x44(r1)
    lwz r9, 0x48(r1)
    lwz r10, 0x4C(r1)
    lwz r11, 0x50(r1)
    lwz r12, 0x54(r1)
    lwz r13, 0x58(r1)
    lwz r14, 0x5C(r1)
    lwz r15, 0x60(r1)
    lwz r16, 0x64(r1)
    lwz r17, 0x68(r1)
    lwz r18, 0x6C(r1)
    lwz r19, 0x70(r1)
    lwz r20, 0x74(r1)
    lwz r21, 0x78(r1)
    lwz r22, 0x7C(r1)

    # restore the stack
    addi r1, r1, 0x90

    # restore original instruction that we replaced in the kernel
    clrlwi    r7, r12, 0

    # jump back
    blr
