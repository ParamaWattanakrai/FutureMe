#undef GLFW_DLL

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstdio>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <vector>

#include "Libs/Mesh.h"
#include "Libs/Shader.h"
#include "Libs/Window.h"
#include "ProjectPaths.h"

const GLint WIDTH = 800;
const GLint HEIGHT = 600;

const std::filesystem::path shaderDirectory =
    std::filesystem::u8path(OPENGL_STARTER_SHADER_DIR);

std::vector<Mesh*> meshList;
std::vector<Shader*> shaderList;

enum MeshId   { MESH_SKY = 0, MESH_SCENE = 1 };
enum ShaderId { SHADER_SKY = 0, SHADER_SCENE = 1 };

// Appends one axis-aligned box (24 verts, 36 indices) to the shared buffers.
// skyBlend/frontSkyBlend (0..1) pick flat colour vs. sky texture per face.
static void AddBox(std::vector<GLfloat>& verts,
                    std::vector<unsigned int>& indices,
                    const glm::vec3& minC,
                    const glm::vec3& maxC,
                    const glm::vec3& colour,
                    float skyBlend = 0.0f,
                    float frontSkyBlend = -1.0f)
{
    if (frontSkyBlend < 0.0f)
        frontSkyBlend = skyBlend;

    const glm::vec3 p[8] =
    {
        { minC.x, minC.y, minC.z }, // 0
        { maxC.x, minC.y, minC.z }, // 1
        { maxC.x, maxC.y, minC.z }, // 2
        { minC.x, maxC.y, minC.z }, // 3
        { minC.x, minC.y, maxC.z }, // 4
        { maxC.x, minC.y, maxC.z }, // 5
        { maxC.x, maxC.y, maxC.z }, // 6
        { minC.x, maxC.y, maxC.z }, // 7
    };

    // 4 corners + outward normal per face (CCW from outside).
    struct Face { int a, b, c, d; glm::vec3 normal; };
    const Face faces[6] =
    {
        { 4, 5, 6, 7, { 0.0f,  0.0f,  1.0f } }, // +Z front
        { 1, 0, 3, 2, { 0.0f,  0.0f, -1.0f } }, // -Z back
        { 0, 4, 7, 3, { -1.0f, 0.0f,  0.0f } }, // -X left
        { 5, 1, 2, 6, { 1.0f,  0.0f,  0.0f } }, // +X right
        { 3, 7, 6, 2, { 0.0f,  1.0f,  0.0f } }, // +Y top
        { 0, 1, 5, 4, { 0.0f, -1.0f,  0.0f } }, // -Y bottom
    };

    for (const Face& f : faces)
    {
        const unsigned int base = static_cast<unsigned int>(verts.size() / 10);
        const int corners[4] = { f.a, f.b, f.c, f.d };
        const float faceSkyBlend = (f.normal.z > 0.5f) ? frontSkyBlend : skyBlend;

        for (int i = 0; i < 4; ++i)
        {
            const glm::vec3& pos = p[corners[i]];
            verts.insert(verts.end(), {
                pos.x, pos.y, pos.z,
                f.normal.x, f.normal.y, f.normal.z,
                colour.r, colour.g, colour.b, faceSkyBlend
            });
        }

        indices.insert(indices.end(), {
            base + 0, base + 1, base + 2,
            base + 2, base + 3, base + 0
        });
    }
}

