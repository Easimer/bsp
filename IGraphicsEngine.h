#pragma once

#include "bsp.h"

class IGraphicsEngine {
public:
    virtual void Initialize(int nScreenWidth, int nScreenHeight, bool bFullscreen) = 0;
    virtual void Shutdown() = 0;

    virtual void ClearScreen() = 0;
    virtual void SwapScreen() = 0;
    virtual void DrawPolygonSet(polygon_container const* pPolySet) = 0;
    virtual void DrawBSPTree(bsp_node const* pTree) = 0;

    virtual void SetCameraPosition(vector4 const* pPos) = 0;
    virtual void SetCameraRotation(vector4 const* pRot) = 0;
    virtual void GetCameraPosition(vector4* pPos) = 0;
    virtual void GetCameraRotation(vector4* pRot) = 0;

    virtual float GetFrameTime() = 0;

    virtual void RenderWireframe(bool bEnable) = 0;
};

IGraphicsEngine* GraphicsEngine();
