#ifndef COMMON_H
#define	COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#define LOADIINE_VERSION        "v0.1"
#define IS_USING_MII_MAKER      1

/* Loadiine common paths */
#define CAFE_OS_SD_PATH         "/vol/external01"
#define SD_PATH                 "sd:"
#define SD_LOADIINE_PATH        "/wiiu"
#define GAMES_PATH               "/games"
#define SAVES_PATH               "/saves"
#define SD_GAMES_PATH            SD_LOADIINE_PATH GAMES_PATH
#define SD_SAVES_PATH            SD_LOADIINE_PATH SAVES_PATH
#define CONTENT_PATH            "/content"
#define RPX_RPL_PATH            "/code"
#define META_PATH               "/meta"

/* Macros for libs */
#define LIB_CORE_INIT           0
#define LIB_NSYSNET             1
#define LIB_GX2                 2
// none dynamic libs
#define LIB_LOADER              0x1001

/* Loadiine Modes */
#define LOADIINE_MODE_MII_MAKER     0
#define LOADIINE_MODE_SMASH_BROS    1

/* RPX Address : where the rpx is copied or retrieve, depends if we dump or replace */
/* Note : from phys 0x30789C5D to 0x31E20000, memory seems empty (space reserved for root.rpx) which let us approximatly 22.5mB of memory free to put the rpx and additional rpls */
#define MEM_BASE                (0xC0800000)
#define OS_FIRMWARE             (*(volatile unsigned int*)(MEM_BASE - 0x04))
#define PREP_TITLE_CALLBACK     (*(volatile unsigned int*)(MEM_BASE - 0x08))
#define SERVER_IP               (*(volatile unsigned int*)(MEM_BASE - 0x0C))
#define RPX_CHECK_NAME          (*(volatile unsigned int*)(MEM_BASE - 0x10))
#define GAME_RPX_LOADED         (*(volatile unsigned int*)(MEM_BASE - 0x14))
#define GAME_LAUNCHED           (*(volatile unsigned int*)(MEM_BASE - 0x18))
#define LOADIINE_MODE           (*(volatile unsigned int*)(MEM_BASE - 0x1C))      // loadiine operation mode (1 = smash bros, 0 = mii maker)
#define MAIN_ENTRY_ADDR         (*(volatile unsigned int*)(MEM_BASE - 0x20))
#define GAME_PATH_STRUCT        ((game_paths_t*)(MEM_BASE - 0x800))

#define OS_SPECIFICS            ((OsSpecifics*)(MEM_BASE + 0x1500))

#define RESTORE_INSTR_MAGIC     0xC001C0DE
#define RESTORE_INSTR_ADDR      ((restore_instructions_t*)(MEM_BASE + 0x1600))

/* RPX_RPL_ARRAY contains an array of multiple rpl/rpl structures: */
/* Note : The first entry is always the one referencing the rpx (cf. struct s_rpx_rpl) */
#define RPX_RPL_ARRAY           ((s_rpx_rpl*)0xC07A0000)

/* MEM_AREA_ARRAY contains empty memory areas address - linked */
#define MEM_AREA_ARRAY          ((s_mem_area*)0xC0790000)

/* Struct used to organize empty memory areas */
typedef struct _s_mem_area
{
    unsigned int        address;
    unsigned int        size;
    struct _s_mem_area* next;
} s_mem_area;

/* Struct used to organize rpx/rpl data in memory */
typedef struct _s_rpx_rpl
{
    struct _s_rpx_rpl* next;
    s_mem_area*        area;
    unsigned int       offset;
    unsigned int       size;
    unsigned char      is_rpx;
    char               name[0];
} s_rpx_rpl;

typedef struct _restore_instructions_t {
    unsigned int magic;
    unsigned int instr_count;
    struct {
        unsigned int addr;
        unsigned int instr;
    } data[0];
} restore_instructions_t;

typedef struct _game_paths_t
{
    char *os_game_path_base;
    char *os_save_path_base;
    char *game_dir;
    char *save_dir_common;
    char *save_dir_user;
} game_paths_t;

#ifdef __cplusplus
}
#endif

#endif	/* COMMON_H */

