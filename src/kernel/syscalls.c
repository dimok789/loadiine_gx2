#include "common/os_defs.h"
#include "common/kernel_defs.h"
#include "common/common.h"
#include "dynamic_libs/os_functions.h"
#include "utils/utils.h"
#include "syscalls.h"

extern void my_PrepareTitle_hook(void);

static unsigned int origPrepareTitleInstr __attribute__((section(".data"))) = 0;

static void KernelCopyData(unsigned int addr, unsigned int src, unsigned int len)
{
    /*
     * Setup a DBAT access with cache inhibited to write through and read directly from memory
     */
    unsigned int dbatu0, dbatl0, dbatu1, dbatl1;
    // save the original DBAT value
    asm volatile("mfdbatu %0, 0" : "=r" (dbatu0));
    asm volatile("mfdbatl %0, 0" : "=r" (dbatl0));
    asm volatile("mfdbatu %0, 1" : "=r" (dbatu1));
    asm volatile("mfdbatl %0, 1" : "=r" (dbatl1));

    unsigned int target_dbatu0 = 0;
    unsigned int target_dbatl0 = 0;
    unsigned int target_dbatu1 = 0;
    unsigned int target_dbatl1 = 0;

    unsigned char *dst_p = (unsigned char*)addr;
    unsigned char *src_p = (unsigned char*)src;

    // we only need DBAT modification for addresses out of our own DBAT range
    // as our own DBAT is available everywhere for user and supervisor
    // since our own DBAT is on DBAT5 position we don't collide here
    if(addr < 0x00800000 || addr >= 0x01000000)
    {
        target_dbatu0 = (addr & 0x00F00000) | 0xC0000000 | 0x1F;
        target_dbatl0 = (addr & 0xFFF00000) | 0x32;
        asm volatile("mtdbatu 0, %0" : : "r" (target_dbatu0));
        asm volatile("mtdbatl 0, %0" : : "r" (target_dbatl0));
        dst_p = (unsigned char*)((addr & 0xFFFFFF) | 0xC0000000);
    }
    if(src < 0x00800000 || src >= 0x01000000)
    {
        target_dbatu1 = (src & 0x00F00000) | 0xB0000000 | 0x1F;
        target_dbatl1 = (src & 0xFFF00000) | 0x32;

        asm volatile("mtdbatu 1, %0" : : "r" (target_dbatu1));
        asm volatile("mtdbatl 1, %0" : : "r" (target_dbatl1));
        src_p = (unsigned char*)((src & 0xFFFFFF) | 0xB0000000);
    }

    asm volatile("eieio; isync");

    unsigned int i;
    for(i = 0; i < len; i++)
    {
        // if we are on the edge to next chunk
        if((target_dbatu0 != 0) && (((unsigned int)dst_p & 0x00F00000) != (target_dbatu0 & 0x00F00000)))
        {
            target_dbatu0 = ((addr + i) & 0x00F00000) | 0xC0000000 | 0x1F;
            target_dbatl0 = ((addr + i) & 0xFFF00000) | 0x32;
            dst_p = (unsigned char*)(((addr + i) & 0xFFFFFF) | 0xC0000000);

            asm volatile("eieio; isync");
            asm volatile("mtdbatu 0, %0" : : "r" (target_dbatu0));
            asm volatile("mtdbatl 0, %0" : : "r" (target_dbatl0));
            asm volatile("eieio; isync");
        }
        if((target_dbatu1 != 0) && (((unsigned int)src_p & 0x00F00000) != (target_dbatu1 & 0x00F00000)))
        {
            target_dbatu1 = ((src + i) & 0x00F00000) | 0xB0000000 | 0x1F;
            target_dbatl1 = ((src + i) & 0xFFF00000) | 0x32;
            src_p = (unsigned char*)(((src + i) & 0xFFFFFF) | 0xB0000000);

            asm volatile("eieio; isync");
            asm volatile("mtdbatu 1, %0" : : "r" (target_dbatu1));
            asm volatile("mtdbatl 1, %0" : : "r" (target_dbatl1));
            asm volatile("eieio; isync");
        }

        *dst_p = *src_p;

        ++dst_p;
        ++src_p;
    }

    /*
     * Restore original DBAT value
     */
    asm volatile("eieio; isync");
    asm volatile("mtdbatu 0, %0" : : "r" (dbatu0));
    asm volatile("mtdbatl 0, %0" : : "r" (dbatl0));
    asm volatile("mtdbatu 1, %0" : : "r" (dbatu1));
    asm volatile("mtdbatl 1, %0" : : "r" (dbatl1));
    asm volatile("eieio; isync");
}

