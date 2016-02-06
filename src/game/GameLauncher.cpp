#include <algorithm>
#include <string>
#include <string.h>
#include <fcntl.h>

#include <zlib.h>
#include "fs/fs_utils.h"
#include "settings/CSettings.h"
#include "GameLauncher.h"
#include "fs/CFile.hpp"
#include "fs/DirList.h"
#include "utils/logger.h"
#include "utils/xml.h"

/* RPX stuff */
#define RPX_SH_STRNDX_OFFSET            0x0032
#define RPX_SHT_START                   0x0040
#define RPX_SHT_ENTRY_SIZE              0x28

#define RPX_SHDR_FLAGS_OFFSET           0x08
#define RPX_SHDR_OFFSET_OFFSET          0x10
#define RPX_SHDR_SIZE_OFFSET            0x14

#define RPX_SHDR_ZLIB_FLAG              0x08000000

/* global variable for CosAppXml struct that is forced to data section */
extern ReducedCosAppXmlInfo cosAppXmlInfoStruct;

typedef struct _asynch_params_t
{
    const discHeader *hdr;
    sigslot::signal2<const discHeader *, int> * asyncLoadFinished;
} asynch_params_t;

CThread * GameLauncher::loadGameToMemoryAsync(const discHeader *header, sigslot::signal2<const discHeader *, int> * asyncLoadFinished)
{
    asynch_params_t *params = new asynch_params_t;
    params->hdr = header;
    params->asyncLoadFinished = asyncLoadFinished;

    CThread * thread = CThread::create(gameLoadCallback, (void*)params, CThread::eAttributeAffCore1);
    thread->resumeThread();
    return thread;
}

void GameLauncher::gameLoadCallback(CThread *thread, void *arg)
{
    asynch_params_t *params = (asynch_params_t*)arg;

    const discHeader *header = params->hdr;
    int result = loadGameToMemory(header);

    //! emit signal and disconnected all connected listener
    if(params->asyncLoadFinished)
    {
        sigslot::signal2<const discHeader *, int> & asyncLoadFinished = *params->asyncLoadFinished;
        asyncLoadFinished(header, result);
    }

    delete params;
}

