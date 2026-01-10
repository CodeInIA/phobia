#ifndef GAME_H
#define GAME_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <FreeImage.h>
#include <memory>
#include <string>

#include "ResourceManager.h"
#include "InputManager.h"
#include "Scene.h"

class Game {
public:
    Game();
    ~Game();

    // Prevent copying
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    // Initialize game (create window, load resources)
    bool init(int width = 1000, int height = 1000, const std::string& title = "Phobia");

    // Main game loop
    void run();

    // Shutdown
    void shutdown();

    // Access managers (for callbacks)
    InputManager& getInputManager() { return m_inputManager; }
    ResourceManager& getResourceManager() { return m_resourceManager; }
    Scene& getScene() { return *m_scene; }

    // Window access
    GLFWwindow* getWindow() { return m_window; }

private:
    // GLFW window
    GLFWwindow* m_window = nullptr;

    // Core systems
    ResourceManager m_resourceManager;
    InputManager m_inputManager;
    std::unique_ptr<Scene> m_scene;

    // Timing
    float m_lastFrame = 0.0f;

    // State
    bool m_initialized = false;
};

#endif // GAME_H