// Builds the staircase, platform and doorway as one mesh.
void CreateScene()
{
    std::vector<GLfloat> verts;
    std::vector<unsigned int> indices;

    const glm::vec3 stoneWhite(0.90f, 0.91f, 0.90f);
    const glm::vec3 doorwayDark(0.035f, 0.045f, 0.06f);

    // Staircase: solid ramp, each column filled from the ground to its tread.
    const int   stepCount   = 13;
    const float stepRun     = 0.24f;   // horizontal distance per step
    const float stepRise    = 0.20f;   // vertical rise per step
    const float stairStartX = -2.75f;
    const float stairHalfDepth = 1.05f; // matches platform depth, flush join

    for (int i = 0; i < stepCount; ++i)
    {
        const float xMin = stairStartX + i * stepRun;
        const float xMax = stairStartX + (i + 1) * stepRun;
        const float yMin = 0.0f;
        const float yMax = (i + 1) * stepRise;

        AddBox(verts, indices,
               glm::vec3(xMin, yMin, -stairHalfDepth),
               glm::vec3(xMax, yMax,  stairHalfDepth),
               stoneWhite, /*skyBlend=*/0.0f, /*frontSkyBlend=*/1.0f);
    }

    // Platform: podium the doorway stands on, flush with the stairs.
    const float platformMinX = stairStartX + stepCount * stepRun;
    const float platformMaxX = platformMinX + 2.1f;
    const float platformHeight = stepCount * stepRise;
    const float platformHalfDepth = stairHalfDepth;

    AddBox(verts, indices,
           glm::vec3(platformMinX, 0.0f, -platformHalfDepth),
           glm::vec3(platformMaxX, platformHeight, platformHalfDepth),
           stoneWhite, /*skyBlend=*/0.0f, /*frontSkyBlend=*/1.0f);

    // Doorway frame.
    const float doorWidth  = 0.62f;
    const float doorHeight = 1.55f;
    const float doorDepth  = 0.55f;
    const float doorMinX = platformMaxX - 0.95f;
    const float doorMaxX = doorMinX + doorWidth;

    AddBox(verts, indices,
           glm::vec3(doorMinX, platformHeight, -doorDepth * 0.5f),
           glm::vec3(doorMaxX, platformHeight + doorHeight, doorDepth * 0.5f),
           stoneWhite, /*skyBlend=*/1.0f);

    // Recessed dark opening (front and back faces).
    const float inset = 0.028f;
    AddBox(verts, indices,
           glm::vec3(doorMinX + inset, platformHeight + inset * 2.0f, -doorDepth * 0.5f - 0.02f),
           glm::vec3(doorMaxX - inset, platformHeight + doorHeight - inset, -doorDepth * 0.5f + inset),
           doorwayDark);
    AddBox(verts, indices,
           glm::vec3(doorMinX + inset, platformHeight + inset * 2.0f, doorDepth * 0.5f - inset),
           glm::vec3(doorMaxX - inset, platformHeight + doorHeight - inset, doorDepth * 0.5f + 0.02f),
           doorwayDark);

    Mesh* sceneMesh = new Mesh();
    sceneMesh->CreateMeshLit(verts.data(), indices.data(),
                              static_cast<unsigned int>(verts.size()),
                              static_cast<unsigned int>(indices.size()));
    meshList[MESH_SCENE] = sceneMesh;
}

// Full-screen quad for the sky background.
void CreateSkyQuad()
{
    GLfloat vertices[] =
    {
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
    };
    unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };

    Mesh* skyMesh = new Mesh();
    skyMesh->CreateMesh(vertices, indices, 12, 6);
    meshList[MESH_SKY] = skyMesh;
}

void CreateShaders()
{
    Shader* skyShader = new Shader();
    skyShader->CreateFromFiles(shaderDirectory / "sky.vert", shaderDirectory / "sky.frag");
    shaderList[SHADER_SKY] = skyShader;

    Shader* sceneShader = new Shader();
    sceneShader->CreateFromFiles(shaderDirectory / "scene.vert", shaderDirectory / "scene.frag");
    shaderList[SHADER_SCENE] = sceneShader;
}

void Cleanup()
{
    for (Mesh* mesh : meshList)
        delete mesh;
    meshList.clear();

    for (Shader* shader : shaderList)
        delete shader;
    shaderList.clear();
}

// Writes an uncompressed BMP (no extra library needed).
static void SaveScreenshotBMP(const std::filesystem::path& path, int width, int height)
{
    const int rowSize = ((width * 3 + 3) / 4) * 4; // BMP rows pad to 4 bytes
    std::vector<unsigned char> pixels(static_cast<size_t>(rowSize) * height);

    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, pixels.data());

    const uint32_t fileSize = 54 + static_cast<uint32_t>(pixels.size());
    unsigned char header[54] = { 0 };
    header[0] = 'B'; header[1] = 'M';
    *reinterpret_cast<uint32_t*>(&header[2]) = fileSize;
    *reinterpret_cast<uint32_t*>(&header[10]) = 54; // pixel data offset
    *reinterpret_cast<uint32_t*>(&header[14]) = 40; // DIB header size
    *reinterpret_cast<int32_t*>(&header[18]) = width;
    *reinterpret_cast<int32_t*>(&header[22]) = height; // bottom-up rows
    *reinterpret_cast<uint16_t*>(&header[26]) = 1;      // planes
    *reinterpret_cast<uint16_t*>(&header[28]) = 24;     // bits per pixel
    *reinterpret_cast<uint32_t*>(&header[34]) = static_cast<uint32_t>(pixels.size());

    std::ofstream out(path, std::ios::binary);
    out.write(reinterpret_cast<char*>(header), sizeof(header));
    out.write(reinterpret_cast<char*>(pixels.data()), pixels.size());

    std::printf("Saved screenshot: %s\n", path.string().c_str());
}