int GameLauncher::loadGameToMemory(const discHeader *header)
{
    if(!header)
        return INVALID_INPUT;

    //! initialize the RPL/RPX table first entry to zero + 1 byte for name zero termination
    //! just in case no RPL/RPX are found, though it wont boot then anyway
    memset(RPX_RPL_ARRAY, 0, sizeof(s_rpx_rpl) + 1);

	DirList rpxList(header->gamepath + RPX_RPL_PATH, ".rpx", DirList::Files);

	if(rpxList.GetFilecount() == 0)
    {
        log_printf("RPX file not found!\n");
        return RPX_NOT_FOUND;
    }
	if(rpxList.GetFilecount() != 1)
    {
        log_printf("Warning: Too many RPX files in the folder! Found %i files! Using first one.\n", rpxList.GetFilecount());
        //return TOO_MANY_RPX_NOT_FOUND;
    }

    u32 entryIndex = 0;
    std::string rpxName;
    std::vector<std::string> rplImportList;

    DirList rplList(header->gamepath + RPX_RPL_PATH, ".rpl", DirList::Files);

    int result = LoadRpxRplToMem(rpxList.GetFilepath(0), rpxList.GetFilename(0), true, entryIndex++, rplImportList);
    if(result < 0)
    {
        log_printf("Failed loading RPX file %s, error %i\n", rpxList.GetFilepath(0), result);
        return result;
    }

    rpxName = rpxList.GetFilename(0);

    //! get all imports from the RPX
    GetRpxImports((s_rpx_rpl *)(RPX_RPL_ARRAY), rplImportList);

    for(int i = 0; i < rplList.GetFilecount(); i++)
    {
        result = LoadRpxRplToMem(rplList.GetFilepath(i), rplList.GetFilename(i), false, entryIndex++, rplImportList);
        if(result < 0)
        {
            log_printf("Failed loading RPL file %s, error %i\n", rplList.GetFilepath(0), result);
            return result;
        }
    }

    //! TODO: clean this path creation up
    std::string game_dir = header->gamepath;
    size_t pos = game_dir.rfind('/');

    if(pos != std::string::npos)
        game_dir = game_dir.substr(pos + 1);

    //! set the save game path from the gamenames path
    std::string saveGamePath = CSettings::getValueAsString(CSettings::GameSavePath) + "/" + game_dir;
    std::string saveGamePathCommon;
    std::string saveGamePathUser;

    if(CSettings::getValueAsU8(CSettings::GameSaveMode) == GAME_SAVES_SHARED)
    {
        saveGamePathUser = "u";
        saveGamePathCommon = "c";
    }
    else
    {
        /* get persistent ID - thanks to Maschell */
        unsigned int nn_act_handle;
        unsigned long (*GetPersistentIdEx)(unsigned char);
        int (*GetSlotNo)(void);
        void (*nn_Initialize)(void);
        void (*nn_Finalize)(void);
        OSDynLoad_Acquire("nn_act.rpl", &nn_act_handle);
        OSDynLoad_FindExport(nn_act_handle, 0, "GetPersistentIdEx__Q2_2nn3actFUc", &GetPersistentIdEx);
        OSDynLoad_FindExport(nn_act_handle, 0, "GetSlotNo__Q2_2nn3actFv", &GetSlotNo);
        OSDynLoad_FindExport(nn_act_handle, 0, "Initialize__Q2_2nn3actFv", &nn_Initialize);
        OSDynLoad_FindExport(nn_act_handle, 0, "Finalize__Q2_2nn3actFv", &nn_Finalize);

        nn_Initialize(); // To be sure that it is really Initialized

        unsigned char slotno = GetSlotNo();
        unsigned int persistentID = GetPersistentIdEx(slotno);
        nn_Finalize(); //must be called an equal number of times to nn_Initialize

        char persistentIdString[10];
        snprintf(persistentIdString, sizeof(persistentIdString), "%08X", persistentID);

        saveGamePathUser = persistentIdString;
        saveGamePathCommon = "common";
    }

    CreateSubfolder((saveGamePath + "/" + saveGamePathUser).c_str());
    CreateSubfolder((saveGamePath + "/" + saveGamePathCommon).c_str());

    std::string tempPath = CSettings::getValueAsString(CSettings::GamePath);
    //! remove "sd:" and replace with "/vol/external01"
    pos = tempPath.find('/');
    if(pos != std::string::npos)
        tempPath = std::string(CAFE_OS_SD_PATH) + tempPath.substr(pos);

    game_paths_t *game_paths = (game_paths_t *)GAME_PATH_STRUCT;

    //! set pointer to firth path and copy it
    game_paths->os_game_path_base = (char*)(game_paths + 1);
    strcpy(game_paths->os_game_path_base, tempPath.c_str());

    //! set pointer to next path and copy it
    game_paths->os_save_path_base = game_paths->os_game_path_base + tempPath.size() + 1;

    tempPath = CSettings::getValueAsString(CSettings::GameSavePath);
    //! remove "sd:" and replace with "/vol/external01"
    pos = tempPath.find('/');
    if(pos != std::string::npos)
        tempPath = std::string(CAFE_OS_SD_PATH) + tempPath.substr(pos);

    strcpy(game_paths->os_save_path_base, tempPath.c_str());

    //! set pointer to next path and copy it
    game_paths->game_dir = game_paths->os_save_path_base + tempPath.size() + 1;
    strcpy(game_paths->game_dir, game_dir.c_str());

    //! set pointer to next path and copy it
    game_paths->save_dir_common = game_paths->game_dir + game_dir.size() + 1;
    strcpy(game_paths->save_dir_common, saveGamePathCommon.c_str());

    //! set pointer to next path and copy it
    game_paths->save_dir_user = game_paths->save_dir_common + saveGamePathCommon.size() + 1;
    strcpy(game_paths->save_dir_user, saveGamePathUser.c_str());

    log_printf("game_paths->os_game_path_base: %s\n", game_paths->os_game_path_base);
    log_printf("game_paths->os_save_path_base: %s\n", game_paths->os_save_path_base);
    log_printf("game_paths->game_dir:          %s\n", game_paths->game_dir);
    log_printf("game_paths->save_dir_common:   %s\n", game_paths->save_dir_common);
    log_printf("game_paths->save_dir_user:     %s\n", game_paths->save_dir_user);

    LoadXmlParameters(&cosAppXmlInfoStruct, rpxName.c_str(), (header->gamepath + RPX_RPL_PATH).c_str());

    DCFlushRange((void*)&cosAppXmlInfoStruct, sizeof(ReducedCosAppXmlInfo));

    log_printf("XML Launching Parameters\n");
    log_printf("rpx_name:        %s\n", cosAppXmlInfoStruct.rpx_name);
    log_printf("version_cos_xml: %i\n", cosAppXmlInfoStruct.version_cos_xml);
    log_printf("os_version:      %08X%08X\n", (unsigned int)(cosAppXmlInfoStruct.os_version >> 32), (unsigned int)(cosAppXmlInfoStruct.os_version & 0xFFFFFFFF));
    log_printf("title_id:        %08X%08X\n", (unsigned int)(cosAppXmlInfoStruct.title_id >> 32), (unsigned int)(cosAppXmlInfoStruct.title_id & 0xFFFFFFFF));
    log_printf("app_type:        %08X\n", cosAppXmlInfoStruct.app_type);
    log_printf("cmdFlags:        %08X\n", cosAppXmlInfoStruct.cmdFlags);
    log_printf("max_size:        %08X\n", cosAppXmlInfoStruct.max_size);
    log_printf("avail_size:      %08X\n", cosAppXmlInfoStruct.avail_size);
    log_printf("codegen_size:    %08X\n", cosAppXmlInfoStruct.codegen_size);
    log_printf("codegen_core:    %08X\n", cosAppXmlInfoStruct.codegen_core);
    log_printf("max_codesize:    %08X\n", cosAppXmlInfoStruct.max_codesize);
    log_printf("overlay_arena:   %08X\n", cosAppXmlInfoStruct.overlay_arena);
    log_printf("sdk_version:     %i\n", cosAppXmlInfoStruct.sdk_version);
    log_printf("title_version:   %08X\n", cosAppXmlInfoStruct.title_version);

    return 0;
}

