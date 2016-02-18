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
