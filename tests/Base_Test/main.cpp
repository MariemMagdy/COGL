//
// Created by ekin4 on 28/04/2017.
//

#include "../../Constants.h"
#include "../../cogl.h"
#include <exception>

#include <cstdio>  /* defines FILENAME_MAX */
#ifdef WINDOWS
    #include <direct.h>
    #define GetCurrentDir _getcwd
#else
    #include <unistd.h>
    #define GetCurrentDir getcwd
#endif

std::string GetCurrentWorkingDir() {
    char cCurrentPath[FILENAME_MAX];

    if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
    {
        throw std::runtime_error("Current Dir makes no sense");
    }

    cCurrentPath[sizeof(cCurrentPath) - 1] = '\0'; /* not really required */

    return cCurrentPath;
}

int main() {
    std::cout << GetCurrentWorkingDir() << std::endl;
    cogl::GLWindow mainWindow(1, 4, 0, 1, 1024, 768);
    check_gl_error();
    mainWindow.enableCapability(GL_VERTEX_PROGRAM_POINT_SIZE);
    check_gl_error();
    mainWindow.enableCapability(GL_DEPTH_TEST);
    check_gl_error();
    mainWindow.enableCapability(GL_CULL_FACE);
    check_gl_error();
    mainWindow.setCullType(GL_BACK);
    check_gl_error();
    mainWindow.setDepthFunction(GL_LESS);
    check_gl_error();
    mainWindow.setAASamples(0);
    check_gl_error();

    cogl::Mesh cube = cogl::Mesh::load_from_obj("dragon.obj");
    cogl::Mesh otherCube = cogl::Mesh::Cube;
    cogl::Vertex test1{0.0f, 0.0f, 0.0f};
    std::vector<cogl::Vertex> list{test1};
    cogl::Mesh test2(list);
    test2.setRenderType(cogl::RenderTypes::Points);

    cogl::MeshInstance cubes(cube, 2500);
    check_gl_error();

    cogl::Camera defaultCamera = cogl::Camera(glm::vec3(1.0f, 1.0f, 0.0f),
                                              glm::vec3({0.f, 0.f, 0.f}),
                                              glm::vec3({0.0f, 1.0f, 0.0f}), cogl::projection::perspective);
    defaultCamera.changeAR(16.0 / 9.0);
    defaultCamera.changeZFar(1000.0);
    defaultCamera.changeZNear(0.001);

    float radius = 3;
    float angle = 0.0;

    for (auto i = 0; i < cubes.activeInstances(); ++i) {
        glm::vec3 dest = {glm::cos(angle), 0.0f, glm::sin(angle)};
        dest *= radius;
        cubes.scaleMesh(i, 0.001, 0.001, 0.001);
        cubes.moveMeshTo(i, dest);
        angle += (float) (2 * PI / cubes.activeInstances());
    }

    defaultCamera.setEventHandling();

    cogl::Shader defShader("cogl/shaders/triTest");
    check_gl_error();
    cogl::Shader solidShader("cogl/shaders/solidColour");
    check_gl_error();
    double previousTime = glfwGetTime();
    int frameCount = 0;
    int frameCounterDebug = 0;
    glm::vec3 target_on_floor;

    while (!mainWindow.shouldClose()) {
        otherCube.moveMeshTo(target_on_floor);
        check_gl_error();
        cubes.rotateMesh(-1, PI * 0.1f, glm::vec3(0.0f, 1.0f, 0.0f));
        check_gl_error();
        mainWindow.renderBegin();
        check_gl_error();


        target_on_floor = defaultCamera.getCameraTargetPosition();
        otherCube.render(solidShader, defaultCamera);
        cubes.render(defShader, defaultCamera);
        mainWindow.renderEnd();
        double currentTime = glfwGetTime();
        frameCount++;
        if (currentTime - previousTime >= 1.0) {
            // Display the frame count here any way you want.
            mainWindow.setTitle(std::to_string(((double) frameCount) / (currentTime - previousTime)) + "FPS");
            previousTime = glfwGetTime();
            frameCount = 0;
        }
        ++frameCounterDebug;
    };

    return 0;
}