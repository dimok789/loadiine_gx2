#include "GameBgImage.h"
#include "video/CVideo.h"
#include "video/shaders/Shader3D.h"

GameBgImage::GameBgImage(const std::string & filename, GuiImageData *preloadImage)
    : GuiImageAsync(filename, preloadImage)
{
    identity = glm::mat4(1.0f);
    alphaFadeOut = glm::vec4(1.0f, 0.075f, 5.305f, 2.0f);
}

GameBgImage::~GameBgImage()
{
}

void GameBgImage::draw(CVideo *pVideo)
{
    if(!getImageData() || !getImageData()->getTexture())
        return;

    //! first setup 2D GUI positions
    f32 currPosX = getCenterX();
    f32 currPosY = getCenterY();
    f32 currPosZ = getDepth();
    f32 currScaleX = getScaleX() * (f32)getWidth() * pVideo->getWidthScaleFactor();
    f32 currScaleY = getScaleY() * (f32)getHeight() * pVideo->getHeightScaleFactor();
    f32 currScaleZ = getScaleZ() * (f32)getWidth() * pVideo->getDepthScaleFactor();

    glm::mat4 m_modelView = glm::translate(identity, glm::vec3(currPosX,currPosY, currPosZ));
    m_modelView = glm::scale(m_modelView, glm::vec3(currScaleX, currScaleY, currScaleZ));

    Shader3D::instance()->setShaders();
    Shader3D::instance()->setProjectionMtx(identity);
    Shader3D::instance()->setViewMtx(identity);
    Shader3D::instance()->setModelViewMtx(m_modelView);
    Shader3D::instance()->setTextureAndSampler(getImageData()->getTexture(), getImageData()->getSampler());
    Shader3D::instance()->setAlphaFadeOut(alphaFadeOut);
    Shader3D::instance()->setDistanceFadeOut(0.0f);
    Shader3D::instance()->setColorIntensity(glm::vec4(1.0f, 1.0f, 1.0f, getAlpha()));
    Shader3D::instance()->setAttributeBuffer();
    Shader3D::instance()->draw();
}
