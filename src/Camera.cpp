#include "Camera.h"
#include <cmath>

Camera::Camera()
    : m_position(4.0f, 2.0f, 4.0f)
    , m_front(0.0f, 0.0f, -1.0f)
    , m_up(0.0f, 1.0f, 0.0f)
    , m_right(1.0f, 0.0f, 0.0f)
    , m_worldUp(0.0f, 1.0f, 0.0f)
    , m_yaw(-90.0f)
    , m_pitch(0.0f)
    , m_lastX(500.0f)
    , m_lastY(500.0f)
    , m_firstMouse(true)
{
    updateVectors();
}

Camera::Camera(glm::vec3 position, float yaw, float pitch)
    : m_position(position)
    , m_front(0.0f, 0.0f, -1.0f)
    , m_up(0.0f, 1.0f, 0.0f)
    , m_right(1.0f, 0.0f, 0.0f)
    , m_worldUp(0.0f, 1.0f, 0.0f)
    , m_yaw(yaw)
    , m_pitch(pitch)
    , m_lastX(500.0f)
    , m_lastY(500.0f)
    , m_firstMouse(true)
{
    updateVectors();
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

glm::vec3 Camera::getFrontXZ() const {
    return glm::normalize(glm::vec3(m_front.x, 0.0f, m_front.z));
}

glm::vec3 Camera::getRightXZ() const {
    return glm::normalize(glm::cross(m_front, m_worldUp));
}

void Camera::setPosition(glm::vec3 position) {
    m_position = position;
}

void Camera::move(glm::vec3 offset) {
    m_position += offset;
}

void Camera::moveForward(float delta) {
    m_position += getFrontXZ() * delta;
}

void Camera::moveBackward(float delta) {
    m_position -= getFrontXZ() * delta;
}

void Camera::moveLeft(float delta) {
    m_position -= getRightXZ() * delta;
}

void Camera::moveRight(float delta) {
    m_position += getRightXZ() * delta;
}

void Camera::updateOrientation(float deltaYaw, float deltaPitch) {
    m_yaw += deltaYaw;
    m_pitch += deltaPitch;

    // Clamp pitch to prevent flipping
    if (m_pitch > 89.0f) m_pitch = 89.0f;
    if (m_pitch < -89.0f) m_pitch = -89.0f;

    updateVectors();
}

void Camera::processMouseMovement(float xpos, float ypos, float sensitivity) {
    if (m_firstMouse) {
        m_lastX = xpos;
        m_lastY = ypos;
        m_firstMouse = false;
    }

    float xoffset = (xpos - m_lastX) * sensitivity;
    float yoffset = (m_lastY - ypos) * sensitivity; // Reversed: y goes bottom to top
    
    m_lastX = xpos;
    m_lastY = ypos;

    updateOrientation(xoffset, yoffset);
}

void Camera::updateVectors() {
    // Calculate new front vector from yaw and pitch
    glm::vec3 front;
    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_front = glm::normalize(front);

    // Recalculate right and up vectors
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));
}