void GameLauncher::Add_RPX_RPL_Entry(const char *name, int offset, int size, int is_rpx, int entry_index, s_mem_area* area)
{
    // fill rpx/rpl entry
    s_rpx_rpl * rpx_rpl_data = (s_rpx_rpl *)(RPX_RPL_ARRAY);
    // get to last entry
    while(rpx_rpl_data->next) {
        rpx_rpl_data = rpx_rpl_data->next;
    }

    // setup next entry on the previous one (only if it is not the first entry)
    if(entry_index > 0) {
        rpx_rpl_data->next = (s_rpx_rpl *)( ((unsigned int)rpx_rpl_data) + sizeof(s_rpx_rpl) + strlen(rpx_rpl_data->name) + 1 );
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
}


int GameLauncher::LoadRpxRplToMem(const std::string & path, const std::string & name, bool isRPX, int entryIndex, const std::vector<std::string> & rplImportList)
{
    // For RPLs :
    if(!isRPX)
    {
        int preload = 0;
        u32 i;
        for(i = 0; i < rplImportList.size(); i++)
        {
            if(strncasecmp(name.c_str(), rplImportList[i].c_str(), name.size() - 4) == 0)
            {
                // file is in the fimports section and therefore needs to be preloaded
                preload = 1;
                break;
            }
        }
        // if we dont need to preload, just add it to the array
        if(!preload)
        {
            // fill rpl entry
            Add_RPX_RPL_Entry(name.c_str(), 0, 0, isRPX, entryIndex, (s_mem_area*)(MEM_AREA_ARRAY));
            return 1;
        }
        log_printf("Pre-loading RPL %s because its in the fimport section\n", name.c_str());
    }

    CFile file(path, CFile::ReadOnly);
    if(!file.isOpen())
        return FILE_OPEN_FAILURE;

    u32 fileSize = file.size();

    // this is the initial area
    s_mem_area* mem_area    = (s_mem_area*)(MEM_AREA_ARRAY);
    int mem_area_addr_start = mem_area->address;
    int mem_area_addr_end   = mem_area->address + mem_area->size;
    int mem_area_offset     = 0;

    // on RPLs we need to find the free area we can store data to (at least RPX was already loaded by this point)
    if(!isRPX)
    {
        s_rpx_rpl *rpl_struct = (s_rpx_rpl*)(RPX_RPL_ARRAY);
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
            while((u32)(rpl_offset + rpl_size) >= mem_area->size)
            {
                rpl_size -= mem_area->size - rpl_offset;
                rpl_offset = 0;
                mem_area = mem_area->next;
            }

            // set new start, end and memory area offset
            mem_area_addr_start = mem_area->address;
            mem_area_addr_end   = mem_area->address + mem_area->size;
            mem_area_offset     = rpl_offset + rpl_size;

            // see if we find entries behind this one that was pre-loaded
            rpl_struct = rpl_struct->next;
        }
    }

    // malloc mem for read file
    std::string strBuffer;
    strBuffer.resize(0x1000);
    unsigned char *pBuffer = (unsigned char*)&strBuffer[0];

    // Get current memory area limits
    s_mem_area* mem_area_rpl_start = mem_area;
    int mem_area_offset_rpl_start = mem_area_offset;

    u32 bytesRead = 0;

    // Copy rpl in memory
    while(bytesRead < fileSize)
    {
        u32 blockSize = strBuffer.size();
        if(blockSize > (fileSize - bytesRead))
            blockSize = fileSize - bytesRead;

        int ret = file.read(pBuffer, blockSize);
        if(ret <= 0)
        {
            log_printf("Failure on reading file %s\n", path.c_str());
            break;
        }

        // Copy in memory and save offset
        for (int j = 0; j < ret; j++)
        {
            if ((mem_area_addr_start + mem_area_offset) >= mem_area_addr_end)
            {
                // Set next memory area
                mem_area            = mem_area->next;
                if(!mem_area)
                {
                    log_printf("Not enough memory for file %s\n", path.c_str());
                    return NOT_ENOUGH_MEMORY;
                }
                mem_area_addr_start = mem_area->address;
                mem_area_addr_end   = mem_area->address + mem_area->size;
                mem_area_offset     = 0;
            }
            *(volatile unsigned char*)(mem_area_addr_start + mem_area_offset) = pBuffer[j];
            mem_area_offset++;
        }
        bytesRead += ret;
    }

    if(bytesRead != fileSize)
    {
        log_printf("File loading not finished for file %s, finished %i of %i bytes\n", path.c_str(), bytesRead, fileSize);
        return FILE_READ_ERROR;
    }

    // remember which RPX has to be checked for on loader for allowing the list compare
    if(isRPX) {
        RPX_CHECK_NAME = *(unsigned int*)name.c_str();
    }

    // fill rpx entry
    Add_RPX_RPL_Entry(name.c_str(), mem_area_offset_rpl_start, fileSize, isRPX, entryIndex, mem_area_rpl_start);

    // return okay
    return 0;
}

