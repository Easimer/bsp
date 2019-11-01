#pragma once

#include "bsp.h"

bool IntersectSegmentByPlane(Line* pLineFront, Line* pLineBack,
    vector4* pXP,
    const Line& line, const Plane& P);

bool IntersectSegmentByPlane(Line* pLineFront, Line* pLineBack,
    vector4* pXP,
    const Line& line, const float aflPlaneCoefficients[4]);