#pragma once

#include "bsp.h"

using HTEXTURE = unsigned long long;
#define TEXTURE_CUBEMAP_POSITIVE_X (0)
#define TEXTURE_CUBEMAP_NEGATIVE_X (TEXTURE_CUBEMAP_POSITIVE_X + 1)
#define TEXTURE_CUBEMAP_POSITIVE_Y (TEXTURE_CUBEMAP_NEGATIVE_X + 1)
#define TEXTURE_CUBEMAP_NEGATIVE_Y (TEXTURE_CUBEMAP_POSITIVE_Y + 1)
#define TEXTURE_CUBEMAP_POSITIVE_Z (TEXTURE_CUBEMAP_NEGATIVE_Y + 1)
#define TEXTURE_CUBEMAP_NEGATIVE_Z (TEXTURE_CUBEMAP_POSITIVE_Z + 1)

class IGraphicsEngine {
public:
    virtual void Initialize(int nScreenWidth, int nScreenHeight, bool bFullscreen) = 0;
    virtual void Shutdown() = 0;

    virtual void ClearScreen() = 0;
    virtual void SwapScreen() = 0;
    virtual void DrawPolygonSet(PolygonContainer const* pPolySet) = 0;
    virtual void DrawBSPTree(bsp_node const* pTree) = 0;

    virtual void SetCameraPosition(vector4 const* pPos) = 0;
    virtual void SetCameraRotation(vector4 const* pRot) = 0;
    virtual void GetCameraPosition(vector4* pPos) = 0;
    virtual void GetCameraRotation(vector4* pRot) = 0;

    virtual float GetFrameTime() = 0;

    virtual void RenderWireframe(bool bEnable) = 0;

    virtual int LoadTexture(HTEXTURE* pHandle, char const* pchPath) = 0;
    virtual int LoadCubemapTexture(HTEXTURE* pHandle, char const* pchPathFaces[6]) = 0;

    virtual void DrawSkybox(HTEXTURE hCubemapTexture) = 0;
};

IGraphicsEngine* GraphicsEngine();
