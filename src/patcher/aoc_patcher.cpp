#include <stdio.h>
#include "aoc_patcher.h"
#include "common/retain_vars.h"
#include "controller_patcher/cp_retain_vars.h"

DECL(int, ACPGetAddOnUniqueId, unsigned int * id_buffer, int buffer_size)
{
    int result = real_ACPGetAddOnUniqueId(id_buffer, buffer_size);

    if(GAME_LAUNCHED && gEnableDLC)
    {
        id_buffer[0] = (cosAppXmlInfoStruct.title_id >> 8) & 0xffff;
        result = 0;
    }

    return result;
}

DECL(int, AOC_OpenTitle, char * path, void * target, void * buffer, unsigned int buffer_size)
{
    int result  = real_AOC_OpenTitle(path, target, buffer, buffer_size);

    if(GAME_LAUNCHED && gEnableDLC && (result != 0))
    {
        sprintf(path, "/vol/aoc0005000c%08x", (u32)(cosAppXmlInfoStruct.title_id & 0xffffffff));
        result = 0;
    }
    return result;
}

/* *****************************************************************************
 * Creates function pointer array
 * ****************************************************************************/

hooks_magic_t method_hooks_aoc[] __attribute__((section(".data"))) = {
    MAKE_MAGIC(AOC_OpenTitle,               LIB_AOC,DYNAMIC_FUNCTION),
    MAKE_MAGIC(ACPGetAddOnUniqueId,         LIB_NN_ACP,DYNAMIC_FUNCTION),
};


u32 method_hooks_size_aoc __attribute__((section(".data"))) = sizeof(method_hooks_aoc) / sizeof(hooks_magic_t);

//! buffer to store our instructions needed for our replacements
volatile unsigned int method_calls_aoc[sizeof(method_hooks_aoc) / sizeof(hooks_magic_t) * FUNCTION_PATCHER_METHOD_STORE_SIZE] __attribute__((section(".data")));
