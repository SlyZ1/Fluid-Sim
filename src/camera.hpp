#ifndef CAMERA
#define CAMERA
#include <glm/glm.hpp>

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
        Camera(float moveSensitivity, float lookSensitivity) 
            : m_moveSensitivity(moveSensitivity), m_lookSensitivity(lookSensitivity) {
                m_pos = glm::vec3(0.0f, 0.0f, 1000.0f);
            }
        void move(const CameraMoveInputs& inputs, float dt);
        void rotate(float mouseX, float mouseY);
        void resetMousePos(float mouseX, float mouseY);
        glm::vec3 lookDir();
        glm::vec3 position() const { return m_pos; }
        void setPosition(glm::vec3 newPos) { m_pos = newPos; }
        bool getIsMoving(int frame);
        void hasStoppedMoving() { m_isMoving = false; m_isLooking = false; }
        CameraProperties* getCameraProperties() { return &m_camProps; }
        glm::mat4 viewMatrix();
};

#endif