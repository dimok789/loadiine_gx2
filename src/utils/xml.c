#include <stdio.h>
#include <string.h>
#include "strings.h"
#include "common/kernel_defs.h"
#include "dynamic_libs/fs_functions.h"
#include "dynamic_libs/os_functions.h"
#include "fs/fs_utils.h"
#include "utils.h"
#include "logger.h"

#define XML_BUFFER_SIZE         8192

char * XML_GetNodeText(const char *xml_part, const char * nodename, char * output, int output_size)
{
    // create '<' + nodename
    char buffer[strlen(nodename) + 3];
    buffer[0] = '<';
    strlcpy(&buffer[1], nodename, sizeof(buffer));

    const char *start = strcasestr(xml_part, buffer);
    if(!start)
        return 0;

    // find closing tag
    while(*start && *start != '>')
        start++;
    // skip '>'
    if(*start == '>')
        start++;

    // create '</' + nodename
    buffer[0] = '<';
    buffer[1] = '/';
    strlcpy(&buffer[2], nodename, sizeof(buffer));

    // search for end of string
    const char *end = strcasestr(start, buffer);
    if(!end)
        return 0;

    // copy the stuff in between
    int len = 0;
    while(start < end && len < (output_size-1))
    {
        output[len++] = *start++;
    }

    output[len] = 0;
    return output;
}

