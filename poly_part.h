#pragma once

#include "bsp.h"

bool PartitionPolygonByPlane(Polygon* front, Polygon* back, const Polygon& poly, const Plane& plane);