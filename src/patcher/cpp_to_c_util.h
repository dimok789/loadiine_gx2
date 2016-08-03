 #ifdef __cplusplus
 #define EXTERNC extern "C"
 #else
 #define EXTERNC
 #endif
 #include "common/types.h"

 typedef void* replacement_FileReplacer_t;
 EXTERNC replacement_FileReplacer_t replacement_FileReplacer_initWithFile(char * path, char * content, char * filename,void * pClient,void * pCmd);
 EXTERNC void replacement_FileReplacer_destroy(replacement_FileReplacer_t filereplacer);
 EXTERNC int replacement_FileReplacer_isFileExisting(replacement_FileReplacer_t self, char* param);

 #undef EXTERNC
