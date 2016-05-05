

void m_DCFlushRange(unsigned int startAddr, unsigned int size)
{
    register unsigned int addr = startAddr & ~0x1F;
    register unsigned int end_addr = startAddr + size;

    while(addr < end_addr)
    {
        asm volatile("dcbf 0, %0" : : "r"(addr));
        addr += 0x20;
    }
    asm volatile("sync; eieio");
}


void m_DCInvalidateRange(unsigned int startAddr, unsigned int size)
{
    register unsigned int addr = startAddr & ~0x1F;
    register unsigned int end_addr = startAddr + size;

    while(addr < end_addr)
    {
        asm volatile("dcbi 0, %0" : : "r"(addr));
        addr += 0x20;
    }
}
