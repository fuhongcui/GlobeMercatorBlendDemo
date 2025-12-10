#pragma once
#include "GlobeProjection.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>

class TileRenderer;

class Application {
private:
    GLFWwindow* window;
    int windowWidth = 1920;
    int windowHeight = 1080;
    
    GlobeProjection projection;
    TileRenderer* renderer;  // 使用指针，延迟初始化
    
public:
    Application();
    ~Application();
    
    void run();
    
private:
    void initGLFW();
    void initOpenGL();
    void render();
    
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
};
