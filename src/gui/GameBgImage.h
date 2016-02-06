#ifndef _GAME_BG_IMAGE_H_
#define _GAME_BG_IMAGE_H_

#include "GuiImageAsync.h"
#include "video/shaders/Shader3D.h"

class GameBgImage : public GuiImageAsync
{
public:
    GameBgImage(const std::string & filename, GuiImageData *preloadImage);
    virtual ~GameBgImage();

    void setAlphaFadeOut(const glm::vec4 & a) {
        alphaFadeOut = a;
    }

    void draw(CVideo *pVideo);
private:
    glm::mat4 identity;
    glm::vec4 alphaFadeOut;
};

#endif // _GAME_BG_IMAGE_H_
