#ifndef CAMERA_HPP
#define CAMERA_HPP
#include <glm/glm.hpp>

#include "core/app.hpp"

struct CameraProperties {
    float fov;
    float aperture;
    float focalLength;
};

struct CameraMoveInputs {
    bool forward;
    bool backward;
    bool right;
    bool left;
    bool up;
    bool down;
    bool sprinting;
    bool slowing;
};

class Camera {
    private:
        std::shared_ptr<App> m_app;

        float m_fov = 0;
        float m_moveSensitivity = 0;
        float m_lookSensitivity = 0;
        glm::vec3 m_pos = glm::vec3(0, 2, 1000);
        glm::vec2 m_angles = glm::vec2(0, 0);
        float m_lastMouseX = 0;
        float m_lastMouseY = 0;
        bool m_isMoving = false;
        bool m_isLooking = false;
        int m_lastMovingFrame = 0;

        CameraProperties m_camProps = { 50.0f, 0.0f, 1.0f };

    public:
        Camera(std::shared_ptr<App> app, float fov, float moveSensitivity, float lookSensitivity) 
            : m_app(app), m_fov(fov), m_moveSensitivity(moveSensitivity), m_lookSensitivity(lookSensitivity) {
                m_pos = glm::vec3(0.0f, 0.0f, 1000.0f);
            }
        void move(const CameraMoveInputs& inputs, float dt);
        void rotate(float mouseX, float mouseY, float dt);
        void resetMousePos(float mouseX, float mouseY);
        glm::vec3 lookDir() const;
        glm::vec3 position() const { return m_pos; }
        void setPosition(glm::vec3 newPos) { m_pos = newPos; }
        bool getIsMoving(int frame);
        void hasStoppedMoving() { m_isMoving = false; m_isLooking = false; }
        CameraProperties* getCameraProperties() { return &m_camProps; }
        float getFov() const {return m_fov;}
        void setFov(float fov) {m_fov = fov;}
        glm::mat4 viewMatrix() const;
        glm::mat4 projectionMatrix() const;
};

#endif