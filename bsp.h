#pragma once

#include "util_vector.h"
#include "util_geostruct.h"

struct bsp_node {
public:
    PolygonContainer list;
    bsp_node* front;
    bsp_node* back;

    bsp_node() :
        front(NULL), back(NULL) {
    }
};

#define SIDE_FRONT (1)
#define SIDE_ON (0)
#define SIDE_BACK (-1)

int WhichSide(const Plane& plane, const vector4& point);
bool SplitPolygon2(Polygon* res0, Polygon* res1, const Polygon& splitted, const Plane& splitter);
PolygonContainer FanTriangulate(const Polygon& poly);
bsp_node* BuildBSPTree(const PolygonContainer& pc);
bool SplitLine(Line* res0, Line* res1, vector4* xp, const Line& splitted, const Plane& splitter);
Polygon FromLines(const LineContainer& lc);
bool PlaneLineIntersection(vector4* res, const Line& line, const Plane& plane);