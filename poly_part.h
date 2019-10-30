#pragma once

#include "bsp.h"

bool PartitionPolygonByPlane(polygon* front, polygon* back, const polygon& poly, const plane& plane);