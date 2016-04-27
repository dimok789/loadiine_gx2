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

static u8 * cursor_colorVtxs = NULL;

void init_cursor(){
    if(!gCursorInitDone){
        if(!cursor_colorVtxs){
            cursor_colorVtxs = (u8*)memalign(0x40, sizeof(u8) * 16);
        }
        memset(cursor_colorVtxs,0xFF,16*sizeof(u8));
        GX2Invalidate(GX2_INVALIDATE_CPU_ATTRIB_BUFFER, cursor_colorVtxs, 16 * sizeof(u8));
        gCursorInitDone = 1;
    }
}

void draw_Cursor_at(f32 x, f32 y) {
    init_cursor();
	f32 widthScaleFactor = 1.0f / (f32)1280;
    f32 heightScaleFactor = 1.0f / (f32)720;

    int width = 20;

    glm::vec3 positionOffsets = glm::vec3(0.0f);

    positionOffsets[0] = (x-((1280)/2)+(width/2)) * widthScaleFactor * 2.0f;
    positionOffsets[1] = -(y-((720)/2)+(width/2)) * heightScaleFactor * 2.0f;

    glm::vec3 scale(width*widthScaleFactor,width*heightScaleFactor,1.0f);

    ColorShader::instance()->setShaders();
    ColorShader::instance()->setAttributeBuffer(cursor_colorVtxs, NULL, 4);
    ColorShader::instance()->setAngle(0);
    ColorShader::instance()->setOffset(positionOffsets);
    ColorShader::instance()->setScale(scale);
    ColorShader::instance()->setColorIntensity(glm::vec4(1.0f));
    ColorShader::instance()->draw(GX2_PRIMITIVE_QUADS, 4);
}

void draw_Cursor_destroy() {
    //! destroy shaders
    ColorShader::destroyInstance();
    if(cursor_colorVtxs){
        free(cursor_colorVtxs);
        cursor_colorVtxs = NULL;
    }
    gCursorInitDone = 0;
}
