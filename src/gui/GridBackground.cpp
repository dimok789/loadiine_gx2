#include "GridBackground.h"
#include "video/CVideo.h"
#include "video/shaders/Shader3D.h"

static const float bgRepeat = 1000.0f;
static const float bgTexRotate = 39.0f;

GridBackground::GridBackground(GuiImageData *img)
    : GuiImage(img)
{
    colorIntensity = glm::vec4(1.0f, 1.0f, 1.0f, 0.9f);
    alphaFadeOut = glm::vec4(0.0f);
    distanceFadeOut = 0.15f;

    vtxCount = 4;

    //! texture and vertex coordinates
    f32 *m_posVtxs = (f32*)memalign(GX2_VERTEX_BUFFER_ALIGNMENT, vtxCount * Shader3D::cuVertexAttrSize);
    f32 *m_texCoords = (f32*)memalign(GX2_VERTEX_BUFFER_ALIGNMENT, vtxCount * Shader3D::cuTexCoordAttrSize);

    if(m_posVtxs)
    {
        int i = 0;
        m_posVtxs[i++] = -1.0f; m_posVtxs[i++] = 0.0f; m_posVtxs[i++] = 1.0f;
        m_posVtxs[i++] =  1.0f; m_posVtxs[i++] = 0.0f; m_posVtxs[i++] = 1.0f;
        m_posVtxs[i++] =  1.0f; m_posVtxs[i++] = 0.0f; m_posVtxs[i++] = -1.0f;
        m_posVtxs[i++] = -1.0f; m_posVtxs[i++] = 0.0f; m_posVtxs[i++] = -1.0f;
        GX2Invalidate(GX2_INVALIDATE_CPU_ATTRIB_BUFFER, m_posVtxs, vtxCount * Shader3D::cuVertexAttrSize);
    }

    if(m_texCoords)
    {
        glm::vec2 texCoordVec[4];
        texCoordVec[0][0] = -0.5f * bgRepeat; texCoordVec[0][1] = 0.5f * bgRepeat;
        texCoordVec[1][0] = 0.5f * bgRepeat; texCoordVec[1][1] = 0.5f * bgRepeat;
        texCoordVec[2][0] = 0.5f * bgRepeat; texCoordVec[2][1] = -0.5f * bgRepeat;
        texCoordVec[3][0] = -0.5f * bgRepeat; texCoordVec[3][1] = -0.5f * bgRepeat;

        const float cosRot = cosf(DegToRad(bgTexRotate));
        const float sinRot = sinf(DegToRad(bgTexRotate));

        glm::mat2 texRotateMtx({
            cosRot, -sinRot,
            sinRot, cosRot
        });

        for(int i = 0; i < 4; i++)  {
            texCoordVec[i] = texRotateMtx * texCoordVec[i];
            m_texCoords[i*2 + 0] = texCoordVec[i][0];
            m_texCoords[i*2 + 1] = texCoordVec[i][1];
        }

        GX2Invalidate(GX2_INVALIDATE_CPU_ATTRIB_BUFFER, m_texCoords, vtxCount * Shader3D::cuTexCoordAttrSize);
    }

    //! assign to internal variables which are const but oh well
    posVtxs = m_posVtxs;
    texCoords = m_texCoords;
}

GridBackground::~GridBackground()
{
    //! remove image so it can not be drawn anymore from this point on
    imageData = NULL;

    //! main image vertexes
    if(posVtxs)
    {
        free((void*)posVtxs);
        posVtxs = NULL;
    }
    if(texCoords)
    {
        free((void*)texCoords);
        texCoords = NULL;
    }
}

void GridBackground::draw(CVideo *pVideo, const glm::mat4 & modelView)
{
    //! first setup 2D GUI positions
    f32 currScaleX = bgRepeat * scaleX * (f32)getWidth() * pVideo->getWidthScaleFactor();
    f32 currScaleY = 1.0f;
    f32 currScaleZ = bgRepeat * scaleZ * (f32)getHeight() * pVideo->getDepthScaleFactor();

    m_modelView = glm::scale(modelView, glm::vec3(currScaleX, currScaleY, currScaleZ));

    colorIntensity[3] = getAlpha();

    Shader3D::instance()->setShaders();
    Shader3D::instance()->setTextureAndSampler(imageData->getTexture(), imageData->getSampler());
    Shader3D::instance()->setProjectionMtx(pVideo->getProjectionMtx());
    Shader3D::instance()->setViewMtx(pVideo->getViewMtx());
    Shader3D::instance()->setModelViewMtx(m_modelView);
    Shader3D::instance()->setDistanceFadeOut(distanceFadeOut);
    Shader3D::instance()->setAlphaFadeOut(alphaFadeOut);
    Shader3D::instance()->setColorIntensity(colorIntensity);
    Shader3D::instance()->setAttributeBuffer(vtxCount, posVtxs, texCoords);
    Shader3D::instance()->draw(GX2_PRIMITIVE_QUADS, vtxCount);
}
