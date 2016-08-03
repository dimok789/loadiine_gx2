#include "cpp_to_c_util.h"
#include <string.h>
#include "utils/FileReplacer.h"
#include "common/common.h"
#include "common/retain_vars.h"
#include "video/shaders/ColorShader.h"

replacement_FileReplacer_t replacement_FileReplacer_initWithFile(char * path, char * content, char * filename,void * pClient,void * pCmd) {
	return new FileReplacer(path,content,filename,pClient,pCmd);
}

void replacement_FileReplacer_destroy(replacement_FileReplacer_t untyped_ptr) {
	FileReplacer* typed_ptr = static_cast<FileReplacer*>(untyped_ptr);
	delete typed_ptr;
}

int replacement_FileReplacer_isFileExisting(replacement_FileReplacer_t untyped_self, char * param) {
	if(untyped_self != NULL){
		FileReplacer* typed_self = static_cast<FileReplacer*>(untyped_self);
		if(param != 0){
			return typed_self->isFileExisting(std::string(param));
		}
	}
	return -1;
}
