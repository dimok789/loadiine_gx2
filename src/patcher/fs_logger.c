#include <malloc.h>
#include "common/common.h"
#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "utils/function_patcher.h"
#include "fs_logger.h"
#include "utils/utils.h"
#include "utils/logger.h"

#define CHECK_ERROR(cond) if (cond) { goto error; }

static int cur_sock = 1;

int fs_logger_connect(int *socket) {
    *socket = cur_sock++;
    log_printf("[%02X] connected\n",*socket);
    return 0;
}

void fs_logger_disconnect(int socket) {
    log_printf("[%02X] disconnected\n",socket);
}

char * getNameForByte(u8 byte){
    switch(byte) {
        case BYTE_NORMAL:                       return "NORMAL";
        case BYTE_SPECIAL:                      return "SPECIAL";
        case BYTE_OK:                           return "OK";
        case BYTE_PING:                         return "PING";
        case BYTE_DISCONNECT:                   return "DISCONNECT";
        case BYTE_LOG_STR:                      return "Logging";
        case BYTE_MOUNT_SD:                     return "Mounting SD Card";
        case BYTE_MOUNT_SD_OK:                  return "Mounting SD Card success";
        case BYTE_MOUNT_SD_BAD:                 return "Mounting SD Card failed";
        case BYTE_STAT:                         return "GetStat";
        case BYTE_STAT_ASYNC:                   return "GetStatAsync";
        case BYTE_OPEN_FILE:                    return "OpenFile";
        case BYTE_OPEN_FILE_ASYNC:              return "OpenFileAsync";
        case BYTE_OPEN_DIR:                     return "OpenDir";
        case BYTE_OPEN_DIR_ASYNC:               return "OpenDirAsync";
        case BYTE_CHANGE_DIR:                   return "ChangeDir";
        case BYTE_CHANGE_DIR_ASYNC:             return "ChangeDirAsync";
        case BYTE_MAKE_DIR:                     return "MakeDir";
        case BYTE_MAKE_DIR_ASYNC:               return "MakeDirAsync";
        case BYTE_RENAME:                       return "Rename";
        case BYTE_RENAME_ASYNC:                 return "RenameAsync";
        case BYTE_REMOVE:                       return "Remove";
        case BYTE_REMOVE_ASYNC:                 return "RemoveAsync";
        case BYTE_CLOSE_FILE:                   return "CloseFile";
        case BYTE_CLOSE_FILE_ASYNC:             return "CloseFileAsync";
        case BYTE_CLOSE_DIR:                    return "CloseDir";
        case BYTE_CLOSE_DIR_ASYNC:              return "CloseDirAsync";
        case BYTE_FLUSH_FILE:                   return "FlushFile";
        case BYTE_GET_ERROR_CODE_FOR_VIEWER:    return "GetErrorCodeV";
        case BYTE_GET_LAST_ERROR:               return "LastError";
        case BYTE_GET_MOUNT_SOURCE:             return "MountSource";
        case BYTE_GET_MOUNT_SOURCE_NEXT:        return "MountSourceNext";
        case BYTE_GET_POS_FILE:                 return "GetPosFile";
        case BYTE_SET_POS_FILE:                 return "SetPosFile";
        case BYTE_GET_STAT_FILE:                return "GetStatFile";
        case BYTE_EOF:                          return "EOF";
        case BYTE_READ_FILE:                    return "ReadFile";
        case BYTE_READ_FILE_ASYNC:              return "ReadFileAsync";
        case BYTE_READ_FILE_WITH_POS:           return "ReadFilePOS";
        case BYTE_READ_DIR:                     return "ReadDir";
        case BYTE_READ_DIR_ASYNC:               return "ReadDirAsync";
        case BYTE_GET_CWD:                      return "GetCWD";
        case BYTE_SET_STATE_CHG_NOTIF:          return "BYTE_SET_STATE_CHG_NOTIF";
        case BYTE_TRUNCATE_FILE:                return "TruncateFile";
        case BYTE_WRITE_FILE:                   return "WriteFile";
        case BYTE_WRITE_FILE_WITH_POS:          return "WriteFilePos";
        case BYTE_SAVE_INIT:                    return "SaveInit";
        case BYTE_SAVE_SHUTDOWN:                return "SaveShutdown";
        case BYTE_SAVE_INIT_SAVE_DIR:           return "SaveInitSaveDir";
        case BYTE_SAVE_FLUSH_QUOTA:             return "SaveFlushQuota";
        case BYTE_SAVE_OPEN_DIR:                return "SaveOpenDir";
        case BYTE_SAVE_REMOVE:                  return "SaveRemove";
        case BYTE_CREATE_THREAD:                return "CreateThread";

        default:                                return "UNKWN";
    }
    return "";
}

void fs_log_string(int sock, const char* str, unsigned char flag_byte) {

    log_printf("[%02d] %-24s: %s\n",sock, getNameForByte(flag_byte), str);
}

void fs_log_byte(int sock, unsigned char flag_byte) {
    log_printf("[%02d] %-24s\n",sock, getNameForByte(flag_byte));
}