static void KernelReadDBATs(bat_table_t * table)
{
    u32 i = 0;

    asm volatile("eieio; isync");

    asm volatile("mfspr %0, 536" : "=r" (table->bat[i].h));
    asm volatile("mfspr %0, 537" : "=r" (table->bat[i].l));
    i++;
    asm volatile("mfspr %0, 538" : "=r" (table->bat[i].h));
    asm volatile("mfspr %0, 539" : "=r" (table->bat[i].l));
    i++;
    asm volatile("mfspr %0, 540" : "=r" (table->bat[i].h));
    asm volatile("mfspr %0, 541" : "=r" (table->bat[i].l));
    i++;
    asm volatile("mfspr %0, 542" : "=r" (table->bat[i].h));
    asm volatile("mfspr %0, 543" : "=r" (table->bat[i].l));
    i++;

    asm volatile("mfspr %0, 568" : "=r" (table->bat[i].h));
    asm volatile("mfspr %0, 569" : "=r" (table->bat[i].l));
    i++;
    asm volatile("mfspr %0, 570" : "=r" (table->bat[i].h));
    asm volatile("mfspr %0, 571" : "=r" (table->bat[i].l));
    i++;
    asm volatile("mfspr %0, 572" : "=r" (table->bat[i].h));
    asm volatile("mfspr %0, 573" : "=r" (table->bat[i].l));
    i++;
    asm volatile("mfspr %0, 574" : "=r" (table->bat[i].h));
    asm volatile("mfspr %0, 575" : "=r" (table->bat[i].l));
}

static void KernelWriteDBATs(bat_table_t * table)
{
    u32 i = 0;

    asm volatile("eieio; isync");

    asm volatile("mtspr 536, %0" : : "r" (table->bat[i].h));
    asm volatile("mtspr 537, %0" : : "r" (table->bat[i].l));
    i++;
    asm volatile("mtspr 538, %0" : : "r" (table->bat[i].h));
    asm volatile("mtspr 539, %0" : : "r" (table->bat[i].l));
    i++;
    asm volatile("mtspr 540, %0" : : "r" (table->bat[i].h));
    asm volatile("mtspr 541, %0" : : "r" (table->bat[i].l));
    i++;
    asm volatile("mtspr 542, %0" : : "r" (table->bat[i].h));
    asm volatile("mtspr 543, %0" : : "r" (table->bat[i].l));
    i++;

    asm volatile("mtspr 568, %0" : : "r" (table->bat[i].h));
    asm volatile("mtspr 569, %0" : : "r" (table->bat[i].l));
    i++;
    asm volatile("mtspr 570, %0" : : "r" (table->bat[i].h));
    asm volatile("mtspr 571, %0" : : "r" (table->bat[i].l));
    i++;
    asm volatile("mtspr 572, %0" : : "r" (table->bat[i].h));
    asm volatile("mtspr 573, %0" : : "r" (table->bat[i].l));
    i++;
    asm volatile("mtspr 574, %0" : : "r" (table->bat[i].h));
    asm volatile("mtspr 575, %0" : : "r" (table->bat[i].l));

    asm volatile("eieio; isync");
}

/* Read a 32-bit word with kernel permissions */
uint32_t __attribute__ ((noinline)) kern_read(const void *addr)
{
	uint32_t result;
	asm volatile (
		"li 3,1\n"
		"li 4,0\n"
		"li 5,0\n"
		"li 6,0\n"
		"li 7,0\n"
		"lis 8,1\n"
		"mr 9,%1\n"
		"li 0,0x3400\n"
		"mr %0,1\n"
		"sc\n"
		"nop\n"
		"mr 1,%0\n"
		"mr %0,3\n"
		:	"=r"(result)
		:	"b"(addr)
		:	"memory", "ctr", "lr", "0", "3", "4", "5", "6", "7", "8", "9", "10",
			"11", "12"
	);

	return result;
}

