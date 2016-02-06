#ifndef _GRID_BACKGROUND_H_
#define _GRID_BACKGROUND_H_

#include "GuiImage.h"
#include "video/shaders/Shader.h"

class GridBackground : public GuiImage
{
public:
    GridBackground(GuiImageData *imgData);
    virtual ~GridBackground();

    void setColorIntensity(const glm::vec4 & color) {
        colorIntensity = color;
    }
    const glm::vec4 & getColorIntensity() const {
        return colorIntensity;
    }
    void setDistanceFadeOut(const float & a) {
        distanceFadeOut = a;
    }
    void draw(CVideo *pVideo, const glm::mat4 & modelView);
private:
    glm::mat4 m_modelView;
    glm::vec4 colorIntensity;
    glm::vec4 alphaFadeOut;
    float distanceFadeOut;
};

#endif // _GRID_BACKGROUND_H_
