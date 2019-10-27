#include <assert.h>
#include "bsp.h"

int main(int argc, char** argv) {
    polygon test;
    plane p(vector4(0, 0, 0), vector4(0, 0, 1), vector4(1, 0, 1));
    line l0, l1;
    polygon p0, p1;

    test += vector4(-1, -1, 0);
    test += vector4(1, -1, 0);
    test += vector4(1, 1, 0);
    test += vector4(-1, 1, 0);

    SplitPolygon(&p0, &p1, test, p);

    printf("Polygon:\n");
    for (auto line : test) {
        printf("(%f, %f, %f) -> (%f, %f, %f)\n",
            line.p[0][0], line.p[0][1], line.p[0][2],
            line.p[1][0], line.p[1][1], line.p[1][2]);
    }
    printf("Result No. 1:\n");
    for (auto line : p0) {
        printf("(%f, %f, %f) -> (%f, %f, %f)\n",
            line.p[0][0], line.p[0][1], line.p[0][2],
            line.p[1][0], line.p[1][1], line.p[1][2]);
    }

    printf("Result No. 2:\n");
    for (auto line : p1) {
        printf("(%f, %f, %f) -> (%f, %f, %f)\n",
            line.p[0][0], line.p[0][1], line.p[0][2],
            line.p[1][0], line.p[1][1], line.p[1][2]);
    }

    return 0;
}