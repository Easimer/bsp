#pragma once

#include "util_vector.h"
#include "util_geostruct.h"

struct bsp_node {
public:
    polygon_container list;
    bsp_node* front;
    bsp_node* back;

    bsp_node() :
        front(NULL), back(NULL) {
    }
};

#define SIDE_FRONT (1)
#define SIDE_ON (0)
#define SIDE_BACK (-1)

int WhichSide(const plane& plane, const vector4& point);
//bool SplitPolygon(polygon* res0, polygon* res1, const polygon& splitted, const plane& splitter);
bool SplitPolygon2(polygon* res0, polygon* res1, const polygon& splitted, const plane& splitter);
polygon_container FanTriangulate(const polygon& poly);
bsp_node* BuildBSPTree(const polygon_container& pc);
bool SplitLine(line* res0, line* res1, vector4* xp, const line& splitted, const plane& splitter);
polygon FromLines(const line_container& lc);
bool PlaneLineIntersection(vector4* res, const line& line, const plane& plane);