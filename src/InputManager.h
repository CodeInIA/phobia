#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <functional>

// Forward declaration
class Game;

// Input action flags (what the player wants to do)
struct InputState {
    // Movement
    bool moveForward  = false;
    bool moveBackward = false;
    bool moveLeft     = false;
    bool moveRight    = false;
    
    // Actions (one-shot, reset after processing)
    bool toggleFlashlight = false;
    bool toggleTopView    = false;
    bool interact         = false;  // E key for doors
    bool quit             = false;
    
    // Mouse delta (accumulated between frames)
    float mouseDeltaX = 0.0f;
    float mouseDeltaY = 0.0f;
    
    // Reset one-shot actions after processing
    void resetActions() {
        toggleFlashlight = false;
        toggleTopView = false;
        interact = false;
        quit = false;
        mouseDeltaX = 0.0f;
        mouseDeltaY = 0.0f;
    }
};

class InputManager {
public:
    InputManager();
    ~InputManager() = default;

    // Initialize with window (sets callbacks)
    void init(GLFWwindow* window, Game* game);

    // Poll continuous input (WASD keys) - call each frame
    void pollInput(GLFWwindow* window);

    // Get current input state
    const InputState& getState() const { return m_state; }
    InputState& getState() { return m_state; }

    // Reset one-shot actions (call after processing)
    void resetActions() { m_state.resetActions(); }

    // Window resize info
    int getWindowWidth() const { return m_windowWidth; }
    int getWindowHeight() const { return m_windowHeight; }

    // Static callbacks (forward to instance methods via Game*)
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);

private:
    // Instance methods called by static callbacks
    void handleKey(int key, int scancode, int action, int mods);
    void handleCursorPos(double xpos, double ypos);
    void handleScroll(double xoffset, double yoffset);
    void handleFramebufferSize(int width, int height);

    InputState m_state;
    
    // Mouse tracking
    float m_lastMouseX = 500.0f;
    float m_lastMouseY = 500.0f;
    bool m_firstMouse = true;
    float m_mouseSensitivity = 0.1f;

    // Window dimensions
    int m_windowWidth = 1000;
    int m_windowHeight = 1000;
};

#endif // INPUT_MANAGER_H