int LoadXmlParameters(ReducedCosAppXmlInfo * xmlInfo, const char *rpx_name, const char *path)
{
    //--------------------------------------------------------------------------------------------
    // setup default data
    //--------------------------------------------------------------------------------------------
    memset(xmlInfo, 0, sizeof(ReducedCosAppXmlInfo));
    xmlInfo->version_cos_xml = 18;             // default for most games
    xmlInfo->os_version = 0x000500101000400A;  // default for most games
    xmlInfo->title_id = OSGetTitleID();        // use mii maker ID
    xmlInfo->app_type = 0x80000000;            // default for most games
    xmlInfo->cmdFlags = 0;                     // default for most games
    strlcpy(xmlInfo->rpx_name, rpx_name, sizeof(xmlInfo->rpx_name));
    xmlInfo->max_size = 0x40000000;            // default for most games
    xmlInfo->avail_size = 0;                   // default for most games
    xmlInfo->codegen_size = 0;                 // default for most games
    xmlInfo->codegen_core = 1;                 // default for most games
    xmlInfo->max_codesize = 0x03000000;        // i think this is the best for most games
    xmlInfo->overlay_arena = 0;                // only very few have that set to 1
    xmlInfo->exception_stack0_size = 0x1000;   // default for most games
    xmlInfo->exception_stack1_size = 0x1000;   // default for most games
    xmlInfo->exception_stack2_size = 0x1000;   // default for most games
    xmlInfo->sdk_version = 20909;              // game dependent, lets take the one from mii maker
    xmlInfo->title_version = 0;                // game dependent, we say its 0
    //--------------------------------------------------------------------------------------------
    char* path_copy = (char*)malloc(FS_MAX_MOUNTPATH_SIZE);
    if (!path_copy)
        return -1;

    char* xmlNodeData = (char*)malloc(XML_BUFFER_SIZE);
    if(!xmlNodeData) {
        free(path_copy);
        return -3;
    }

    // create path
    snprintf(path_copy, FS_MAX_MOUNTPATH_SIZE, "%s/cos.xml", path);

    char* xmlData = NULL;
    u32 xmlSize = 0;

    if(LoadFileToMem(path_copy, (u8**) &xmlData, &xmlSize) > 0)
    {
        // ensure 0 termination
        xmlData[XML_BUFFER_SIZE-1] = 0;


        if(XML_GetNodeText(xmlData, "version", xmlNodeData, XML_BUFFER_SIZE))
        {
            uint32_t value = m_strtoll(xmlNodeData, 0, 10);
            xmlInfo->version_cos_xml = value;
        }
        if(XML_GetNodeText(xmlData, "cmdFlags", xmlNodeData, XML_BUFFER_SIZE))
        {
            uint32_t value = m_strtoll(xmlNodeData, 0, 10);
            xmlInfo->cmdFlags = value;
        }
        if(XML_GetNodeText(xmlData, "argstr", xmlNodeData, XML_BUFFER_SIZE))
        {
            // use arguments from xml if rpx name matches with FS
            if (strncasecmp(xmlNodeData, rpx_name, strlen(rpx_name)) == 0)
            {
                strlcpy(xmlInfo->rpx_name, xmlNodeData, sizeof(xmlInfo->rpx_name));
            }
        }
        if(XML_GetNodeText(xmlData, "avail_size", xmlNodeData, XML_BUFFER_SIZE))
        {
            uint32_t value = m_strtoll(xmlNodeData, 0, 16);
            xmlInfo->avail_size = value;
        }
        if(XML_GetNodeText(xmlData, "codegen_size", xmlNodeData, XML_BUFFER_SIZE))
        {
            uint32_t value = m_strtoll(xmlNodeData, 0, 16);
            xmlInfo->codegen_size = value;
        }
        if(XML_GetNodeText(xmlData, "codegen_core", xmlNodeData, XML_BUFFER_SIZE))
        {
            uint32_t value = m_strtoll(xmlNodeData, 0, 16);
            xmlInfo->codegen_core = value;
        }
        if(XML_GetNodeText(xmlData, "max_size", xmlNodeData, XML_BUFFER_SIZE))
        {
            uint32_t value = m_strtoll(xmlNodeData, 0, 16);
            xmlInfo->max_size = value;
        }
        if(XML_GetNodeText(xmlData, "max_codesize", xmlNodeData, XML_BUFFER_SIZE))
        {
            uint32_t value = m_strtoll(xmlNodeData, 0, 16);
            xmlInfo->max_codesize = value;
        }
        if(XML_GetNodeText(xmlData, "overlay_arena", xmlNodeData, XML_BUFFER_SIZE))
        {
            uint32_t value = m_strtoll(xmlNodeData, 0, 16);
            xmlInfo->overlay_arena = value;
        }
        if(XML_GetNodeText(xmlData, "default_stack0_size", xmlNodeData, XML_BUFFER_SIZE))
        {
            uint32_t value = m_strtoll(xmlNodeData, 0, 16);
            xmlInfo->default_stack0_size = value;
        }
        if(XML_GetNodeText(xmlData, "default_stack1_size", xmlNodeData, XML_BUFFER_SIZE))
        {
            uint32_t value = m_strtoll(xmlNodeData, 0, 16);
            xmlInfo->default_stack1_size = value;
        }
        if(XML_GetNodeText(xmlData, "default_stack2_size", xmlNodeData, XML_BUFFER_SIZE))
        {
            uint32_t value = m_strtoll(xmlNodeData, 0, 16);
            xmlInfo->default_stack2_size = value;
        }
        if(XML_GetNodeText(xmlData, "default_redzone0_size", xmlNodeData, XML_BUFFER_SIZE))
        {
            uint32_t value = m_strtoll(xmlNodeData, 0, 16);
            xmlInfo->default_redzone0_size = value;
        }
        if(XML_GetNodeText(xmlData, "default_redzone1_size", xmlNodeData, XML_BUFFER_SIZE))
        {
            uint32_t value = m_strtoll(xmlNodeData, 0, 16);
            xmlInfo->default_redzone1_size = value;
        }
        if(XML_GetNodeText(xmlData, "default_redzone2_size", xmlNodeData, XML_BUFFER_SIZE))
        {
            uint32_t value = m_strtoll(xmlNodeData, 0, 16);
            xmlInfo->default_redzone2_size = value;
        }
        if(XML_GetNodeText(xmlData, "exception_stack0_size", xmlNodeData, XML_BUFFER_SIZE))
        {
            uint32_t value = m_strtoll(xmlNodeData, 0, 16);
            xmlInfo->exception_stack0_size = value;
        }
        if(XML_GetNodeText(xmlData, "exception_stack1_size", xmlNodeData, XML_BUFFER_SIZE))
        {
            uint32_t value = m_strtoll(xmlNodeData, 0, 16);
            xmlInfo->exception_stack0_size = value;
        }
        if(XML_GetNodeText(xmlData, "exception_stack2_size", xmlNodeData, XML_BUFFER_SIZE))
        {
            uint32_t value = m_strtoll(xmlNodeData, 0, 16);
            xmlInfo->exception_stack0_size = value;
        }
    }

    //! free previous XML data memory
    free(xmlData);

    // create path
    snprintf(path_copy, FS_MAX_MOUNTPATH_SIZE, "%s/app.xml", path);

    if(LoadFileToMem(path_copy, (u8**) &xmlData, &xmlSize) > 0)
    {
        // ensure 0 termination
        xmlData[XML_BUFFER_SIZE-1] = 0;

        //--------------------------------------------------------------------------------------------
        // version tag is still unknown where it is used
        //--------------------------------------------------------------------------------------------
        if(XML_GetNodeText(xmlData, "os_version", xmlNodeData, XML_BUFFER_SIZE))
        {
            uint64_t value = m_strtoll(xmlNodeData, 0, 16);
            xmlInfo->os_version = value;
        }
        if(XML_GetNodeText(xmlData, "title_id", xmlNodeData, XML_BUFFER_SIZE))
        {
            uint64_t value = m_strtoll(xmlNodeData, 0, 16);
            xmlInfo->title_id = value;

        }
        if(XML_GetNodeText(xmlData, "title_version", xmlNodeData, XML_BUFFER_SIZE))
        {
            uint32_t value = m_strtoll(xmlNodeData, 0, 16);
            xmlInfo->title_version = value;
        }
        if(XML_GetNodeText(xmlData, "sdk_version", xmlNodeData, XML_BUFFER_SIZE))
        {
            uint32_t value = m_strtoll(xmlNodeData, 0, 10);
            xmlInfo->sdk_version = value;
        }
        if(XML_GetNodeText(xmlData, "app_type", xmlNodeData, XML_BUFFER_SIZE))
        {
            uint32_t value = m_strtoll(xmlNodeData, 0, 16);
            xmlInfo->app_type = value;
        }
        //--------------------------------------------------------------------------------------------
        // group_id tag is still unknown where it is used
        //--------------------------------------------------------------------------------------------
    }

    free(xmlData);
    free(xmlNodeData);
    free(path_copy);

    return 0;
}

