#include <iostream>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_impl_glfw.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

using namespace std;

class App {
    private:
        GLFWwindow *m_window = {};
        ImGuiIO* m_io = {};
        bool m_cursorHidden = false;

    public:
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