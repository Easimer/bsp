#include <assert.h>
#include "bsp.h"

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <glad/glad.h>

void test() {
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
}

int main(int argc, char** argv) {
    SDL_Window* hWnd;
    SDL_GLContext hGLCTX;
    SDL_Renderer* hRenderer;
    bool bDone = false;

    SDL_SetMainReady();
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    hWnd = SDL_CreateWindow("bsp", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    hRenderer = SDL_CreateRenderer(hWnd, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    SDL_GL_SetSwapInterval(-1);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    hGLCTX = SDL_GL_CreateContext(hWnd);

    gladLoadGLLoader(SDL_GL_GetProcAddress);
    glClearColor(0.7, 0.2, 0.3, 1.0);

    while (!bDone) {
        glClear(GL_COLOR_BUFFER_BIT);
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_QUIT:
                bDone = true;
                break;
            case SDL_KEYUP:
                break;
            }
        }

        SDL_GL_SwapWindow(hWnd);
    }

    SDL_GL_DeleteContext(hWnd);
    SDL_DestroyRenderer(hRenderer);
    SDL_DestroyWindow(hWnd);

    SDL_Quit();

    return EXIT_SUCCESS;
}