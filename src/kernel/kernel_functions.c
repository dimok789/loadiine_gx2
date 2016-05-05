#include <string.h>
#include "common/common.h"
#include "common/kernel_defs.h"
#include "kernel/kernel_functions.h"
#include "kernel/syscalls.h"

/* our retain data */
ReducedCosAppXmlInfo cosAppXmlInfoStruct __attribute__((section(".data")));
/*
 *  This function is a kernel hook function. It is called directly from kernel code at position 0xFFF18558.
 */
void my_PrepareTitle(CosAppXmlInfo *xmlKernelInfo)
{
    /**
     *  DBAT for access to our data region is setup at this point for the 0xC0000000 area.
     */
    // check for Mii Maker RPX or Smash Bros RPX when we started (region independent check)
    if(GAME_LAUNCHED &&
       (   ((strncasecmp("ffl_app.rpx", xmlKernelInfo->rpx_name, FS_MAX_ENTNAME_SIZE) == 0) && (LOADIINE_MODE == LOADIINE_MODE_MII_MAKER))
        || ((strncasecmp("cross_f.rpx", xmlKernelInfo->rpx_name, FS_MAX_ENTNAME_SIZE) == 0) && (LOADIINE_MODE == LOADIINE_MODE_SMASH_BROS))
        || ((strncasecmp("app.rpx", xmlKernelInfo->rpx_name, FS_MAX_ENTNAME_SIZE) == 0) && (LOADIINE_MODE == LOADIINE_MODE_KARAOKE))
        || ((strncasecmp("Treasure.rpx", xmlKernelInfo->rpx_name, FS_MAX_ENTNAME_SIZE) == 0) && (LOADIINE_MODE == LOADIINE_MODE_ART_ATELIER))))
    {
        //! Copy all data from the parsed XML info
        strncpy(xmlKernelInfo->rpx_name, cosAppXmlInfoStruct.rpx_name, FS_MAX_ENTNAME_SIZE);

        // split arguments
        for(unsigned int i = 0; i < strlen(cosAppXmlInfoStruct.rpx_name); i++)
        {
            if (xmlKernelInfo->rpx_name[i] == ' ')
            {
                xmlKernelInfo->rpx_name[i] = 0;
            }
        }

        xmlKernelInfo->version_cos_xml = cosAppXmlInfoStruct.version_cos_xml;
        xmlKernelInfo->os_version = cosAppXmlInfoStruct.os_version;
        xmlKernelInfo->title_id = cosAppXmlInfoStruct.title_id;
        xmlKernelInfo->app_type = cosAppXmlInfoStruct.app_type;
        xmlKernelInfo->cmdFlags = cosAppXmlInfoStruct.cmdFlags;
        xmlKernelInfo->max_size = cosAppXmlInfoStruct.max_size;
        xmlKernelInfo->avail_size = cosAppXmlInfoStruct.avail_size;
        xmlKernelInfo->codegen_size = cosAppXmlInfoStruct.codegen_size;
        xmlKernelInfo->codegen_core = cosAppXmlInfoStruct.codegen_core;
        xmlKernelInfo->max_codesize = cosAppXmlInfoStruct.max_codesize;
        xmlKernelInfo->overlay_arena = cosAppXmlInfoStruct.overlay_arena;
        xmlKernelInfo->default_stack0_size = cosAppXmlInfoStruct.default_stack0_size;
        xmlKernelInfo->default_stack1_size = cosAppXmlInfoStruct.default_stack1_size;
        xmlKernelInfo->default_stack2_size = cosAppXmlInfoStruct.default_stack2_size;
        xmlKernelInfo->default_redzone0_size = cosAppXmlInfoStruct.default_redzone0_size;
        xmlKernelInfo->default_redzone1_size = cosAppXmlInfoStruct.default_redzone1_size;
        xmlKernelInfo->default_redzone2_size = cosAppXmlInfoStruct.default_redzone2_size;
        xmlKernelInfo->exception_stack0_size = cosAppXmlInfoStruct.exception_stack0_size;
        xmlKernelInfo->exception_stack1_size = cosAppXmlInfoStruct.exception_stack1_size;
        xmlKernelInfo->exception_stack2_size = cosAppXmlInfoStruct.exception_stack2_size;
        xmlKernelInfo->sdk_version = cosAppXmlInfoStruct.sdk_version;
        xmlKernelInfo->title_version = cosAppXmlInfoStruct.title_version;
    }
}

void SetupKernelCallback(void)
{
    KernelSetupSyscalls();
}
