#ifndef SCENE_H
#define SCENE_H

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "Camera.h"
#include "DoorManager.h"
#include "ExitSignManager.h"
#include "ExitRoomManager.h"
#include "PendulumManager.h"
#include "SpiderwebManager.h"
#include "TorchManager.h"
#include "ResourceManager.h"
#include "InputManager.h"
#include "Shaders.h"

class Scene {
public:
    Scene(ResourceManager& resources);
    ~Scene() = default;

    // Load map and initialize scene
    void loadMap(const std::string& filename);

    // Update scene state (movement, doors, etc.)
    void update(float deltaTime, InputManager& input);

    // Render the scene
    void render(int windowWidth, int windowHeight);

    // Camera access
    Camera& getCamera() { return m_camera; }
    const Camera& getCamera() const { return m_camera; }

    // State toggles
    bool isFlashlightOn() const { return m_flashlightOn; }
    bool isTopViewMode() const { return m_topViewMode; }
    void toggleFlashlight() { m_flashlightOn = !m_flashlightOn; }
    void toggleTopView() { m_topViewMode = !m_topViewMode; }

private:
    // Input processing
    void processMovement(float deltaTime, const InputState& input);
    void processActions(const InputState& input);

    // Collision detection
    bool checkCollision(glm::vec3 newPos);

    // Rendering helpers
    void setLights(glm::mat4 P, glm::mat4 V);
    void drawObjectMat(Model& model, Material& material, glm::mat4 P, glm::mat4 V, glm::mat4 M);
    void drawObjectTex(Model& model, Textures& textures, glm::mat4 P, glm::mat4 V, glm::mat4 M, float alpha = 0.0f, bool useAlpha = false);
    void drawTiledPlane(Model& model, Textures& tex, glm::mat4 P, glm::mat4 V, 
                        int gridX, int gridZ, float tileSize, float yPos, float scale, bool flip);
    void renderDoors(glm::mat4 P, glm::mat4 V);
    void renderExitSigns(glm::mat4 P, glm::mat4 V);
    void renderExitRooms(glm::mat4 P, glm::mat4 V);
    void renderPendulums(glm::mat4 P, glm::mat4 V);
    void renderSpiderwebs(glm::mat4 P, glm::mat4 V);
    void renderTorches(glm::mat4 P, glm::mat4 V);
    void renderFlashlight(glm::mat4 P, glm::mat4 V);
    
    // Flashlight helpers
    glm::mat4 calculateFlashlightTransform(glm::vec3& flashlightTipOut);
    void updateFlashlightLight(const glm::vec3& flashlightTip);

    // Reference to resources (not owned)
    ResourceManager& m_resources;

    // Camera/Player
    Camera m_camera;

    // Map data
    std::vector<std::string> m_mapLevel;

    // Door system
    DoorManager m_doorManager;

    // Exit sign system
    ExitSignManager m_exitSignManager;

    // Exit room system
    ExitRoomManager m_exitRoomManager;

    // Pendulum system
    PendulumManager m_pendulumManager;

    // Spiderweb system
    SpiderwebManager m_spiderwebManager;

    // Torch system
    TorchManager m_torchManager;

    // Lights
    Light m_lightG;           // Global ambient
    Light m_lightD[1];        // Directional lights
    Light m_lightP[12];       // Point lights (0: unused, 1: exit sign, 2-11: torches)
    Light m_lightF[2];        // Flashlights/Spotlights
    Material m_mluz;          // Light material
    Material m_mBlackWall;    // Black wall material for top view

    // State
    bool m_flashlightOn = true;
    bool m_topViewMode = false;
    float m_walkTime = 0.0f;
    float m_gameTime = 0.0f;  // Total elapsed time for torch flicker
    bool m_isWalking = false;

    // Constants
    static constexpr float BLOCK_SIZE = 4.0f;
    static constexpr float BLOCK_HEIGHT = 5.0f;
    static constexpr float PLAYER_HEIGHT = 3.2f;
    static constexpr float PLAYER_RADIUS = 0.4f;
    static constexpr float WALK_SPEED = 7.0f;
    static constexpr float DOOR_INTERACTION_DISTANCE = 6.0f;
};

#endif // SCENE_H
