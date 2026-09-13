#ifndef APP_HPP
#define APP_HPP

#include <iostream>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_impl_glfw.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "helpers/metrics.hpp"
#include "helpers/stats.hpp"

using namespace std;

class App : public IStatsProvider {
    private:
        GLFWwindow *m_window = {};
        ImGuiIO* m_io = {};
        bool m_cursorHidden = false;
        
        const string FPS_LABEL = "FPS";
        const string FRAME_TIME_LABEL = "Frame Time";
        CPUTimer m_frameTimer = {};
        FPSCounter m_fpsCounter = {};

    public:
        App() : IStatsProvider("App") {}
        void init(int width, int height, const char *name);
        void setClearColor(float r, float g, float b, float a);
        void startFrame(int frameCount);
        void endFrame();
        bool shouldClose();
        bool keyPressed(int key);
        bool keyPressedOnce(int key, int frame);
        void toggleCursor(bool show);
        bool cursorIsHidden();
        float mouseX();
        float mouseY();
        unsigned int width();
        unsigned int height();
        void terminate();
};

#endif