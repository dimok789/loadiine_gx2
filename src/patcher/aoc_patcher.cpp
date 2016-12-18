#include <stdio.h>
#include <string.h>
#include <string>
#include "aoc_patcher.h"
#include "common/retain_vars.h"
#include "controller_patcher/cp_retain_vars.h"

const char  *aoc_title_dir;
int num_content = 0;

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

DECL(int, AOC_ListTitle, u32 * num_titles, AOC_TitleListType* titles, u32 max_titles, void * buffer, u32 buffer_size)
{
	int result = real_AOC_ListTitle(num_titles, titles, max_titles, buffer, buffer_size);

    if(GAME_LAUNCHED && gEnableDLC)
    {
		*num_titles = strlen(gAoc_Id) / 2;
		aoc_title_dir = gAoc_Id;
		titles->title_ID = 0x0005000c00000000  | (unsigned int)(cosAppXmlInfoStruct.title_id & 0xffffffff);

		result = 0;
    }

    return result;
}

DECL(int, AOC_OpenTitle, char * path, void * target, void * buffer, unsigned int buffer_size)
{
    int result  = real_AOC_OpenTitle(path, target, buffer, buffer_size);
		
    if(GAME_LAUNCHED && gEnableDLC && (result != 0))
    {
		std::string Aoc_id;
		snprintf( path, 23, "/vol/aoc0005000c%08x", (u32)(cosAppXmlInfoStruct.title_id & 0xffffffff));

		for(int i = num_content; i < (num_content + 2) ; i++)
			Aoc_id += aoc_title_dir[i];
		
		strcat(path, Aoc_id.c_str());
		
		aoc_title_dir +=1;
		num_content ++;
		
		result = 0;
    }
	
    return result;
}


DECL(int, AOC_GetPurchaseInfo, u32 * bResult, u64 title_id, u16 contentIndexes[], u32 numberOfContent, void * buffer, u32 buffer_size)
{
    int result  = real_AOC_GetPurchaseInfo(bResult, title_id, contentIndexes, numberOfContent, buffer, buffer_size );
		
	if(GAME_LAUNCHED && gEnableDLC)
    {
		for (u32 i = 0; i < numberOfContent; i++)
		{
			if (bResult != NULL)
				*bResult++ = 1;
		}
		result = 0;
	}
    
    return result;
}

/* *****************************************************************************
 * Creates function pointer array
 * ****************************************************************************/

hooks_magic_t method_hooks_aoc[] __attribute__((section(".data"))) = {
	MAKE_MAGIC(ACPGetAddOnUniqueId,         LIB_NN_ACP,DYNAMIC_FUNCTION),
	MAKE_MAGIC(AOC_ListTitle,               LIB_AOC,DYNAMIC_FUNCTION),
    MAKE_MAGIC(AOC_OpenTitle,               LIB_AOC,DYNAMIC_FUNCTION),
	MAKE_MAGIC(AOC_GetPurchaseInfo,         LIB_AOC,DYNAMIC_FUNCTION),
};


u32 method_hooks_size_aoc __attribute__((section(".data"))) = sizeof(method_hooks_aoc) / sizeof(hooks_magic_t);

//! buffer to store our instructions needed for our replacements
volatile unsigned int method_calls_aoc[sizeof(method_hooks_aoc) / sizeof(hooks_magic_t) * FUNCTION_PATCHER_METHOD_STORE_SIZE] __attribute__((section(".data")));