int main(int argc, char** argv)
{
    Window mainWindow(WIDTH, HEIGHT, 3, 3);
    if (mainWindow.initialise() != 0)
        return 1;

    meshList.resize(2);
    shaderList.resize(2);

    CreateSkyQuad();
    CreateScene();
    CreateShaders();

    // Narrow FOV from far away flattens the perspective, like a long lens.
    const glm::vec3 eyePos(-0.59f, 2.375f, 21.0f);
    const glm::vec3 lookTarget(-0.59f, 2.375f, 0.0f);
    const glm::mat4 view = glm::lookAt(eyePos, lookTarget, glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::mat4 projection = glm::perspective(glm::radians(14.1f),
                                                    (float)WIDTH / (float)HEIGHT,
                                                    0.1f, 100.0f);
    const glm::mat4 model(1.0f); // vertices already authored in world space
    const glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));

    // Single directional "sun" light.
    const glm::vec3 lightDirection = glm::normalize(glm::vec3(-0.45f, -0.85f, -0.35f));
    const glm::vec3 lightColour(1.0f, 0.98f, 0.95f);

    bool autoCaptureMode = (argc > 1);
    std::filesystem::path autoCapturePath = autoCaptureMode ? std::filesystem::u8path(argv[1]) : "";
    int framesRendered = 0;

    while (!mainWindow.getShouldClose())
    {
        glfwPollEvents();

        if (!autoCaptureMode && glfwGetKey(mainWindow.getWindow(), GLFW_KEY_S) == GLFW_PRESS)
            SaveScreenshotBMP("screenshot.bmp", mainWindow.getBufferWidth(), mainWindow.getBufferHeight());

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Sky background, drawn behind the depth-tested scene.
        glDepthMask(GL_FALSE);
        shaderList[SHADER_SKY]->UseShader();
        glUniform1f(shaderList[SHADER_SKY]->GetUniformLocation("time"), (float)glfwGetTime());
        meshList[MESH_SKY]->RenderMesh();
        glDepthMask(GL_TRUE);

        // Lit scene.
        Shader* sceneShader = shaderList[SHADER_SCENE];
        sceneShader->UseShader();
        glUniformMatrix4fv(sceneShader->GetUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(sceneShader->GetUniformLocation("view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(sceneShader->GetUniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix3fv(sceneShader->GetUniformLocation("normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
        glUniform3fv(sceneShader->GetUniformLocation("lightDirection"), 1, glm::value_ptr(lightDirection));
        glUniform3fv(sceneShader->GetUniformLocation("lightColour"), 1, glm::value_ptr(lightColour));
        glUniform3fv(sceneShader->GetUniformLocation("viewPos"), 1, glm::value_ptr(eyePos));
        glUniform1f(sceneShader->GetUniformLocation("ambientStrength"), 0.33f);
        glUniform1f(sceneShader->GetUniformLocation("specularStrength"), 0.15f);
        glUniform1f(sceneShader->GetUniformLocation("shininess"), 24.0f);
        glUniform2f(sceneShader->GetUniformLocation("resolution"),
                    (float)mainWindow.getBufferWidth(), (float)mainWindow.getBufferHeight());
        glUniform1f(sceneShader->GetUniformLocation("time"), (float)glfwGetTime());
        meshList[MESH_SCENE]->RenderMesh();

        glUseProgram(0);

        if (autoCaptureMode)
        {
            glFinish();
            ++framesRendered;
            if (framesRendered >= 3) // let a few frames settle first
            {
                SaveScreenshotBMP(autoCapturePath, mainWindow.getBufferWidth(), mainWindow.getBufferHeight());
                break;
            }
        }

        mainWindow.swapBuffers();
    }

    Cleanup(); // before the Window destructor kills the GL context
    return 0;
}
