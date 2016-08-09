#ifndef COMMON_H
#define	COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#define LOADIINE_VERSION        "v0.3"
#define IS_USING_MII_MAKER      1

/* Loadiine common paths */
#define CAFE_OS_SD_PATH         "/vol/external01"
#define SD_PATH                 "sd:"
#define WIIU_PATH                "/wiiu"
#define GAMES_PATH               "/games"
#define SAVES_PATH               "/saves"
#define SD_GAMES_PATH            WIIU_PATH GAMES_PATH
#define SD_SAVES_PATH            WIIU_PATH SAVES_PATH
#define CONTENT_PATH            "/content"
#define RPX_RPL_PATH            "/code"
#define META_PATH               "/meta"

/* file replacement */
#define UPDATE_PATH             "/updates"
#define COMMON_UPDATE_PATH      "<none>"
#define FILELIST_NAME			"filelist.txt"
#define DIR_IDENTIFY			"?"  /* maximum length = 1*/
#define PARENT_DIR_IDENTIFY 	"?.."

/* Macros for libs */
#define LIB_CORE_INIT           0
#define LIB_NSYSNET             1
#define LIB_GX2                 2
#define LIB_AOC                 3
#define LIB_AX                  4
#define LIB_FS                  5
#define LIB_OS                  6
#define LIB_PADSCORE            7
#define LIB_SOCKET              8
#define LIB_SYS                 9
#define LIB_VPAD                10
#define LIB_NN_ACP              11
#define LIB_SYSHID              12
#define LIB_VPADBASE            13

// functions types
#define STATIC_FUNCTION         0
#define DYNAMIC_FUNCTION        1


// none dynamic libs
#define LIB_LOADER              0x1001

/* Loadiine Modes */
#define LOADIINE_MODE_MII_MAKER     0
#define LOADIINE_MODE_SMASH_BROS    1
#define LOADIINE_MODE_KARAOKE       2
#define LOADIINE_MODE_ART_ATELIER   3
#define LOADIINE_MODE_DEFAULT	    255

/* homebrew launcher return codes */
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS                0
#endif
#define EXIT_RELAUNCH_ON_LOAD       0xFFFFFFFD

/* RPX Address : where the rpx is copied or retrieve, depends if we dump or replace */
/* Note : from phys 0x30789C5D to 0x31E20000, memory seems empty (space reserved for root.rpx) which let us approximatly 22.5mB of memory free to put the rpx and additional rpls */
#ifndef MEM_BASE
#define MEM_BASE                (0x00800000)
#endif

#define ELF_DATA_ADDR           (*(volatile unsigned int*)(MEM_BASE + 0x1300 + 0x00))
#define ELF_DATA_SIZE           (*(volatile unsigned int*)(MEM_BASE + 0x1300 + 0x04))
#define MAIN_ENTRY_ADDR         (*(volatile unsigned int*)(MEM_BASE + 0x1400 + 0x00))
#define OS_FIRMWARE             (*(volatile unsigned int*)(MEM_BASE + 0x1400 + 0x04))
#define PREP_TITLE_CALLBACK     (*(volatile unsigned int*)(MEM_BASE + 0x1400 + 0x08))
#define SERVER_IP               (*(volatile unsigned int*)(MEM_BASE + 0x1400 + 0x0C))
#define RPX_CHECK_NAME          (*(volatile unsigned int*)(MEM_BASE + 0x1400 + 0x10))
#define GAME_RPX_LOADED         (*(volatile unsigned int*)(MEM_BASE + 0x1400 + 0x14))
#define GAME_LAUNCHED           (*(volatile unsigned int*)(MEM_BASE + 0x1400 + 0x18))
#define LOADIINE_MODE           (*(volatile unsigned int*)(MEM_BASE + 0x1400 + 0x1C))      // loadiine operation mode (1 = smash bros, 0 = mii maker)

#define OS_SPECIFICS            ((OsSpecifics*)(MEM_BASE + 0x1500))

typedef struct _game_paths_t
{
    char os_game_path_base[511];
    char os_save_path_base[511];
    char game_dir[255];
	char update_folder[255];
    char save_dir_common[10];
    char save_dir_user[10];
    int extraSave;
} game_paths_t;

#ifdef __cplusplus
}
#endif

#endif	/* COMMON_H */

