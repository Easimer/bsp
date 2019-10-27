#include <assert.h>
#include "bsp.h"
#include "util_matrix.h"

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

static polygon_container FanTriangulate(const polygon& poly) {
    polygon_container ret;

    assert(poly.cnt >= 3);

    for (int vertexIdx = 1; vertexIdx < poly.cnt - 2; vertexIdx++) {
        polygon tri;
        tri += poly.points[0];
        tri += poly.points[vertexIdx];
        tri += poly.points[vertexIdx + 1];
        ret += tri;
    }

    polygon last;
    last += poly.points[0];
    last += poly.points[poly.cnt - 2];
    last += poly.points[poly.cnt - 1];
    ret += last;

    return ret;
}

static void tri_test() {
    polygon octagon;
    octagon += vector4(0, 0);
    octagon += vector4(1, 1);
    octagon += vector4(2, 2);
    octagon += vector4(3, 0);
    octagon += vector4(2, 0);
    FanTriangulate(octagon);
}

struct RenderRequest {
    polygon_container const * pcPolys;
    vector4 vCamera, vRotation;
    GLuint iShaderProgram;
    math::matrix4 matProj;
};

static void DrawPolygonSet(const RenderRequest& rr) {
    math::matrix4 matView, matMVP;
    int iMVP;
    unsigned int iVAO, iVBO;
    // Bind shader
    glUseProgram(rr.iShaderProgram);
    // Set up matrices
    matView = math::translate(rr.vCamera[0], rr.vCamera[1], rr.vCamera[2]);
    //matMVP = rr.matProj * matView;
    matMVP = matView * rr.matProj;
    iMVP = glGetUniformLocation(rr.iShaderProgram, "matMVP");
    glUniformMatrix4fv(iMVP, 1, GL_FALSE, matMVP.ptr());
    // Triangulate polygons
    int nTotalVertices = 0;
    polygon_container* aPolyConts = new polygon_container[rr.pcPolys->cnt];
    for (int i = 0; i < rr.pcPolys->cnt; i++) {
        aPolyConts[i] = FanTriangulate(rr.pcPolys->polygons[i]);
        for (int j = 0; j < aPolyConts[i].cnt; j++) {
            nTotalVertices += aPolyConts[i].polygons[j].cnt;
        }
    }
    // Upload triangles into one VAO/VBO
    float* aflVertices = new float[nTotalVertices * 3];
    int iOffArray = 0;
    for (int iPolyContIdx = 0; iPolyContIdx < rr.pcPolys->cnt; iPolyContIdx++) {
        auto& pc = aPolyConts[iPolyContIdx];
        for (int iPolyIdx = 0; iPolyIdx < pc.cnt; iPolyIdx++) {
            auto& poly = pc.polygons[iPolyIdx];
            for (int iVtxIdx = 0; iVtxIdx < poly.cnt; iVtxIdx++, iOffArray += 3) {
                auto& point = poly.points[iVtxIdx];
                aflVertices[iOffArray + 0] = point[0];
                aflVertices[iOffArray + 1] = point[1];
                aflVertices[iOffArray + 2] = point[2];
            }
        }
    }

    glGenVertexArrays(1, &iVAO);
    glBindVertexArray(iVAO);
    glGenBuffers(1, &iVBO);
    glBindBuffer(GL_ARRAY_BUFFER, iVBO);
    glBufferData(GL_ARRAY_BUFFER, nTotalVertices * 3 * sizeof(float), aflVertices, GL_STREAM_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
    glEnableVertexAttribArray(0);
    delete[] aflVertices;

    // Draw call

    glDrawArrays(GL_TRIANGLES, 0, nTotalVertices);

    glDeleteBuffers(1, &iVBO);
    glDeleteVertexArrays(1, &iVAO);

    delete[] aPolyConts;
}

char const* const gpchVertexSource = "                            \n\
#version 330 core                                                 \n\
layout(location = 0) in vec3 aPos;                                \n\
uniform mat4 matMVP;                                              \n\
                                                                  \n\
void main() {                                                     \n\
    gl_Position = matMVP * vec4(aPos.x, aPos.y, aPos.z, 1.0);     \n\
}                                                                 \n\
";

char const* const gpchFragmentSource = "                 \n\
#version 330 core                                        \n\
out vec4 FragColor;                                      \n\
                                                         \n\
void main() {                                            \n\
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);            \n\
}                                                        \n\
";

static void LoadShaders(RenderRequest* rr) {
    int res;
    char pchLog[512];
    auto iShaderVertex = glCreateShader(GL_VERTEX_SHADER);
    auto iShaderFragment = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(iShaderVertex, 1, &gpchVertexSource, NULL);
    glShaderSource(iShaderFragment, 1, &gpchFragmentSource, NULL);
    glCompileShader(iShaderVertex);
    glGetShaderiv(iShaderVertex, GL_COMPILE_STATUS, &res);
    if (!res) {
        glGetShaderInfoLog(iShaderVertex, 512, NULL, pchLog);
        fprintf(stderr, "Vertex shader compilation has failed: '%s'\n", pchLog);
    }
    glCompileShader(iShaderFragment);
    glGetShaderiv(iShaderFragment, GL_COMPILE_STATUS, &res);
    if (!res) {
        glGetShaderInfoLog(iShaderFragment, 512, NULL, pchLog);
        fprintf(stderr, "Fragment shader compilation has failed: '%s'\n", pchLog);
    }

    rr->iShaderProgram = glCreateProgram();
    glAttachShader(rr->iShaderProgram, iShaderVertex);
    glAttachShader(rr->iShaderProgram, iShaderFragment);
    glLinkProgram(rr->iShaderProgram);
    glGetProgramiv(rr->iShaderProgram, GL_LINK_STATUS, &res);
    if (!res) {
        glGetProgramInfoLog(rr->iShaderProgram, 512, NULL, pchLog);
        fprintf(stderr, "Shader program linking has failed: '%s'\n", pchLog);
    }

    glDeleteShader(iShaderVertex);
    glDeleteShader(iShaderFragment);
}

static void SetupProjection(int width, int height, RenderRequest* rr) {
    math::matrix4 matProjInv;
    math::perspective(rr->matProj, matProjInv, width, height, M_PI / 4.0f, 0.01f, 1000.0f);
}

enum eCameraMovement {
    eCameraInvalid = 0,
    eCameraForward = 1,
    eCameraBackward = 2,
    eCameraStrafeLeft = 4,
    eCameraStrafeRight = 8,
    eCameraTurnLeft = 16,
    eCameraTurnRight = 32,
};

eCameraMovement MapKeyMovement(SDL_Keycode vk) {
    switch (vk) {
    case SDLK_w: return eCameraForward;
    case SDLK_s: return eCameraBackward;
    case SDLK_a: return eCameraTurnLeft;
    case SDLK_d: return eCameraTurnRight;
    case SDLK_COMMA: return eCameraStrafeLeft;
    case SDLK_PERIOD: return eCameraStrafeRight;
    default: return eCameraInvalid;
    }
}

static void MoveCamera(vector4* camera, vector4* rotation, int eMovement, float dt) {
    vector4 ds;

    if (eMovement & eCameraForward) {
        ds = ds + vector4{0, 0, 1};
    }
    if (eMovement & eCameraBackward) {
        ds = ds + vector4{0, 0, -1};
    }
    if (eMovement & eCameraStrafeLeft) {
        ds = ds + vector4{-1, 0, 0};
    }
    if (eMovement & eCameraStrafeRight) {
        ds = ds + vector4{1, 0, 0};
    }

    ds = dt * ds;

    printf("%f\n", dt);

    *camera = *camera + ds;
}

int main(int argc, char** argv) {
    SDL_Window* hWnd;
    SDL_GLContext hGLCTX;
    SDL_Renderer* hRenderer;
    bool bDone = false;
    polygon_container pc;
    RenderRequest rr;
    int nCameraMovement = 0;
    Uint64 tNow, tLast;

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

    LoadShaders(&rr);
    SetupProjection(800, 600, &rr);
    rr.vCamera = { 0, 0, -4 };

    rr.pcPolys = &pc;
    polygon square;
    square += {-1, -1};
    square += {1, -1};
    square += {1, 1};
    square += {-1, 1};
    pc += square;

    glClearColor(0.7, 0.2, 0.3, 1.0);
    tNow = SDL_GetPerformanceCounter();
    tLast = 0;

    while (!bDone) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_QUIT:
                bDone = true;
                break;
            case SDL_KEYUP:
                nCameraMovement &= ~(MapKeyMovement(ev.key.keysym.sym));
                break;
            case SDL_KEYDOWN:
                nCameraMovement |= MapKeyMovement(ev.key.keysym.sym);
                break;
            }
        }

        tLast = tNow;
        tNow = SDL_GetPerformanceCounter();
        float dt = (float)(((tNow - tLast)) / (double)SDL_GetPerformanceFrequency());

        MoveCamera(&rr.vCamera, &rr.vRotation, nCameraMovement, dt);

        glClear(GL_COLOR_BUFFER_BIT);
        DrawPolygonSet(rr);
        SDL_GL_SwapWindow(hWnd);
    }

    SDL_GL_DeleteContext(hWnd);
    SDL_DestroyRenderer(hRenderer);
    SDL_DestroyWindow(hWnd);

    SDL_Quit();

    return EXIT_SUCCESS;
}