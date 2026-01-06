#include <GL/glew.h>
#include "InputManager.h"
#include "Game.h"
#include <iostream>

InputManager::InputManager()
    : m_state()
    , m_lastMouseX(500.0f)
    , m_lastMouseY(500.0f)
    , m_firstMouse(true)
    , m_mouseSensitivity(0.1f)
    , m_windowWidth(1000)
    , m_windowHeight(1000)
{
}

void InputManager::init(GLFWwindow* window, Game* game) {
    // Store Game pointer in window user data for callbacks
    glfwSetWindowUserPointer(window, game);
    
    // Set callbacks
    glfwSetKeyCallback(window, InputManager::keyCallback);
    glfwSetCursorPosCallback(window, InputManager::cursorPosCallback);
    glfwSetScrollCallback(window, InputManager::scrollCallback);
    glfwSetFramebufferSizeCallback(window, InputManager::framebufferSizeCallback);
    
    // Capture mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void InputManager::pollInput(GLFWwindow* window) {
    // Poll continuous movement keys
    m_state.moveForward  = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
    m_state.moveBackward = (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);
    m_state.moveLeft     = (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
    m_state.moveRight    = (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
}

// Static callbacks - retrieve Game* from window user pointer

void InputManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
    if (game) {
        game->getInputManager().handleKey(key, scancode, action, mods);
    }
}

void InputManager::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
    if (game) {
        game->getInputManager().handleCursorPos(xpos, ypos);
    }
}

void InputManager::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
    if (game) {
        game->getInputManager().handleScroll(xoffset, yoffset);
    }
}

void InputManager::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
    if (game) {
        game->getInputManager().handleFramebufferSize(width, height);
    }
}

// Instance handlers

void InputManager::handleKey(int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                m_state.quit = true;
                break;
            case GLFW_KEY_F:
                m_state.toggleFlashlight = true;
                break;
            case GLFW_KEY_U:
                m_state.toggleTopView = true;
                break;
            case GLFW_KEY_E:
                m_state.interact = true;
                break;
        }
    }
}

void InputManager::handleCursorPos(double xpos, double ypos) {
    if (m_firstMouse) {
        m_lastMouseX = static_cast<float>(xpos);
        m_lastMouseY = static_cast<float>(ypos);
        m_firstMouse = false;
    }

    float xoffset = (static_cast<float>(xpos) - m_lastMouseX) * m_mouseSensitivity;
    float yoffset = (m_lastMouseY - static_cast<float>(ypos)) * m_mouseSensitivity; // Reversed

    m_lastMouseX = static_cast<float>(xpos);
    m_lastMouseY = static_cast<float>(ypos);

    // Accumulate mouse delta for this frame
    m_state.mouseDeltaX += xoffset;
    m_state.mouseDeltaY += yoffset;
}

void InputManager::handleScroll(double xoffset, double yoffset) {
    // Currently unused, but available for future features (zoom, etc.)
}

void InputManager::handleFramebufferSize(int width, int height) {
    glViewport(0, 0, width, height);
    m_windowWidth = width;
    m_windowHeight = height;
}
