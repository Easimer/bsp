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

    campos = campos + ds;
    camrot = camrot + dtheta;

    GraphicsEngine()->SetCameraPosition(&campos);
    GraphicsEngine()->SetCameraRotation(&camrot);

    return ret;
}

int main(int argc, char** argv) {
    bool bDone = false;
    polygon_container pc;

    polygon square;
    square += {-1, -1};
    square += {1, -1};
    square += {1, 1};
    square += {-1, 1};
    pc += square;

    GraphicsEngine()->Initialize(800, 600, false);
    Input()->Initialize();

    while (!bDone) {
        eInputAction eInput;
        int bRelease;
        while (Input()->GetNextInputAction(&eInput, &bRelease));

        bDone = MoveCamera();

        GraphicsEngine()->ClearScreen();
        GraphicsEngine()->DrawPolygonSet(&pc);
        GraphicsEngine()->SwapScreen();
    }

    Input()->Shutdown();
    GraphicsEngine()->Shutdown();


    return EXIT_SUCCESS;
}