#ifndef __SD_FAT_DEVOPTAB_H_
#define __SD_FAT_DEVOPTAB_H_

#ifdef __cplusplus
extern "C" {
#endif

int mount_sd_fat(const char *path);
int unmount_sd_fat(const char *path);

#ifdef __cplusplus
}
#endif

#endif // __SD_FAT_DEVOPTAB_H_
