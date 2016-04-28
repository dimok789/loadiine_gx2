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
     * Setup a DBAT access for our 0xC0800000 area and our 0xBC000000 area which hold our variables like GAME_LAUNCHED and our BSS/rodata section
     */
    register int dbatu0, dbatl0, dbatu1, dbatl1;
    // Cache firmware version since we might break dbat that gives us access to it
    int os_firmware = OS_FIRMWARE;
    // save the original DBAT value
    asm volatile("mfdbatu %0, 0" : "=r" (dbatu0));
    asm volatile("mfdbatl %0, 0" : "=r" (dbatl0));
    asm volatile("mfdbatu %0, 1" : "=r" (dbatu1));
    asm volatile("mfdbatl %0, 1" : "=r" (dbatl1));

    // write our own DBATs into the array
    if( ((addr & 0xFFF00000) == 0xFFF00000) || ((src & 0xFFF00000) == 0xFFF00000) )
    {
        // setup kernel code access
        unsigned int dbatu = 0;
        unsigned int dbatl = 0;

        if((src & 0xFFF00000) == 0xFFF00000) {
            dbatu = (src & 0xFFF00000) | 0x02;
            dbatl = (src & 0xFFF00000) | 0x32;
        }
        else {
            dbatu = (addr & 0xFFF00000) | 0x02;
            dbatl = (addr & 0xFFF00000) | 0x32;
        }

        if( ((addr & 0xFFF00000) != (dbatu0 & 0xFFF00000)) && ((src & 0xFFF00000) != (dbatu0 & 0xFFF00000)) )
        {
            asm volatile("mtdbatu 0, %0" : : "r" (dbatu));
            asm volatile("mtdbatl 0, %0" : : "r" (dbatl));
        }
        else
        {
            asm volatile("mtdbatu 1, %0" : : "r" (dbatu));
            asm volatile("mtdbatl 1, %0" : : "r" (dbatl));
        }
    }
    else
    {
        unsigned int dst_dbatu = 0;
        unsigned int dst_dbatl = 0;
        unsigned int src_dbatu = 0;
        unsigned int src_dbatl = 0;
        // Since we apply different operations, but never use multiple-regions ranges
        // it's best to setup one dbat for source and second for the destination depending on their addresses.
        // But we should not create two dbats with the same mapping.
        if ((addr >= 0xB8000000) && (addr < 0xC0000000)) // destination address is from home menu region
        {
            dst_dbatu = 0xB8000FFF;
            dst_dbatl = 0x28000012;
        }
        else if ((addr >= 0xC0000000) && (addr < 0xC2000000)) // destination address is from root.rpx region
        {
            dst_dbatu = 0xC00003FF;
            if (os_firmware >= 410)
                dst_dbatl = 0x30000012;
            else
                dst_dbatl = 0x4E000012;
        }
        else if ((addr >= 0xC2000000) && (addr < 0xC3000000)) // destination address is from coreinit.rpl region
        {
            dst_dbatu = 0xC20001FF;
            if (os_firmware >= 410)
                dst_dbatl = 0x32000012;
            else
                dst_dbatl = 0x4D000012;
        }
        if ((src >= 0xB8000000) && (src < 0xC0000000)) // source address is from home menu region
        {
            src_dbatu = 0xB8000FFF;
            src_dbatl = 0x28000012;
        }
        else if ((src >= 0xC0000000) && (src < 0xC2000000)) // source address is from root.rpx region
        {
            src_dbatu = 0xC00003FF;
            if (os_firmware >= 410)
                src_dbatl = 0x30000012;
            else
                src_dbatl = 0x4E000012;
        }
        else if ((src >= 0xC2000000) && (src < 0xC3000000)) // source address is from coreinit.rpl region
        {
            src_dbatu = 0xC20001FF;
            if (os_firmware >= 410)
                src_dbatl = 0x32000012;
            else
                src_dbatl = 0x4D000012;
        }
        asm volatile("mtdbatu 0, %0" : : "r" (dst_dbatu));
        asm volatile("mtdbatl 0, %0" : : "r" (dst_dbatl));
        if (dst_dbatu != src_dbatu) // Setup second dbat only if virtual range is different
        {
            asm volatile("mtdbatu 1, %0" : : "r" (src_dbatu));
            asm volatile("mtdbatl 1, %0" : : "r" (src_dbatl));
        }
    }
    asm volatile("eieio; isync");


    unsigned char *src_p = (unsigned char*)src;
    unsigned char *dst_p = (unsigned char*)addr;

    unsigned int i;
    for(i = 0; i < len; i++)
    {
        dst_p[i] = src_p[i];
    }

    /*
     * Restore original DBAT value
     */
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

    SC0x25_KernelCopyData((u32)&origPrepareTitleInstr, (u32)addr_PrepareTitle_hook, 4);
    SC0x25_KernelCopyData((u32)addr_PrepareTitle_hook, (u32)&addr_my_PrepareTitle_hook, 4);
}


void KernelRestoreInstructions(void)
{
    if(origPrepareTitleInstr != 0)
        SC0x25_KernelCopyData((u32)addr_PrepareTitle_hook, (u32)&origPrepareTitleInstr, 4);
}
