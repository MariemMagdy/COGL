//
// Created by ekin4 on 26/02/2017.
//

#include "GLWindow.h"

namespace cogl {

    GLWindow::GLWindow(int _swapInterval, int _contextMajor, int _aaSamples, int _contextMinor, int _windowWidth,
                       int _windowHeight,
                       int _aspectRatioWidth, int _aspectRatioHeight, std::string _windowTitle, std::string pps_file) :
            swapInterval(_swapInterval), contextMajor(_contextMajor), aaSamples(_aaSamples),
            contextMinor(_contextMinor), windowWidth(_windowWidth), windowHeight(_windowHeight),
            aspectRatioWidth(_aspectRatioWidth), aspectRatioHeight(_aspectRatioHeight), windowTitle(_windowTitle) {

        if (!glfwInit()) {
            exit(EXIT_FAILURE);
        }
        glfwWindowHint(GLFW_SAMPLES, 0);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, contextMajor);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, contextMinor);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE); // To make MacOS happy; should not be needed
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL
        glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
        contextHandle = glfwCreateWindow(windowWidth, windowHeight, windowTitle.c_str(), NULL, NULL);
        if (!contextHandle) {
            glfwTerminate();
            exit(EXIT_FAILURE);
        }
        glfwMakeContextCurrent(contextHandle);
        this->setEventHandling();
        glfwSetErrorCallback((GLFWerrorfun) error_callback);
        glfwSetWindowSizeCallback(contextHandle, (GLFWwindowsizefun) StateBaseGLWindow::windowsizecallback_dispatch);
        glfwSetKeyCallback(contextHandle, cogl::StateBase::keycallback_dispatch);
        glfwSetScrollCallback(contextHandle, cogl::StateBase::scrollcallback_dispatch);
        glfwSetWindowAspectRatio(contextHandle, aspectRatioWidth, aspectRatioHeight);
        gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
        glfwSwapInterval(swapInterval);
        check_gl_error();

        nonMSFBO = std::make_unique<Framebuffer>(windowWidth, windowHeight);
        MSFBO = std::make_unique<FramebufferMultisampled>(aaSamples, windowWidth, windowHeight);
        check_gl_error();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glGenVertexArrays(1, &quad_vao);
        glBindVertexArray(quad_vao);
        glGenBuffers(1, &quad_vertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec3), &g_quad_vertex_buffer_data, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glGenBuffers(1, &quad_indexbuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_indexbuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), &g_quad_vertex_buffer_indices, GL_STATIC_DRAW);
        glBindVertexArray(0);
        glDeleteBuffers(1, &quad_vertexbuffer);
        glDeleteBuffers(1, &quad_indexbuffer);
        check_gl_error();

        postProcessingShader = std::make_unique<Shader>(std::string(pps_file));
        check_gl_error();
        windowCreated = true;
    }

    GLWindow::~GLWindow() {
        glDeleteVertexArrays(1, &quad_vao);
        terminateWindow();
    }

    void GLWindow::terminateWindow() {
        glfwDestroyWindow(contextHandle);
        glfwTerminate();
        windowCreated = false;
    }

    int GLWindow::shouldClose() {
        return glfwWindowShouldClose(contextHandle);
    }

    void GLWindow::setKeyCallback(GLFWkeyfun key_callback) {
        glfwSetKeyCallback(contextHandle, key_callback);
    }

    void GLWindow::setCursorPosCallback(GLFWcursorposfun cpos_callback) {
        glfwSetCursorPosCallback(contextHandle, cpos_callback);
    }

    void GLWindow::setScrollCallback(GLFWscrollfun scroll_callback) {
        glfwSetScrollCallback(contextHandle, scroll_callback);
    }

    void GLWindow::handleResizing() {
        nonMSFBO.reset();
        MSFBO.reset();
        nonMSFBO = std::make_unique<Framebuffer>(windowWidth, windowHeight);
        MSFBO = std::make_unique<FramebufferMultisampled>(aaSamples, windowWidth, windowHeight);
    }

    void GLWindow::windowsizecallback(GLFWwindow *window, int width, int height) {
        windowWidth = width;
        windowHeight = height;
        handleResizing();
    }

    void GLWindow::error_callback(int error, const char *description) {
        fprintf(stderr, "Error: %s\n", description);
    }

    void GLWindow::setWidth(int width) {
        windowWidth = width;
        glfwSetWindowAspectRatio(contextHandle, windowWidth, windowHeight);
    }

    void GLWindow::setHeight(int height) {
        windowHeight = height;
        glfwSetWindowAspectRatio(contextHandle, windowWidth, windowHeight);
    }

    void GLWindow::setAASamples(int aaSamp) {
        aaSamples = aaSamp;
        handleResizing();
    }

    void GLWindow::setTitle(const std::string title) {
        windowTitle = title;
        glfwSetWindowTitle(contextHandle, windowTitle.c_str());
    };

    void GLWindow::setTitle(const char *title) {
        windowTitle = std::string(title);
        glfwSetWindowTitle(contextHandle, title);
    };

    void GLWindow::renderBegin() {
        MSFBO->bindFBO();
        glViewport(0, 0, MSFBO->width, MSFBO->height);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    void GLWindow::renderEnd() {
        renderFramebuffers();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glfwSwapBuffers(contextHandle);
        glfwPollEvents();
        if (glfwGetKey(contextHandle, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(contextHandle, GL_TRUE);
        }
    }

    bool GLWindow::enableCapability(GLenum Capability) {
        if (windowCreated) {
            std::unique_lock<std::mutex> lock(paramLock);
            glEnable(Capability);
        }
        return glIsEnabled(Capability);
    }

    bool GLWindow::disableCapability(GLenum Capability) {
        if (windowCreated) {
            std::unique_lock<std::mutex> lock(paramLock);
            glDisable(Capability);
        }
        return !glIsEnabled(Capability);
    }

    bool GLWindow::setDepthFunction(GLenum depthFunc) {
        if (windowCreated) {
            std::unique_lock<std::mutex> lock(paramLock);
            glDepthFunc(depthFunc);
        }
        return glIsEnabled(GL_DEPTH_TEST);
    }

    bool GLWindow::setCullType(GLenum mode) {
        if (windowCreated) {
            std::unique_lock<std::mutex> lock(paramLock);
            glCullFace(mode);
        }
        return glIsEnabled(GL_DEPTH_TEST);
    }

    void GLWindow::renderFramebuffers() {
        MSFBO->bindFBORead();
        nonMSFBO->bindFBODraw();
        glViewport(0, 0, windowWidth, windowHeight);
        glBlitFramebuffer(0, 0, MSFBO->width, MSFBO->height,
                          0, 0, nonMSFBO->width, nonMSFBO->height,
                          GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, windowWidth, windowHeight);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        GLuint fbo_textureID = postProcessingShader->getUniformLoc("fbo_texture");
        GLuint timeID = postProcessingShader->getUniformLoc("time");
        GLuint exposureID = postProcessingShader->getUniformLoc("exposure");
        postProcessingShader->bind();
        glUniform1ui(fbo_textureID, /*GL_TEXTURE*/0);
        glUniform1f(timeID, (float) glfwGetTime());
        glUniform1f(exposureID, (float) 1.0);
        glBindVertexArray(quad_vao);
        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, nonMSFBO->colorBuffers);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        postProcessingShader->unbind();
    }
}