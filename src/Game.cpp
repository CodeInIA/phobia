#include "Game.h"
#include <iostream>

Game::Game()
    : m_window(nullptr)
    , m_resourceManager()
    , m_inputManager()
    , m_scene(nullptr)
    , m_lastFrame(0.0f)
    , m_initialized(false)
{
}

Game::~Game() {
    shutdown();
}

bool Game::init(int width, int height, const std::string& title) {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // Window hints for OpenGL 3.3 Core
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create window
    m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!m_window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // VSync

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (GLEW_OK != glewInit()) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwDestroyWindow(m_window);
        glfwTerminate();
        return false;
    }

    // Initialize input manager with this game instance
    m_inputManager.init(m_window, this);

    // OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Load all resources
    std::cout << "Loading resources..." << std::endl;
    m_resourceManager.loadAllResources();

    // Create and initialize scene
    m_scene = std::make_unique<Scene>(m_resourceManager);
    m_scene->loadMap("resources/models/map.cub");

    m_initialized = true;
    std::cout << "Game initialized successfully" << std::endl;
    return true;
}

void Game::run() {
    if (!m_initialized) {
        std::cerr << "Game not initialized!" << std::endl;
        return;
    }

    std::cout << "Starting game loop..." << std::endl;

    while (!glfwWindowShouldClose(m_window)) {
        // Calculate delta time
        float currentFrame = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrame - m_lastFrame;
        m_lastFrame = currentFrame;

        // Poll events first
        glfwPollEvents();

        // Poll continuous input (WASD)
        m_inputManager.pollInput(m_window);

        // Check for quit
        if (m_inputManager.getState().quit) {
            glfwSetWindowShouldClose(m_window, true);
        }

        // Update scene
        m_scene->update(deltaTime, m_inputManager);

        // Reset one-shot actions after processing
        m_inputManager.resetActions();

        // Render scene
        m_scene->render(m_inputManager.getWindowWidth(), m_inputManager.getWindowHeight());

        // Swap buffers
        glfwSwapBuffers(m_window);
    }
}

void Game::shutdown() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
    m_initialized = false;
}
