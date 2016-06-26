#include <algorithm>
#include <string>
#include <map>
#include <string.h>
#include <fcntl.h>

#include <zlib.h>
#include "fs/fs_utils.h"
#include "settings/CSettings.h"
#include "GameLauncher.h"
#include "fs/CFile.hpp"
#include "fs/DirList.h"
#include "kernel/kernel_functions.h"
#include "utils/StringTools.h"
#include "utils/logger.h"
#include "utils/xml.h"
#include "utils/utils.h"
#include "settings/CSettingsGame.h"
#include "settings/SettingsGameDefs.h"
#include "utils/FileReplacer.h"
#include "common/retain_vars.h"

/* RPX stuff */
#define RPX_SH_STRNDX_OFFSET            0x0032
#define RPX_SHT_START                   0x0040
#define RPX_SHT_ENTRY_SIZE              0x28

#define RPX_SHDR_FLAGS_OFFSET           0x08
#define RPX_SHDR_OFFSET_OFFSET          0x10
#define RPX_SHDR_SIZE_OFFSET            0x14

#define RPX_SHDR_ZLIB_FLAG              0x08000000

game_paths_t gamePathStruct;

/* global variable for CosAppXml struct that is forced to data section */
extern ReducedCosAppXmlInfo cosAppXmlInfoStruct;

GameLauncher * GameLauncher::loadGameToMemoryAsync(const discHeader *hdr)
{
    GameLauncher * launcher = new GameLauncher(hdr);
    launcher->resumeThread();
    return launcher;
}

void GameLauncher::executeThread()
{
    int result = loadGameToMemory(discHdr);
    asyncLoadFinished(this, discHdr, result);
}