static int GetMemorySegment(s_rpx_rpl * rpx_data, unsigned int offset, unsigned int size, unsigned char *buffer)
{
    s_mem_area *mem_area = rpx_data->area;
    int mem_area_addr_start = mem_area->address;
    int mem_area_addr_end   = mem_area_addr_start + mem_area->size;
    int mem_area_offset     = rpx_data->offset;

    unsigned int buffer_position = 0;

    // Replace rpx/rpl data
    for (unsigned int position = 0; position < rpx_data->size; position++)
    {
        if ((mem_area_addr_start + mem_area_offset) >= mem_area_addr_end) // TODO: maybe >, not >=
        {
            mem_area            = mem_area->next;
            if(!mem_area) {
                return buffer_position;
            }
            mem_area_addr_start = mem_area->address;
            mem_area_addr_end   = mem_area_addr_start + mem_area->size;
            mem_area_offset     = 0;
        }
        if(position >= offset) {
            buffer[buffer_position] = *(unsigned char *)(mem_area_addr_start + mem_area_offset);
            buffer_position++;
        }
        if(buffer_position >= size) {
            return buffer_position;
        }
        mem_area_offset++;
    }

    return 0;
}

void GameLauncher::GetRpxImports(s_rpx_rpl *rpx_data, std::vector<std::string> & rplImports)
{
    std::string strBuffer;
    strBuffer.resize(0x1000);

    // get the header information of the RPX
    if(!GetMemorySegment(rpx_data, 0, 0x1000, (unsigned char *)&strBuffer[0]))
        return;

    // Who needs error checks...
    // Get section number
    int shstrndx = *(unsigned short*)(&strBuffer[RPX_SH_STRNDX_OFFSET]);
    // Get section offset
    int section_offset = *(unsigned int*)(&strBuffer[RPX_SHT_START + (shstrndx * RPX_SHT_ENTRY_SIZE) + RPX_SHDR_OFFSET_OFFSET]);
    // Get section size
    int section_size = *(unsigned int*)(&strBuffer[RPX_SHT_START + (shstrndx * RPX_SHT_ENTRY_SIZE) + RPX_SHDR_SIZE_OFFSET]);
    // Get section flags
    int section_flags = *(unsigned int*)(&strBuffer[RPX_SHT_START + (shstrndx * RPX_SHT_ENTRY_SIZE) + RPX_SHDR_FLAGS_OFFSET]);

    // Align read to 64 for SD access (section offset already aligned to 64 @ elf/rpx?!)
    int section_offset_aligned = (section_offset / 64) * 64;
    int section_alignment = section_offset - section_offset_aligned;
    int section_size_aligned = ((section_alignment + section_size) / 64) * 64 + 64;

    std::string section_data;
    section_data.resize(section_size_aligned);

    // get the header information of the RPX
    if(!GetMemorySegment(rpx_data, section_offset_aligned, section_size_aligned, (unsigned char *)&section_data[0]))
        return;

    //Check if inflate is needed (ZLIB flag)
    if (section_flags & RPX_SHDR_ZLIB_FLAG)
    {
        // Section is compressed, inflate
        int section_size_inflated = *(unsigned int*)(&section_data[section_alignment]);
        std::string inflatedData;
        inflatedData.resize(section_size_inflated);

        unsigned int zlib_handle;
        OSDynLoad_Acquire("zlib125", &zlib_handle);

        /* Zlib functions */
        int(*ZinflateInit_)(z_streamp strm, const char *version, int stream_size);
        int(*Zinflate)(z_streamp strm, int flush);
        int(*ZinflateEnd)(z_streamp strm);

        OSDynLoad_FindExport(zlib_handle, 0, "inflateInit_", &ZinflateInit_);
        OSDynLoad_FindExport(zlib_handle, 0, "inflate", &Zinflate);
        OSDynLoad_FindExport(zlib_handle, 0, "inflateEnd", &ZinflateEnd);

        int ret = 0;
        z_stream s;
        memset(&s, 0, sizeof(s));

        s.zalloc = Z_NULL;
        s.zfree = Z_NULL;
        s.opaque = Z_NULL;

        ret = ZinflateInit_(&s, ZLIB_VERSION, sizeof(s));
        if (ret != Z_OK)
            return;

        s.avail_in = section_size - 0x04;
        s.next_in = (Bytef *)(&section_data[0] + section_alignment + 0x04);

        s.avail_out = section_size_inflated;
        s.next_out = (Bytef *)&inflatedData[0];

        ret = Zinflate(&s, Z_FINISH);
        if (ret != Z_OK && ret != Z_STREAM_END)
            return;

        ZinflateEnd(&s);
        section_data.swap(inflatedData);
    }

    // Parse imports
    size_t offset = 0;
    do
    {
        if (strncmp(&section_data[offset+1], ".fimport_", 9) == 0)
        {
            rplImports.push_back(std::string(&section_data[offset+1+9]));
        }
        offset++;
        while (section_data[offset] != 0) {
            offset++;
        }
    } while(offset + 1 < section_data.size());
}