int GetId6FromMeta(const char *path, char *id6)
{
    id6[0] = 0;
    char* path_copy = (char*)malloc(FS_MAX_MOUNTPATH_SIZE);
    if (!path_copy)
        return -1;

    char* xmlNodeData = (char*)malloc(XML_BUFFER_SIZE);
    if(!xmlNodeData) {
        free(path_copy);
        return -3;
    }

    // create path
    snprintf(path_copy, FS_MAX_MOUNTPATH_SIZE, "%s/meta.xml", path);

    char* xmlData = NULL;
    u32 xmlSize = 0;

    if(LoadFileToMem(path_copy, (u8**) &xmlData, &xmlSize) > 0)
    {
        // ensure 0 termination
        xmlData[XML_BUFFER_SIZE-1] = 0;

        if(XML_GetNodeText(xmlData, "product_code", xmlNodeData, XML_BUFFER_SIZE) && strlen(xmlNodeData) == 10)
            strncpy(id6, xmlNodeData + 6, 4);
        if(XML_GetNodeText(xmlData, "company_code", xmlNodeData, XML_BUFFER_SIZE) && strlen(xmlNodeData) == 4)
            strncpy(id6 + 4, xmlNodeData + 2, 2);

        id6[6] = 0;
    }

    free(xmlData);
    free(xmlNodeData);
    free(path_copy);

    if(strlen(id6) == 6)
        return 0;
    else
        return -2;
}
