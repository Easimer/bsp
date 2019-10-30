#include <assert.h>
#include "IGraphicsEngine.h"
#include "IInputHandler.h"

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <glad/glad.h>

#include "util_matrix.h"
#include "util_vector.h"
#include "bsp.h"

static bool ReadEntireFileIntoMemory(char const* pchPath, char** pContents, unsigned* pLen) {
    bool bRet = false;

    FILE* hFile = fopen(pchPath, "r+b");

    if (hFile) {
        fseek(hFile, 0, SEEK_END);
        *pLen = ftell(hFile);
        *pContents = new char[(*pLen) + 1];
        fseek(hFile, 0, SEEK_SET);
        fread(*pContents, 1, *pLen, hFile);
        (*pContents)[(*pLen)] = 0;
        fclose(hFile);
        bRet = true;
    }

    return bRet;
}

static void FreeFileFromMemory(char* pContents) {
    free(pContents);
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

            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            glFrontFace(GL_CCW);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            glClearDepth(1.0f);
            glViewport(0, 0, nScreenWidth, nScreenHeight);
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
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    virtual void DrawPolygonSet(polygon_container const* pPolySet) override {
        unsigned int iVAO;
        unsigned int aiVBO[2];
        int iMVP, iCamPos, iCamDir;
        long long nTotalVertices = 0;
        math::matrix4 matMVP;
        math::matrix4 matViewRotation = MakeRotationZ(m_vCameraRotation[2]) * MakeRotationY(m_vCameraRotation[1]);
        math::matrix4 matView =
            math::translate(m_vCameraPosition[0], m_vCameraPosition[1], m_vCameraPosition[2]) * matViewRotation;
        polygon_container* aPolyConts;
        unsigned nTotalFloats, nVerticesSize;
        float* aflPositions;
        float* aflNormals;
        int iOffArray;
        vector4 vCamViewDir = matViewRotation * vector4(0, 0, -1);

        //fprintf(stderr, "Cam dir: %f %f %f\n", vCamViewDir[0], vCamViewDir[1], vCamViewDir[2]);

        // Bind shader
        glUseProgram(m_iShaderProgram);
        // Set up matrices
        matMVP = matView * m_matProj;

        iMVP = glGetUniformLocation(m_iShaderProgram, "matMVP");
        iCamPos = glGetUniformLocation(m_iShaderProgram, "posCamera");
        iCamDir = glGetUniformLocation(m_iShaderProgram, "dirCamera");
        glUniformMatrix4fv(iMVP, 1, GL_FALSE, matMVP.ptr());
        glUniform4fv(iCamPos, 1, m_vCameraPosition.v);
        glUniform4fv(iCamDir, 1, vCamViewDir.v);

        // Triangulate polygons
        aPolyConts = new polygon_container[pPolySet->cnt];
        for (int i = 0; i < pPolySet->cnt; i++) {
            aPolyConts[i] = FanTriangulate(pPolySet->polygons[i]);
            for (int j = 0; j < aPolyConts[i].cnt; j++) {
                nTotalVertices += aPolyConts[i].polygons[j].cnt;
            }
        }

        // Upload triangles into one VAO/VBO
        nTotalFloats = nTotalVertices * 3;
        aflPositions = new float[nTotalFloats];
        aflNormals = new float[nTotalFloats];
        iOffArray = 0;
        for (int iPolyContIdx = 0; iPolyContIdx < pPolySet->cnt; iPolyContIdx++) {
            auto& pc = aPolyConts[iPolyContIdx];
            for (int iPolyIdx = 0; iPolyIdx < pc.cnt; iPolyIdx++) {
                auto& poly = pc.polygons[iPolyIdx];
                auto normal = poly.GetNormal();
                for (int iVtxIdx = 0; iVtxIdx < poly.cnt; iVtxIdx++, iOffArray += 3) {
                    auto& point = poly.points[iVtxIdx];
                    aflPositions[iOffArray + 0] = point[0];
                    aflPositions[iOffArray + 1] = point[1];
                    aflPositions[iOffArray + 2] = point[2];
                    aflNormals[iOffArray + 0] = normal[0];
                    aflNormals[iOffArray + 1] = normal[1];
                    aflNormals[iOffArray + 2] = normal[2];
                }
            }
        }

        nVerticesSize = nTotalVertices * 3 * sizeof(float);
        glGenVertexArrays(1, &iVAO);
        glBindVertexArray(iVAO);
        glGenBuffers(2, aiVBO);

        glBindBuffer(GL_ARRAY_BUFFER, aiVBO[0]);
        glBufferData(GL_ARRAY_BUFFER, nVerticesSize, aflPositions, GL_STREAM_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, aiVBO[1]);
        glBufferData(GL_ARRAY_BUFFER, nVerticesSize, aflNormals, GL_STREAM_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
        glEnableVertexAttribArray(1);

        delete[] aflPositions;
        delete[] aflNormals;

        // Draw call

        glDrawArrays(GL_TRIANGLES, 0, nTotalVertices);

        glDeleteBuffers(2, aiVBO);
        glDeleteVertexArrays(1, &iVAO);

        delete[] aPolyConts;
    }

    void DrawBSPNodeBackToFront(bsp_node const* pTree) {
        if (pTree) {
            int side = WhichSide(
                PlaneFromPolygon(pTree->list.polygons[0]),
                m_vCameraPosition);
            if (side == SIDE_FRONT) {
                DrawBSPNodeBackToFront(pTree->back);
                DrawPolygonSet(&(pTree->list));
                DrawBSPNodeBackToFront(pTree->front);
            } else if (side == SIDE_BACK) {
                DrawBSPNodeBackToFront(pTree->front);
                DrawPolygonSet(&(pTree->list));
                DrawBSPNodeBackToFront(pTree->back);

            } else if (side == SIDE_ON) {
                DrawBSPNodeBackToFront(pTree->front);
                DrawBSPNodeBackToFront(pTree->back);
            }
        }
    }

    void DrawBSPNodeFrontToBack(bsp_node const* pTree) {
        int iSide;
        if (pTree) {
            iSide = WhichSide(PlaneFromPolygon(pTree->list.polygons[0]), m_vCameraPosition);

            switch (iSide) {
            case SIDE_FRONT:
                DrawBSPNodeFrontToBack(pTree->front);
                DrawPolygonSet(&(pTree->list));
                DrawBSPNodeFrontToBack(pTree->back);
                break;
            case SIDE_BACK:
                DrawBSPNodeFrontToBack(pTree->back);
                DrawPolygonSet(&(pTree->list));
                DrawBSPNodeFrontToBack(pTree->front);
                break;
            case SIDE_ON:
                DrawBSPNodeBackToFront(pTree->back);
                DrawBSPNodeBackToFront(pTree->front);
                break;
            }
        }
    }

    virtual void DrawBSPTree(bsp_node const* pTree) {
        DrawBSPNodeFrontToBack(pTree);
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

        unsigned nVertexLen, nFragmentLen;
        char* pchVertexSource;
        char* pchFragmentSource;

        ReadEntireFileIntoMemory("data/shaders/basic.vert.glsl", &pchVertexSource, &nVertexLen);
        ReadEntireFileIntoMemory("data/shaders/basic.frag.glsl", &pchFragmentSource, &nFragmentLen);

        glShaderSource(iShaderVertex, 1, &pchVertexSource, NULL);
        glShaderSource(iShaderFragment, 1, &pchFragmentSource, NULL);
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

        FreeFileFromMemory(pchFragmentSource);
        FreeFileFromMemory(pchVertexSource);
    }

    virtual void RenderWireframe(bool bEnable) override {
        if (bEnable) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
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