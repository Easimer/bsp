#include <vector>
#include <assert.h>
#include "IGraphicsEngine.h"
#include "IInputHandler.h"

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <glad/glad.h>

#include "util_matrix.h"
#include "util_vector.h"
#include "bsp.h"

#include "stb_image.h"

static bool ReadEntireFileIntoMemory(char const* pchPath, char** pContents, unsigned* pLen) {
    bool bRet = false;

    FILE* hFile = fopen(pchPath, "r+b");

    if (hFile) {
        fseek(hFile, 0, SEEK_END);
        *pLen = ftell(hFile);
        *pContents = new char[((unsigned long)*pLen) + 1];
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
            LoadBasicShader();
            LoadSkybox();

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

    virtual void DrawPolygonSet(PolygonContainer const* pPolySet) override {
        unsigned int iVAO;
        unsigned int aiVBO[2];
        int iMVP, iCamPos, iCamDir;
        long long nTotalVertices = 0;
        math::matrix4 matMVP;
        math::matrix4 matViewRotation = MakeRotationZ(m_vCameraRotation[2]) * MakeRotationY(m_vCameraRotation[1]);
        math::matrix4 matView =
            math::translate(m_vCameraPosition[0], m_vCameraPosition[1], m_vCameraPosition[2]) * matViewRotation;
        PolygonContainer* aPolyConts;
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
        aPolyConts = new PolygonContainer[pPolySet->Count()];
        for (int i = 0; i < pPolySet->Count(); i++) {
            aPolyConts[i] = FanTriangulate(pPolySet->GetPolygon(i));
            for (int j = 0; j < aPolyConts[i].Count(); j++) {
                nTotalVertices += aPolyConts[i][j].Count();
            }
        }

        // Upload triangles into one VAO/VBO
        nTotalFloats = nTotalVertices * 3;
        aflPositions = new float[nTotalFloats];
        aflNormals = new float[nTotalFloats];
        iOffArray = 0;
        for (int iPolyContIdx = 0; iPolyContIdx < pPolySet->Count(); iPolyContIdx++) {
            auto& pc = aPolyConts[iPolyContIdx];
            for (int iPolyIdx = 0; iPolyIdx < pc.Count(); iPolyIdx++) {
                auto& poly = pc[iPolyIdx];
                auto normal = poly.GetNormal();
                for (int iVtxIdx = 0; iVtxIdx < poly.Count(); iVtxIdx++, iOffArray += 3) {
                    auto& point = poly[iVtxIdx];
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
                PlaneFromPolygon(pTree->list[0]),
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
            iSide = WhichSide(PlaneFromPolygon(pTree->list[0]), m_vCameraPosition);

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

    int LoadShader(GLuint* pHandle, GLenum iType, const char* pchPath) {
        int ret = 0, res;
        auto iShader = glCreateShader(iType);
        char* pchSource;
        unsigned nSourceLen;
        char pchLog[512];

        if (ReadEntireFileIntoMemory(pchPath, &pchSource, &nSourceLen)) {
            glShaderSource(iShader, 1, &pchSource, NULL);
            glCompileShader(iShader);
            glGetShaderiv(iShader, GL_COMPILE_STATUS, &res);
            FreeFileFromMemory(pchSource);
            if (res) {
                ret = 1;
                *pHandle = iShader;
            } else {
                glGetShaderInfoLog(iShader, 512, NULL, pchLog);
                fprintf(stderr, "Shader compilation has failed: '%s'\n", pchLog);
            }
        }

        return ret;
    }

    int CreateProgram(GLuint* pProgramHandle, GLuint iVertex, GLuint iFragment) {
        int ret = 0, res;
        GLint iShaderProgram;
        char pchLog[512];

        iShaderProgram = glCreateProgram();
        glAttachShader(iShaderProgram, iVertex);
        glAttachShader(iShaderProgram, iFragment);
        glLinkProgram(iShaderProgram);
        glGetProgramiv(iShaderProgram, GL_LINK_STATUS, &res);
        if (res) {
            ret = 1;
            *pProgramHandle = iShaderProgram;
        } else {
            glGetProgramInfoLog(iShaderProgram, 512, NULL, pchLog);
            fprintf(stderr, "Shader program linking has failed: '%s'\n", pchLog);
        }

        return ret;
    }

    void LoadBasicShader() {
        int res;
        GLuint iShaderVertex, iShaderFragment;

        res = LoadShader(&iShaderVertex, GL_VERTEX_SHADER, "data/shaders/basic.vert.glsl");
        res = LoadShader(&iShaderFragment, GL_FRAGMENT_SHADER, "data/shaders/basic.frag.glsl");

        res = CreateProgram(&m_iShaderProgram, iShaderVertex, iShaderFragment);
        glDeleteShader(iShaderVertex);
        glDeleteShader(iShaderFragment);
    }

    void LoadSkybox() {
        int res;
        GLuint iShaderVertex, iShaderFragment, iCubeVAO, iCubeVBO;

        float skyboxVertices[] = {
            // positions          
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f
        };

        res = LoadShader(&iShaderVertex, GL_VERTEX_SHADER, "data/shaders/sky.vert.glsl");
        res = LoadShader(&iShaderFragment, GL_FRAGMENT_SHADER, "data/shaders/sky.frag.glsl");

        res = CreateProgram(&m_iProgramSkybox, iShaderVertex, iShaderFragment);
        glDeleteShader(iShaderVertex);
        glDeleteShader(iShaderFragment);

        glGenVertexArrays(1, &iCubeVAO);
        glGenBuffers(1, &iCubeVBO);
        glBindVertexArray(iCubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, iCubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);

        m_iVAOSkybox = iCubeVAO;
    }

    virtual void RenderWireframe(bool bEnable) override {
        if (bEnable) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }

    virtual int LoadTexture(HTEXTURE* pHandle, char const* pchPath) override {
        int res = 0;
        return res;
    }

    virtual int LoadCubemapTexture(HTEXTURE* pHandle, char const* pchPathFaces[6]) override {
        int ret = 1;
        GLuint iTex;
        int nWidth, nHeight, nChannels;
        unsigned char* pImage;

        assert(pHandle != NULL);
        assert(pchPathFaces != NULL);

        glGenTextures(1, &iTex);
        glBindTexture(GL_TEXTURE_CUBE_MAP, iTex);

        for (int i = 0; i < 6; i++) {
            assert(pchPathFaces[i] != NULL);
            pImage = stbi_load(pchPathFaces[i], &nWidth, &nHeight, &nChannels, STBI_rgb);
            if (pImage) {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                    0, GL_RGB, nWidth, nHeight, 0, GL_RGB, GL_UNSIGNED_BYTE,
                    pImage);
                stbi_image_free(pImage);
            } else {
                ret = 0;
            }
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        *pHandle = m_aTextures.size();
        m_aTextures.push_back(iTex);

        return ret;
    }

    virtual void DrawSkybox(HTEXTURE hCubemapTexture) override {
        GLuint iMVP, iTex;
        auto matViewRotation = 
            MakeRotationZ(m_vCameraRotation[2]) * MakeRotationY(m_vCameraRotation[1]);
       auto matMVP = matViewRotation * m_matProj;
        matMVP.m_flValues[15] = 1;
        glUseProgram(m_iProgramSkybox);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_aTextures[hCubemapTexture]);
        glBindVertexArray(m_iVAOSkybox);

        iMVP = glGetUniformLocation(m_iProgramSkybox, "matMVP");
        iTex = glGetUniformLocation(m_iProgramSkybox, "skybox");
        glUniformMatrix4fv(iMVP, 1, GL_FALSE, matMVP.ptr());
        glUniform1i(iTex, 0);

        glDrawArrays(GL_TRIANGLES, 0, 36);
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
    GLuint m_iProgramSkybox;
    GLuint m_iVAOSkybox;

    bool m_bActionActive[eInputLast] = { false };

    std::vector<GLuint> m_aTextures;
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