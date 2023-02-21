/*
* This head file is part of the experiment, Trajectory Queries Using Rasterization.
* This file helps to create OpenGL context in a simple way.
* We use macro to simplize OpenGL context creation. We may extract them as class later.
* Copyright (c) 2023, Bruce Chen, brucechen@whu.edu.cn
*/

#pragma once
#include<glfw3.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define WINDOW_TITLE "TestOpenGL"
#define OPENGL_VERSION_MAJOR 4
#define OPENGL_VERSION_MINOR 6


#define initOpenGL(){\
	glfwInit();\
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_VERSION_MAJOR);\
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_VERSION_MINOR);\
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);\
    \
}

#define createWindow(SCR_WIDTH,SCR_HEIGHT,TITLE) glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, TITLE, NULL, NULL);

#define initWindowAndGlad(window){\
    if (window == NULL)\
    {\
        std::cout << "Failed to create GLFW window" << std::endl;\
        glfwTerminate();\
        return -1;\
    }\
    glfwMakeContextCurrent(window);\
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);\
    \
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))\
    {\
        std::cout << "Failed to initialize GLAD" << std::endl;\
        return -1;\
    }\
}



#define initOpenGLContext(){\
    initOpenGL();\
    GLFWwindow* window = createWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);\
    initWindowAndGlad(window);\
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

