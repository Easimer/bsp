#include <assert.h>
#include "IGraphicsEngine.h"
#include "IInputHandler.h"

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <glad/glad.h>

#include "util_matrix.h"
#include "util_vector.h"
#include "bsp.h"

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

static math::matrix4 MakeRotationZ(float flRot) {
    math::matrix4 ret(1.0f);

    ret.idx(0, 0) = cos(flRot);
    ret.idx(0, 1) = sin(flRot);
    ret.idx(1, 0) = -sin(flRot);
    ret.idx(1, 1) = cos(flRot);

    return ret;
}

static math::matrix4 MakeRotationY(float flRot) {
    math::matrix4 ret(1.0f);

    ret.idx(0, 0) = cos(flRot);
    ret.idx(0, 2) = sin(flRot);
    ret.idx(2, 0) = -sin(flRot);
    ret.idx(2, 2) = cos(flRot);

    return ret;
}

class CSDL2Core : public IGraphicsEngine, public IInputHandler {
public:
    virtual void Initialize(int nScreenWidth, int nScreenHeight, bool bFullscreen) override {
        if (m_bShutdown) {
            SDL_SetMainReady();
            SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

            m_pWnd = SDL_CreateWindow("bsp",
                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                nScreenWidth, nScreenHeight,
                SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN |
                (bFullscreen ? SDL_WINDOW_FULLSCREEN : 0));
            m_pRenderer = SDL_CreateRenderer(m_pWnd, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
            SDL_GL_SetSwapInterval(-1);
            //SDL_SetRelativeMouseMode(SDL_TRUE);

            m_pGLCTX = SDL_GL_CreateContext(m_pWnd);
            gladLoadGLLoader(SDL_GL_GetProcAddress);

            SetupProjection(nScreenWidth, nScreenHeight, M_PI / 4.0f);
            LoadShaders();
        }
    }

    virtual void Shutdown() override {
        if (!m_bShutdown) {
            SDL_GL_DeleteContext(m_pGLCTX);
            SDL_DestroyRenderer(m_pRenderer);
            SDL_DestroyWindow(m_pWnd);

            SDL_Quit();
            m_bShutdown = true;
        }
    }

    virtual void ClearScreen() override {
        glClear(GL_COLOR_BUFFER_BIT);
    }

    math::matrix4 GenerateViewMatrix() {
        return math::translate(
            m_vCameraPosition[0], m_vCameraPosition[1], m_vCameraPosition[2])
            * MakeRotationZ(m_vCameraRotation[2])
            * MakeRotationY(m_vCameraRotation[1]);
    }

    virtual void DrawPolygonSet(polygon_container const* pPolySet) override {
        unsigned int iVAO, iVBO;
        int iMVP;
        long long nTotalVertices = 0;
        math::matrix4 matMVP;
        math::matrix4 matView = GenerateViewMatrix();
        // Bind shader
        glUseProgram(m_iShaderProgram);
        // Set up matrices
        matMVP = matView * m_matProj;
        iMVP = glGetUniformLocation(m_iShaderProgram, "matMVP");
        glUniformMatrix4fv(iMVP, 1, GL_FALSE, matMVP.ptr());
        // Triangulate polygons
        polygon_container* aPolyConts = new polygon_container[pPolySet->cnt];
        for (int i = 0; i < pPolySet->cnt; i++) {
            aPolyConts[i] = FanTriangulate(pPolySet->polygons[i]);
            for (int j = 0; j < aPolyConts[i].cnt; j++) {
                nTotalVertices += aPolyConts[i].polygons[j].cnt;
            }
        }
        // Upload triangles into one VAO/VBO
        auto nTotalFloats = nTotalVertices * 3;
        float* aflVertices = new float[nTotalFloats];
        int iOffArray = 0;
        for (int iPolyContIdx = 0; iPolyContIdx < pPolySet->cnt; iPolyContIdx++) {
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

        auto nVerticesSize = nTotalVertices * 3 * sizeof(float);
        glGenVertexArrays(1, &iVAO);
        glBindVertexArray(iVAO);
        glGenBuffers(1, &iVBO);
        glBindBuffer(GL_ARRAY_BUFFER, iVBO);
        glBufferData(GL_ARRAY_BUFFER, nVerticesSize, aflVertices, GL_STREAM_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
        glEnableVertexAttribArray(0);
        delete[] aflVertices;

        // Draw call

        glDrawArrays(GL_TRIANGLES, 0, nTotalVertices);

        glDeleteBuffers(1, &iVBO);
        glDeleteVertexArrays(1, &iVAO);

        delete[] aPolyConts;
    }

    void DrawBSPNode(bsp_node const* pTree) {
        if (pTree) {
            int side = WhichSide(
                PlaneFromPolygon(pTree->list.polygons[0]),
                m_vCameraPosition);
            if (side == SIDE_FRONT) {
                DrawBSPNode(pTree->back);
                DrawPolygonSet(&(pTree->list));
                DrawBSPNode(pTree->front);
            } else if (side == SIDE_BACK) {
                DrawBSPNode(pTree->front);
                DrawPolygonSet(&(pTree->list));
                DrawBSPNode(pTree->back);

            } else if (side == SIDE_ON) {
                DrawBSPNode(pTree->front);
                DrawBSPNode(pTree->back);
            }
        }
    }

    virtual void DrawBSPTree(bsp_node const* pTree) {
        DrawBSPNode(pTree);
    }

    virtual void SetCameraPosition(vector4 const* pPos) override {
        if (pPos) {
            m_vCameraPosition = *pPos;
        }
    }

    virtual void SetCameraRotation(vector4 const* pRot) override {
        if (pRot) {
            m_vCameraRotation = *pRot;
        }
    }

    virtual void GetCameraPosition(vector4* pPos) override {
        if (pPos) {
            *pPos = m_vCameraPosition;
        }
    }

    virtual void GetCameraRotation(vector4* pRot) override {
        if (pRot) {
            *pRot = m_vCameraRotation;
        }
    }

    virtual void Initialize() override {
    }

    eInputAction MapKeyToAction(SDL_Keycode vk) {
        switch (vk) {
        case SDLK_w: return eInputForward;
        case SDLK_s: return eInputBackward;
        case SDLK_a: return eInputTurnLeft;
        case SDLK_d: return eInputTurnRight;
        case SDLK_COMMA: return eInputStrafeLeft;
        case SDLK_PERIOD: return eInputStrafeRight;
        default: return eInputInvalid;
        }
    }

    virtual int GetNextInputAction(eInputAction* pInputAction, int* bRelease) override {
        int ret = 0;
        SDL_Event ev;

        if (pInputAction && bRelease) {
            *pInputAction = eInputInvalid;
            *bRelease = 0;
            if (SDL_PollEvent(&ev)) {
                switch (ev.type) {
                case SDL_KEYDOWN:
                    *pInputAction = MapKeyToAction(ev.key.keysym.sym);
                    *bRelease = 0;
                    if (*pInputAction != eInputInvalid) {
                        m_bActionActive[*pInputAction] = true;
                        ret = 1;
                    }
                    break;
                case SDL_KEYUP:
                    *pInputAction = MapKeyToAction(ev.key.keysym.sym);
                    *bRelease = 1;
                    if (*pInputAction != eInputInvalid) {
                        m_bActionActive[*pInputAction] = false;
                        ret = 1;
                    }
                    break;
                case SDL_QUIT:
                    *pInputAction = eInputQuitGame;
                    m_bActionActive[eInputQuitGame] = true;
                    *bRelease = 1;
                    ret = 1;
                    break;
                }
            }
        }

        return ret;
    }

    virtual int IsPressed(eInputAction eAction) {
        return m_bActionActive[eAction] ? 1 : 0;
    }

    virtual float GetFrameTime() {
        return m_flFrameTime;
    }

    virtual void SwapScreen() {
        m_tLast = m_tNow;
        m_tNow = SDL_GetPerformanceCounter();
        m_flFrameTime = (float)(((m_tNow - m_tLast)) / (double)SDL_GetPerformanceFrequency());

        SDL_GL_SwapWindow(m_pWnd);
        SDL_Delay(30); // TODO:
    }

    void SetupProjection(int nWidth, int nHeight, float flFov) {
        math::matrix4 matProjInv;
        math::perspective(m_matProj, matProjInv, nWidth, nHeight, flFov, 0.01f, 1000.0f);
    }

    void LoadShaders() {
        int res;
        GLint iShaderProgram;
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

        iShaderProgram = glCreateProgram();
        glAttachShader(iShaderProgram, iShaderVertex);
        glAttachShader(iShaderProgram, iShaderFragment);
        glLinkProgram(iShaderProgram);
        glGetProgramiv(iShaderProgram, GL_LINK_STATUS, &res);
        if (!res) {
            glGetProgramInfoLog(iShaderProgram, 512, NULL, pchLog);
            fprintf(stderr, "Shader program linking has failed: '%s'\n", pchLog);
        }

        glDeleteShader(iShaderVertex);
        glDeleteShader(iShaderFragment);

        m_iShaderProgram = iShaderProgram;
    }
private:
    SDL_Window* m_pWnd;
    SDL_GLContext m_pGLCTX;
    SDL_Renderer* m_pRenderer;
    bool m_bShutdown = true;

    Uint64 m_tNow, m_tLast;
    float m_flFrameTime;

    math::matrix4 m_matProj;
    vector4 m_vCameraPosition, m_vCameraRotation;

    GLuint m_iShaderProgram;

    bool m_bActionActive[eInputLast] = { false };
};

static CSDL2Core* gpSDL2Core = NULL;

IGraphicsEngine* GraphicsEngine() {
    if (!gpSDL2Core) {
        gpSDL2Core = new CSDL2Core;
    }
    return gpSDL2Core;
}

IInputHandler* Input() {
    if (!gpSDL2Core) {
        gpSDL2Core = new CSDL2Core;
    }
    return gpSDL2Core;
}