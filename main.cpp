#include <cmath>
#include <assert.h>
#include "bsp.h"
#include "util_vector.h"
#include "util_matrix.h"

#include "IGraphicsEngine.h"
#include "IInputHandler.h"

#define M_PI (3.1415926f)

static bool MoveCamera() {
    bool ret = false;
    vector4 ds, dtheta;
    vector4 campos, camrot;
    int bRelease;
    float dt = GraphicsEngine()->GetFrameTime();

    GraphicsEngine()->GetCameraPosition(&campos);
    GraphicsEngine()->GetCameraRotation(&camrot);

    if (Input()->IsPressed(eInputForward)) {
        ds = ds + vector4{ 0, 0, 1 };
    }
    if (Input()->IsPressed(eInputBackward)) {
        ds = ds + vector4{ 0, 0, -1 };
    }
    if (Input()->IsPressed(eInputStrafeLeft)) {
        ds = ds + vector4{ -1, 0, 0 };
    }
    if (Input()->IsPressed(eInputStrafeRight)) {
        ds = ds + vector4{ 1, 0, 0 };
    }
    if (Input()->IsPressed(eInputTurnLeft)) {
        dtheta = dtheta + vector4{ 0, 2 * M_PI, 0 };
    }
    if (Input()->IsPressed(eInputTurnRight)) {
        dtheta = dtheta + vector4{ 0, 2 * -M_PI, 0 };
    }
    if (Input()->IsPressed(eInputQuitGame)) {
        ret = true;
    }

    ds = dt * ds;
    dtheta = dt * dtheta;

    campos = campos + MakeRotationY(camrot[1]) * ds;
    camrot = camrot + dtheta;

    GraphicsEngine()->SetCameraPosition(&campos);
    GraphicsEngine()->SetCameraRotation(&camrot);

    return ret;
}

polygon_container From2D(int nPointPairs, int* pPoints) {
    polygon_container ret;

    for (int i = 0; i < nPointPairs; i++) {
        polygon sq;
        auto p0x = (float)pPoints[i * 4 + 0];
        auto p0y = (float)pPoints[i * 4 + 1];
        auto p1x = (float)pPoints[i * 4 + 2];
        auto p1y = (float)pPoints[i * 4 + 3];

        sq += {p0x, +0.25f, p0y};
        sq += {p0x, -0.25f, p0y};
        sq += {p1x, -0.25f, p1y};
        sq += {p1x, +0.25f, p1y};

        ret += sq;
    }

    return ret;
}

int main(int argc, char** argv) {
    bool bDone = false;
    polygon_container pc;

    int asd[] = {
        0, 2, 1, 1,
        1, 1, 2, 1,
        2, 1, 3, 2,
        3, 2, 3, 4,
        3, 4, 0, 4,
        0, 4, 0, 2,
    };
    pc = From2D(sizeof(asd) / sizeof(int) / 4, asd);
    auto tree = BuildBSPTree(pc);

    GraphicsEngine()->Initialize(800, 600, false);
    Input()->Initialize();
    GraphicsEngine()->RenderWireframe(false);
    vector4 posCamInit(-0.883, 0, -1.772);
    GraphicsEngine()->SetCameraPosition(&posCamInit);

    while (!bDone) {
        eInputAction eInput;
        int bRelease;
        while (Input()->GetNextInputAction(&eInput, &bRelease));

        bDone = MoveCamera();

        GraphicsEngine()->ClearScreen();
        GraphicsEngine()->DrawBSPTree(tree);
        GraphicsEngine()->SwapScreen();
    }

    Input()->Shutdown();
    GraphicsEngine()->Shutdown();

    return EXIT_SUCCESS;
}