int GameLauncher::loadGameToMemory(const discHeader *header)
{
    if(!header)
        return INVALID_INPUT;

    //! initialize our tables required for the games
    memoryInitAreaTable();
    rpxRplTableInit();


	std::map<std::string, std::string> rplUpdateNameList;
	std::map<std::string, std::string> rplFinalNameList;
	std::string completeUpdatePath(header->gamepath);
	DirList rpxList(header->gamepath + RPX_RPL_PATH, ".rpx", DirList::Files);
	std::string rpxName;
	std::string rpxPath;

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

	rpxName = rpxList.GetFilename(0);
	rpxPath = rpxList.GetFilepath(0);

	//! TODO: clean this path creation up
    std::string game_dir = header->gamepath;
    size_t pos = game_dir.rfind('/');

    if(pos != std::string::npos)
        game_dir = game_dir.substr(pos + 1);

	//! set the save game path from the gamenames path
    std::string saveGamePath = CSettings::getValueAsString(CSettings::GameSavePath) + "/" + game_dir;

	std::string saveGamePathCommon;
    std::string saveGamePathUser;
	std::string updateFolder = COMMON_UPDATE_PATH;

	u8 save_method = CSettings::getValueAsU8(CSettings::GameSaveMode);

	GameSettings gs;

    bool extra_save = false;

	bool gs_result = CSettingsGame::getInstance()->LoadGameSettings(header->id, gs);
	bool use_new_xml = false;
	if(gs_result){
        log_printf("Found game settings\n");
        if(gs.updateFolder.compare(COMMON_UPDATE_PATH) != 0){
            log_printf("Using Update folder\n");

            gSettingUseUpdatepath = 1;
            updateFolder = gs.updateFolder;
            completeUpdatePath += UPDATE_PATH + std::string("/") + updateFolder;

            createFileList(completeUpdatePath);

            //Checking RPX
            DirList rpxUpdateList(completeUpdatePath + RPX_RPL_PATH, ".rpx", DirList::Files);
            if(rpxUpdateList.GetFilecount() != 0){
                log_printf("Using RPX from update path\n");
                use_new_xml = true;
                rpxName = rpxUpdateList.GetFilename(0);
                rpxPath = rpxUpdateList.GetFilepath(0);
            }else{
                log_printf("Using RPX from game path\n");
            }

            //Checking RPL
            DirList rplUpdateList(completeUpdatePath + RPX_RPL_PATH, ".rpl", DirList::Files);
            if(rplUpdateList.GetFilecount() != 0)
            {
                log_printf("Using RPL from update path\n");
                for(int i = 0; i < rplUpdateList.GetFilecount(); i++){
                    rplUpdateNameList[rplUpdateList.GetFilename(i)] = rplUpdateList.GetFilepath(i);
                    rplFinalNameList[rplUpdateList.GetFilename(i)] = rplUpdateList.GetFilepath(i);
                }
            }else{
                log_printf("Using RPL from game path\n");
            }

            if(gs.extraSave){
               log_print("Using extra save path for update\n");
               extra_save = true;
            }
		}else{
            log_printf("Not using Update folder\n");
            gSettingUseUpdatepath = 0;
        }

		switch(gs.save_method){
			case GAME_SAVES_DEFAULT:
				log_printf("Using save method from Settings\n");
				break; //leave it from settings
			case GAME_SAVES_SHARED:
				log_printf("Using GAME_SAVES_SHARED cfg\n");
				save_method = GAME_SAVES_SHARED;
				break;
			case GAME_SAVES_UNIQUE:
				log_printf("Using GAME_SAVES_UNIQUE cfg\n");
				save_method = GAME_SAVES_UNIQUE;
				break;
			default:
				log_printf("Default: GAME_SAVES_SHARED\n");
				log_printf("%d\n",gs.save_method);
				save_method = GAME_SAVES_SHARED;
				break;
		}
	}

    u32 entryIndex = 0;
    std::vector<std::string> rplImportList;

    //! get all imports from the RPX
    CFile rpxFile(rpxPath.c_str(),CFile::ReadOnly);

    int result = LoadRpxRplToMem(rpxPath.c_str(), rpxName.c_str(), true, entryIndex++, rplImportList);
    if(result < 0)
    {
        log_printf("Failed loading RPX file %s, error %i\n", rpxName.c_str(), result);
        return result;
    }else{
		log_printf("Loaded RPX file %s, result %i\n", rpxPath.c_str(), result);
	}

	//! add missing rpl to vector
	DirList rplList(header->gamepath + RPX_RPL_PATH, ".rpl", DirList::Files);
	std::map<std::string, std::string>::iterator itr;
	std::map<std::string, std::string>::iterator itr_lower;

    std::string new_lower;
    std::string old_lower;

    bool found = false;
	for(int i = 0; i < rplList.GetFilecount(); i++)
    {
        new_lower = rplList.GetFilename(i);
        found = false;
        for(itr_lower = rplUpdateNameList.begin(); itr_lower != rplUpdateNameList.end(); itr_lower++) {
            old_lower = itr_lower->first;
            std::transform(old_lower.begin(), old_lower.end(), old_lower.begin(), ::tolower);
            std::transform(new_lower.begin(), new_lower.end(), new_lower.begin(), ::tolower);
            if(old_lower.compare(new_lower) == 0){
                log_printf("Adding RPL %s from %s\n",itr_lower->first.c_str(),itr_lower->second.c_str());
                rplFinalNameList[itr_lower->first] = itr_lower->second;
                found = true;
                break;
            }
        }

		if(!found){
			log_printf("Adding RPL %s from %s\n",rplList.GetFilename(i),rplList.GetFilepath(i));
			rplFinalNameList[rplList.GetFilename(i)] = rplList.GetFilepath(i);
		}
	}

	//! get imports
    GetRpxImportsRecursive(rpxFile,rplImportList,rplFinalNameList);

	for(itr = rplFinalNameList.begin(); itr != rplFinalNameList.end(); itr++) {
        result = LoadRpxRplToMem(itr->second.c_str(), itr->first.c_str(), false, entryIndex++, rplImportList);
        if(result < 0)
        {
            log_printf("Failed loading RPL file %s, error %i\n", itr->second.c_str(), result);
            return result;
        }else{
			log_printf("Loaded RPL file %s, result %i\n", itr->second.c_str(), result);
		}
    }

    if(save_method == GAME_SAVES_SHARED)
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

    if(!extra_save){
        CreateSubfolder((saveGamePath + "/" + saveGamePathUser).c_str());
        CreateSubfolder((saveGamePath + "/" + saveGamePathCommon).c_str());
    }else{
        CreateSubfolder((saveGamePath + "/" + updateFolder + "/" + saveGamePathUser).c_str());
        CreateSubfolder((saveGamePath + "/" + updateFolder + "/" + saveGamePathCommon).c_str());
    }


    std::string tempPath = CSettings::getValueAsString(CSettings::GamePath);
    //! remove "sd:" and replace with "/vol/external01"
    pos = tempPath.find('/');
    if(pos != std::string::npos)
        tempPath = std::string(CAFE_OS_SD_PATH) + tempPath.substr(pos);

    strlcpy(gamePathStruct.os_game_path_base, tempPath.c_str(), sizeof(gamePathStruct.os_game_path_base));

    tempPath = CSettings::getValueAsString(CSettings::GameSavePath);
    //! remove "sd:" and replace with "/vol/external01"
    pos = tempPath.find('/');
    if(pos != std::string::npos)
        tempPath = std::string(CAFE_OS_SD_PATH) + tempPath.substr(pos);

    strlcpy(gamePathStruct.os_save_path_base, tempPath.c_str(), sizeof(gamePathStruct.os_save_path_base));
    strlcpy(gamePathStruct.game_dir, game_dir.c_str(), sizeof(gamePathStruct.game_dir));
	strlcpy(gamePathStruct.update_folder, updateFolder.c_str(), sizeof(gamePathStruct.update_folder));
    strlcpy(gamePathStruct.save_dir_common, saveGamePathCommon.c_str(), sizeof(gamePathStruct.save_dir_common));
    strlcpy(gamePathStruct.save_dir_user, saveGamePathUser.c_str(), sizeof(gamePathStruct.save_dir_user));

    if(extra_save){
        gamePathStruct.extraSave = 1;
    }else{
        gamePathStruct.extraSave = 0;
    }

    log_printf("gamePathStruct.os_game_path_base: %s\n", gamePathStruct.os_game_path_base);
    log_printf("gamePathStruct.os_save_path_base: %s\n", gamePathStruct.os_save_path_base);
    log_printf("gamePathStruct.game_dir:          %s\n", gamePathStruct.game_dir);
	log_printf("gamePathStruct.update_folder:     %s\n", gamePathStruct.update_folder);
    log_printf("gamePathStruct.save_dir_common:   %s\n", gamePathStruct.save_dir_common);
    log_printf("gamePathStruct.save_dir_user:     %s\n", gamePathStruct.save_dir_user);
    log_printf("gamePathStruct.extraSave:         %d\n", gamePathStruct.extraSave);

	if(!use_new_xml){
        log_printf("Getting XML from game\n");
		LoadXmlParameters(&cosAppXmlInfoStruct, rpxName.c_str(), (header->gamepath + RPX_RPL_PATH).c_str());
	}else{
        log_printf("Getting XML from update\n");
		LoadXmlParameters(&cosAppXmlInfoStruct, rpxName.c_str(), (completeUpdatePath + RPX_RPL_PATH).c_str());
	}

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

bool GameLauncher::createFileList(const std::string & filepath){
    bool result = false;

    std::string filelist_name = filepath + "/" +  std::string(FILELIST_NAME);
    log_printf("Reading %s\n",filelist_name.c_str());
    bool createNewFile = true;
    CFile file(filelist_name, CFile::ReadOnly);
    if (file.isOpen()){
        if(file.size() != 0){
            createNewFile = false;
        }
        file.close();
    }
    if(createNewFile){
        log_printf("Filelist is missing, creating it!\n");

        log_printf("Creating filelist of content in %s\n",filepath.c_str());
        log_printf("Saving it to %s\n",filelist_name.c_str());
        progressWindow.setProgress(100.0f);
        FileReplacer replacer(filepath,CONTENT_PATH,"",progressWindow);
        progressWindow.setTitle("");
        std::string strBuffer = replacer.getFileListAsString();
        if(strBuffer.length() > 0){
            CFile  filelist(filelist_name, CFile::WriteOnly);
            if (filelist.isOpen()){
                int ret = 0;
                progressWindow.setTitle("Writing list to SD");
                if((ret = filelist.write((u8*)strBuffer.c_str(), strBuffer.size())) == -1){
                    log_printf("Error on write: %d\n",ret);
                }
                filelist.close();
            }else{
                log_printf("Error. Couldn't open file\n");
            }
        }

    }
    return result;
}

int GameLauncher::LoadRpxRplToMem(const std::string & path, const std::string & name, bool isRPX, int entryIndex, std::vector<std::string> & rplImportList)
{
    // For RPLs :
    int preload = 0;
    if(!isRPX)
    {
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
            rpxRplTableAddEntry(name.c_str(), 0, 0, isRPX, entryIndex, memoryGetAreaTable());
            return 1;
        }
        log_printf("Pre-loading RPL %s because its in the fimport section\n", name.c_str());
    }

    CFile file(path, CFile::ReadOnly);
    if(!file.isOpen())
        return FILE_OPEN_FAILURE;

    u32 fileSize = file.size();

    // this is the initial area
    s_mem_area* mem_area    = memoryGetAreaTable();
    unsigned int mem_area_addr_start = mem_area->address;
    unsigned int mem_area_addr_end   = mem_area->address + mem_area->size;
    unsigned int mem_area_offset     = 0;

    // on RPLs we need to find the free area we can store data to (at least RPX was already loaded by this point)
    if(!isRPX)
        mem_area = rpxRplTableGetNextFreeMemArea(&mem_area_addr_start, &mem_area_addr_end, &mem_area_offset);

    if(!mem_area)
    {
        log_printf("Not enough memory for file %s\n", path.c_str());
        return NOT_ENOUGH_MEMORY;
    }

    // malloc mem for read file
    std::string strBuffer;
    strBuffer.resize(0x10000);
    unsigned char *pBuffer = (unsigned char*)&strBuffer[0];
    unsigned char *pBufferPhysical = (unsigned char*)OSEffectiveToPhysical(&strBuffer[0]);

    // fill rpx entry
    u32 bytesRead = 0;
    s_rpx_rpl* rpx_rpl_struct = rpxRplTableAddEntry(name.c_str(), mem_area_offset, 0, isRPX, entryIndex, mem_area);
    if(!rpx_rpl_struct)
    {
        log_printf("Not enough memory for file %s\n", path.c_str());
        return NOT_ENOUGH_MEMORY;
    }

    progressWindow.setTitle(strfmt("Loading file %s", name.c_str()));

    // Copy rpl in memory
    while(bytesRead < fileSize)
    {
        progressWindow.setProgress(100.0f * (f32)bytesRead / (f32)fileSize);

        u32 blockSize = strBuffer.size();
        if(blockSize > (fileSize - bytesRead))
            blockSize = fileSize - bytesRead;

        int ret = file.read(pBuffer, blockSize);
        if(ret <= 0)
        {
            log_printf("Failure on reading file %s\n", path.c_str());
            break;
        }

		DCFlushRange(pBuffer, ret);

        int copiedData = rpxRplCopyDataToMem(rpx_rpl_struct, bytesRead, pBufferPhysical, ret);
        if(copiedData != ret)
        {
            log_printf("Not enough memory for file %s. Could not copy all data %i != %i.\n", rpx_rpl_struct->name, copiedData, ret);
            return NOT_ENOUGH_MEMORY;
        }
        rpx_rpl_struct->size += ret;
        bytesRead += ret;
    }

    progressWindow.setProgress((f32)bytesRead / (f32)fileSize);

    if(bytesRead != fileSize)
    {
        log_printf("File loading not finished for file %s, finished %i of %i bytes\n", path.c_str(), bytesRead, fileSize);
        return FILE_READ_ERROR;
    }

    // remember which RPX has to be checked for on loader for allowing the list compare
    if(isRPX) {
        RPX_CHECK_NAME = *(unsigned int*)name.c_str();
    }

    // return okay
    return 0;
}

