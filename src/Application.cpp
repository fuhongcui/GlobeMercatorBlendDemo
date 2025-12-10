#include "Application.h"
#include "TileRenderer.h"

#include <algorithm>
#include <cmath>
#include <iostream>

Application::Application() : renderer(nullptr)
{
    initGLFW();
    initOpenGL();
    // OpenGL 上下文创建后，初始化 renderer
    renderer = new TileRenderer();
}

Application::~Application()
{
    delete renderer;
    glfwTerminate();
}

void Application::run()
{
    std::cout << "\n=== Globe-Mercator Transition Demo ===" << std::endl;
    std::cout << "UP/DOWN: pan latitude" << std::endl;
    std::cout << "LEFT/RIGHT: pan longitude" << std::endl;
    std::cout << "W/S: adjust transition (0=flat, 1=globe)" << std::endl;
    std::cout << "+/-: zoom" << std::endl;
    std::cout << "ESC: quit\n" << std::endl;
    
    while (!glfwWindowShouldClose(window))
    {
        render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Application::initGLFW()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        exit(-1);
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    window = glfwCreateWindow(windowWidth, windowHeight, "Globe Transition Test [UP/DOWN: lat, LEFT/RIGHT: lon, W/S: transition, +/-: zoom]", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create window" << std::endl;
        glfwTerminate();
        exit(-1);
    }
    
    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
}

void Application::initOpenGL()
{
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        exit(-1);
    }
    // 深度测试设置（maplibre 使用标准设置）
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);  // 默认值，但显式设置以确保一致性
    glDepthMask(GL_TRUE);  // 启用深度写入
    glViewport(0, 0, windowWidth, windowHeight);
}

void Application::render()
{
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    float aspect = static_cast<float>(windowWidth) / windowHeight;
    renderer->render(projection, aspect);
}

void Application::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        switch (key)
        {
        case GLFW_KEY_UP:
            app->projection.centerLat = std::min(85.0f, app->projection.centerLat + 3);
            break;
        case GLFW_KEY_DOWN:
            app->projection.centerLat = std::max(-85.0f, app->projection.centerLat - 3);
            break;
        case GLFW_KEY_LEFT:
            app->projection.centerLon -= 5;  // 不 wrap，允许连续旋转
            break;
        case GLFW_KEY_RIGHT:
            app->projection.centerLon += 5;
            break;
        case GLFW_KEY_W:
            app->projection.transition = std::min(1.0f, app->projection.transition + 0.02f);
            break;
        case GLFW_KEY_S:
            app->projection.transition = std::max(0.0f, app->projection.transition - 0.02f);
            break;
        case GLFW_KEY_EQUAL:
        case GLFW_KEY_KP_ADD:
            app->projection.zoom = std::min(6.0f, app->projection.zoom + 0.2f);
            break;
        case GLFW_KEY_MINUS:
        case GLFW_KEY_KP_SUBTRACT:
            app->projection.zoom = std::max(0.0f, app->projection.zoom - 0.2f);
            break;
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, true);
            break;
        }
        
        // 显示当前状态
        float displayLon = fmod(app->projection.centerLon, 360.0f);
        if (displayLon > 180.0f) displayLon -= 360.0f;
        if (displayLon < -180.0f) displayLon += 360.0f;

        std::cout << "Transition: " << app->projection.transition << " | Lon: " << displayLon << " | Lat: " << app->projection.centerLat << " | Zoom: " << app->projection.zoom << std::endl;
    }
}

void Application::framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    app->windowWidth = width;
    app->windowHeight = height;
    glViewport(0, 0, width, height);
}

