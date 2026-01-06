#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    Camera();
    Camera(glm::vec3 position, float yaw = -90.0f, float pitch = 0.0f);

    // Getters
    glm::vec3 getPosition() const { return m_position; }
    glm::vec3 getFront() const { return m_front; }
    glm::vec3 getUp() const { return m_up; }
    glm::vec3 getRight() const { return m_right; }
    float getYaw() const { return m_yaw; }
    float getPitch() const { return m_pitch; }
    
    // Get view matrix for rendering
    glm::mat4 getViewMatrix() const;
    
    // Get horizontal front (for movement, ignores pitch)
    glm::vec3 getFrontXZ() const;
    glm::vec3 getRightXZ() const;

    // Setters
    void setPosition(glm::vec3 position);
    void setPositionX(float x) { m_position.x = x; }
    void setPositionY(float y) { m_position.y = y; }
    void setPositionZ(float z) { m_position.z = z; }

    // Movement (raw position changes, collision handled externally)
    void move(glm::vec3 offset);
    void moveForward(float delta);
    void moveBackward(float delta);
    void moveLeft(float delta);
    void moveRight(float delta);

    // Mouse look
    void updateOrientation(float deltaYaw, float deltaPitch);
    void processMouseMovement(float xpos, float ypos, float sensitivity = 0.1f);

private:
    void updateVectors();

    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::vec3 m_right;
    glm::vec3 m_worldUp;

    float m_yaw;
    float m_pitch;

    // Mouse tracking
    float m_lastX;
    float m_lastY;
    bool m_firstMouse;
};

#endif // CAMERA_H