void GameLauncher::GetRpxImportsRecursive(CFile file, std::vector<std::string> & rplImports, std::map<std::string, std::string> & rplNameList)
{
    std::string strBuffer;
    strBuffer.resize(0x1000);

    if(!file.isOpen()){
         log_printf("GetRpxImportsRecursive error: file not open\n");
         return;
    }

    // get the header information of the RPX
    if(file.read((unsigned char *)&strBuffer[0], 0x1000) == -1){
        log_printf("GetRpxImportsRecursive error: reading file\n");
        return;
    }
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

    int res = 0;
    // get the header information of the RPX
    if((res = file.seek(section_offset_aligned,SEEK_SET)) != section_offset_aligned){
        log_printf("GetRpxImportsRecursive error: file.seek failed! Result: %d, %d\n",res);
        return;
    }

    if((res = file.read((unsigned char *)&section_data[0], section_size_aligned)) == -1){
        log_printf("GetRpxImportsRecursive error: file read failed! Result: %d\n",res);
        return;
    }

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
    std::map<std::string, std::string>::iterator itr;
    size_t offset = 0;
    std::string name_lower;
    std::string import_lower;
    do
    {
        if (strncmp(&section_data[offset+1], ".fimport_", 9) == 0)
        {
            std::string import = std::string(&section_data[offset+1+9]);
            // Add file suffix for easier handling
            if (import.find(".rpl") == std::string::npos)
            {
                import += ".rpl";
            }

            if(std::find(rplImports.begin(), rplImports.end(), import)==rplImports.end()){ //new
                rplImports.push_back(import);

                bool found = false;

                for(itr = rplNameList.begin(); itr != rplNameList.end(); itr++) {
                    std::vector<std::string> name_vec = stringSplit(itr->second,"/");
                    if(name_vec.size() < 1) break;
                    name_lower  = name_vec.at(name_vec.size()-1);
                    import_lower  = import;
                    std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(), ::tolower);
                    std::transform(import_lower.begin(), import_lower.end(), import_lower.begin(), ::tolower);
                    //log_printf("%s = %s\n",name_lower.c_str(),import_lower.c_str());
                    if(name_lower.compare(import_lower) == 0){
                        found = true;

                        break;
                    }
                }

                if(found){
                    CFile newFile(itr->second,CFile::ReadOnly);
                    if(newFile.isOpen()){
                        GetRpxImportsRecursive(newFile,rplImports,rplNameList);
                        newFile.close();
                    }else{
                        log_printf("GetRpxImportsRecursive error: Couldn't open RPL (RPL: %s from path: %s)\n",import.c_str(),itr->second.c_str());
                    }
                }
            }
        }
        offset++;
        while (section_data[offset] != 0) {
            offset++;
        }
    } while(offset + 1 < section_data.size());
}
