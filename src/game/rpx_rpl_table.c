#include <string.h>
#include "rpx_rpl_table.h"
#include "kernel/kernel_functions.h"
#include "common/common.h"
#include "utils/logger.h"

//! static container holding our retain data
static unsigned char ucRpxData[0xffff];

void rpxRplTableInit(void)
{
    s_rpx_rpl *pRpxData = (s_rpx_rpl*)ucRpxData;
    //! initialize the RPL/RPX table first entry to zero + 1 byte for name zero termination
    //! just in case no RPL/RPX are found, though it wont boot then anyway
    memset(pRpxData, 0, sizeof(s_rpx_rpl) + 1);
}

s_rpx_rpl * rpxRplTableAddEntry(const char *name, int offset, int size, int is_rpx, int entry_index, s_mem_area* area)
{
    // fill rpx/rpl entry
    s_rpx_rpl * rpx_rpl_data = (s_rpx_rpl *)(ucRpxData);
    // get to last entry
    while(rpx_rpl_data->next) {
        rpx_rpl_data = rpx_rpl_data->next;
    }

    // setup next entry on the previous one (only if it is not the first entry)
    if(entry_index > 0) {
        rpx_rpl_data->next = (s_rpx_rpl *)( ((u32)rpx_rpl_data) + sizeof(s_rpx_rpl) + strlen(rpx_rpl_data->name) + 1 );
        rpx_rpl_data = rpx_rpl_data->next;
    }

    // setup current entry
    rpx_rpl_data->area = area;
    rpx_rpl_data->size = size;
    rpx_rpl_data->offset = offset;
    rpx_rpl_data->is_rpx = is_rpx;
    rpx_rpl_data->next = 0;
    strcpy(rpx_rpl_data->name, name);

    log_printf("%s: loaded into 0x%08X, offset: 0x%08X, size: 0x%08X\n", name, area->address, offset, size);
    return rpx_rpl_data;
}

s_rpx_rpl* rpxRplTableGet(void)
{
    return (s_rpx_rpl*)ucRpxData;
}

s_mem_area *rpxRplTableGetNextFreeMemArea(u32 * mem_area_addr_start, u32 * mem_area_addr_end, u32 * mem_area_offset)
{
    s_mem_area * mem_area = memoryGetAreaTable();
    s_rpx_rpl *rpl_struct = rpxRplTableGet();
    while(rpl_struct != 0)
    {
        // check if this entry was loaded into memory
        if(rpl_struct->size == 0) {
            // see if we find entries behind this one that was pre-loaded
            rpl_struct = rpl_struct->next;
            // entry was not loaded into memory -> skip it
            continue;
        }

        // this entry has been loaded to memory, remember it's area
        mem_area = rpl_struct->area;

        int rpl_size = rpl_struct->size;
        int rpl_offset = rpl_struct->offset;
        // find the end of the entry and switch between areas if needed
        while(mem_area && (u32)(rpl_offset + rpl_size) >= mem_area->size)
        {
            rpl_size -= mem_area->size - rpl_offset;
            rpl_offset = 0;
            mem_area = mem_area->next;
        }

        if(!mem_area)
            return NULL;

        // set new start, end and memory area offset
        *mem_area_addr_start = mem_area->address;
        *mem_area_addr_end   = mem_area->address + mem_area->size;
        *mem_area_offset     = rpl_offset + rpl_size;

        // see if we find entries behind this one that was pre-loaded
        rpl_struct = rpl_struct->next;
    }
    return mem_area;
}

int rpxRplCopyDataToMem(s_rpx_rpl *rpx_rpl_struct, u32 fileOffset, const u8 *data, u32 dataSize)
{
    s_mem_area *mem_area = rpx_rpl_struct->area;
    u32 mem_area_addr_start = mem_area->address;
    u32 mem_area_addr_end   = mem_area_addr_start + mem_area->size;
    u32 mem_area_offset     = rpx_rpl_struct->offset;

    // add to offset
    mem_area_offset += fileOffset;

    // skip position to the end of the fill
    while ((mem_area_addr_start + mem_area_offset) >= mem_area_addr_end) // TODO: maybe >, not >=
    {
        // subtract what was in the offset left from last memory block
        mem_area_offset     = (mem_area_addr_start + mem_area_offset) - mem_area_addr_end;
        mem_area            = mem_area->next;
        if(!mem_area)
            return 0;

        mem_area_addr_start = mem_area->address;
        mem_area_addr_end   = mem_area_addr_start + mem_area->size;
    }

    // copy to memory
    u32 copiedBytes = 0;
    while(copiedBytes < dataSize)
    {
        u32 blockSize = dataSize - copiedBytes;
        u32 mem_area_addr_dest = mem_area_addr_start + mem_area_offset;

        if((mem_area_addr_dest + blockSize) > mem_area_addr_end)
            blockSize = mem_area_addr_end - mem_area_addr_dest;

        if(blockSize == 0)
        {
            // Set next memory area
            mem_area            = mem_area->next;
            if(!mem_area)
                return 0;

            mem_area_addr_start = mem_area->address;
            mem_area_addr_end   = mem_area->address + mem_area->size;
            mem_area_offset     = 0;
            continue;
        }

        SC0x25_KernelCopyData(mem_area_addr_dest, (u32)&data[copiedBytes], blockSize);
        mem_area_offset += blockSize;
        copiedBytes += blockSize;
    }

    return copiedBytes;
}

int rpxRplCopyDataFromMem(s_rpx_rpl *rpx_rpl_struct, u32 fileOffset, u8 *data, u32 dataSize)
{
    s_mem_area *mem_area = rpx_rpl_struct->area;
    u32 mem_area_addr_start = mem_area->address;
    u32 mem_area_addr_end   = mem_area_addr_start + mem_area->size;
    u32 mem_area_offset     = rpx_rpl_struct->offset;

    if(fileOffset > rpx_rpl_struct->size)
        return 0;

    if((fileOffset + dataSize) > rpx_rpl_struct->size)
        dataSize = rpx_rpl_struct->size - fileOffset;

    // add to offset
    mem_area_offset += fileOffset;

    // skip position to the end of the fill
    while ((mem_area_addr_start + mem_area_offset) >= mem_area_addr_end) // TODO: maybe >, not >=
    {
        // subtract what was in the offset left from last memory block
        mem_area_offset     = (mem_area_addr_start + mem_area_offset) - mem_area_addr_end;
        mem_area            = mem_area->next;
        if(!mem_area)
            return 0;

        mem_area_addr_start = mem_area->address;
        mem_area_addr_end   = mem_area_addr_start + mem_area->size;
    }

    // copy to memory
    u32 copiedBytes = 0;
    while(copiedBytes < dataSize)
    {
        u32 blockSize = dataSize - copiedBytes;
        u32 mem_area_addr_dest = mem_area_addr_start + mem_area_offset;

        if((mem_area_addr_dest + blockSize) > mem_area_addr_end)
            blockSize = mem_area_addr_end - mem_area_addr_dest;

        if(blockSize == 0)
        {
            // Set next memory area
            mem_area            = mem_area->next;
            if(!mem_area)
                return 0;

            mem_area_addr_start = mem_area->address;
            mem_area_addr_end   = mem_area->address + mem_area->size;
            mem_area_offset     = 0;
            continue;
        }

        SC0x25_KernelCopyData((u32)&data[copiedBytes], mem_area_addr_dest, blockSize);
        mem_area_offset += blockSize;
        copiedBytes += blockSize;
    }

    return copiedBytes;
}