/* Write a 32-bit word with kernel permissions */
void __attribute__ ((noinline)) kern_write(void *addr, uint32_t value)
{
	asm volatile (
		"li 3,1\n"
		"li 4,0\n"
		"mr 5,%1\n"
		"li 6,0\n"
		"li 7,0\n"
		"lis 8,1\n"
		"mr 9,%0\n"
		"mr %1,1\n"
		"li 0,0x3500\n"
		"sc\n"
		"nop\n"
		"mr 1,%1\n"
		:
		:	"r"(addr), "r"(value)
		:	"memory", "ctr", "lr", "0", "3", "4", "5", "6", "7", "8", "9", "10",
			"11", "12"
		);
}

void KernelSetupSyscalls(void)
{
    //! assign 1 so that this variable gets into the retained .data section
    static uint8_t ucSyscallsSetupRequired = 1;
    if(!ucSyscallsSetupRequired)
        return;

    ucSyscallsSetupRequired = 0;

    kern_write((void*)(OS_SPECIFICS->addr_KernSyscallTbl1 + (0x36 * 4)), (unsigned int)KernelReadDBATs);
    kern_write((void*)(OS_SPECIFICS->addr_KernSyscallTbl2 + (0x36 * 4)), (unsigned int)KernelReadDBATs);
    kern_write((void*)(OS_SPECIFICS->addr_KernSyscallTbl3 + (0x36 * 4)), (unsigned int)KernelReadDBATs);
    kern_write((void*)(OS_SPECIFICS->addr_KernSyscallTbl4 + (0x36 * 4)), (unsigned int)KernelReadDBATs);
    kern_write((void*)(OS_SPECIFICS->addr_KernSyscallTbl5 + (0x36 * 4)), (unsigned int)KernelReadDBATs);

    kern_write((void*)(OS_SPECIFICS->addr_KernSyscallTbl1 + (0x37 * 4)), (unsigned int)KernelWriteDBATs);
    kern_write((void*)(OS_SPECIFICS->addr_KernSyscallTbl2 + (0x37 * 4)), (unsigned int)KernelWriteDBATs);
    kern_write((void*)(OS_SPECIFICS->addr_KernSyscallTbl3 + (0x37 * 4)), (unsigned int)KernelWriteDBATs);
    kern_write((void*)(OS_SPECIFICS->addr_KernSyscallTbl4 + (0x37 * 4)), (unsigned int)KernelWriteDBATs);
    kern_write((void*)(OS_SPECIFICS->addr_KernSyscallTbl5 + (0x37 * 4)), (unsigned int)KernelWriteDBATs);

    kern_write((void*)(OS_SPECIFICS->addr_KernSyscallTbl1 + (0x25 * 4)), (unsigned int)KernelCopyData);
    kern_write((void*)(OS_SPECIFICS->addr_KernSyscallTbl2 + (0x25 * 4)), (unsigned int)KernelCopyData);
    kern_write((void*)(OS_SPECIFICS->addr_KernSyscallTbl3 + (0x25 * 4)), (unsigned int)KernelCopyData);
    kern_write((void*)(OS_SPECIFICS->addr_KernSyscallTbl4 + (0x25 * 4)), (unsigned int)KernelCopyData);
    kern_write((void*)(OS_SPECIFICS->addr_KernSyscallTbl5 + (0x25 * 4)), (unsigned int)KernelCopyData);

    //! write our hook to the
    u32 addr_my_PrepareTitle_hook = ((u32)my_PrepareTitle_hook) | 0x48000003;
    DCFlushRange(&addr_my_PrepareTitle_hook, sizeof(addr_my_PrepareTitle_hook));

    SC0x25_KernelCopyData((u32)&origPrepareTitleInstr, (u32)addr_PrepareTitle_hook, 4);
    SC0x25_KernelCopyData((u32)addr_PrepareTitle_hook, (u32)OSEffectiveToPhysical(&addr_my_PrepareTitle_hook), 4);
}


void KernelRestoreInstructions(void)
{
    if(origPrepareTitleInstr != 0)
        SC0x25_KernelCopyData((u32)addr_PrepareTitle_hook, (u32)&origPrepareTitleInstr, 4);
}
