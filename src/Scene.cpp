#include "Scene.h"
#include <fstream>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

// Using I macro from Model.h (identity matrix)

Scene::Scene(ResourceManager& resources)
    : m_resources(resources)
    , m_camera()
    , m_flashlightOn(true)
    , m_topViewMode(false)
    , m_walkTime(0.0f)
    , m_isWalking(false)
{
    // Initialize lights
    m_lightG.ambient = glm::vec3(0.1f, 0.1f, 0.12f);

    m_lightD[0].direction = glm::vec3(-1.0f, -1.0f, 0.0f);
    m_lightD[0].ambient   = glm::vec3(0.03f, 0.03f, 0.03f);
    m_lightD[0].diffuse   = glm::vec3(0.2f, 0.2f, 0.25f);
    m_lightD[0].specular  = glm::vec3(0.08f, 0.08f, 0.08f);

    m_lightP[0].position = glm::vec3(40.0f, 10.0f, 40.0f);
    m_lightP[0].ambient  = glm::vec3(0.01f, 0.01f, 0.01f);
    m_lightP[0].diffuse  = glm::vec3(0.15f, 0.15f, 0.2f);
    m_lightP[0].specular = glm::vec3(0.05f, 0.05f, 0.05f);
    m_lightP[0].c0 = 1.00f; m_lightP[0].c1 = 0.05f; m_lightP[0].c2 = 0.01f;

    // Flashlight
    m_lightF[0].position    = glm::vec3(0.0f, 0.0f, 0.0f);
    m_lightF[0].direction   = glm::vec3(0.0f, 0.0f, -1.0f);
    m_lightF[0].ambient     = glm::vec3(0.0f, 0.0f, 0.0f);
    m_lightF[0].diffuse     = glm::vec3(0.8f, 0.7f, 0.6f);
    m_lightF[0].specular    = glm::vec3(0.3f, 0.3f, 0.3f);
    m_lightF[0].innerCutOff = 10.0f;
    m_lightF[0].outerCutOff = 15.0f;
    m_lightF[0].c0 = 1.0f; m_lightF[0].c1 = 0.09f; m_lightF[0].c2 = 0.032f;
    m_lightF[1] = m_lightF[0];

    // Materials
    m_mluz.ambient = glm::vec4(0); 
    m_mluz.diffuse = glm::vec4(0);
    m_mluz.specular = glm::vec4(0); 
    m_mluz.emissive = glm::vec4(1); 
    m_mluz.shininess = 1.0f;

    m_mBlackWall.ambient = glm::vec4(0.05f, 0.05f, 0.05f, 1.0f);
    m_mBlackWall.diffuse = glm::vec4(0.05f, 0.05f, 0.05f, 1.0f);
    m_mBlackWall.specular = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    m_mBlackWall.emissive = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    m_mBlackWall.shininess = 1.0f;
}

void Scene::loadMap(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }

    m_mapLevel.clear();
    m_doorManager.clear();
    
    std::string line;
    int rowIndex = 0;

    while (std::getline(file, line)) {
        if (!line.empty()) {
            for (int colIndex = 0; colIndex < static_cast<int>(line.size()); colIndex++) {
                // Find player start position
                if (line[colIndex] == 'N') {
                    glm::vec3 startPos;
                    startPos.x = colIndex * BLOCK_SIZE;
                    startPos.z = rowIndex * BLOCK_SIZE;
                    startPos.y = PLAYER_HEIGHT;
                    m_camera.setPosition(startPos);
                    line[colIndex] = '0';
                    std::cout << "Initial position: grid(" << colIndex << ", " << rowIndex 
                              << ") -> world(" << startPos.x << ", " << startPos.z << ")" << std::endl;
                }
                // Detect doors - 'D' for normal, 'E' for fire exit
                if (line[colIndex] == 'D' || line[colIndex] == 'E') {
                    DoorType type = (line[colIndex] == 'D') ? DoorType::NORMAL : DoorType::FIRE_EXIT;
                    m_doorManager.addDoor(glm::ivec2(colIndex, rowIndex), type);
                }
            }
            m_mapLevel.push_back(line);
            rowIndex++;
        }
    }
    file.close();

    // Calculate door orientations based on adjacent walls
    m_doorManager.calculateOrientations(m_mapLevel);

    std::cout << "Map loaded: " << m_mapLevel.size() << " rows, " 
              << m_doorManager.getDoorCount() << " doors" << std::endl;
}

void Scene::update(float deltaTime, InputManager& input) {
    // Input is already polled in Game::run() before this call
    const InputState& state = input.getState();

    // Process one-shot actions
    processActions(state);

    // Process movement
    processMovement(deltaTime, state);

    // Update camera orientation from mouse
    if (state.mouseDeltaX != 0.0f || state.mouseDeltaY != 0.0f) {
        m_camera.updateOrientation(state.mouseDeltaX, state.mouseDeltaY);
    }

    // Update door animations
    m_doorManager.update(deltaTime);
}

void Scene::processActions(const InputState& input) {
    if (input.toggleFlashlight) {
        toggleFlashlight();
    }
    if (input.toggleTopView) {
        toggleTopView();
    }
    if (input.interact) {
        m_doorManager.toggleNearestDoor(m_camera.getPosition(), DOOR_INTERACTION_DISTANCE);
    }
}

void Scene::processMovement(float deltaTime, const InputState& input) {
    float cameraSpeed = WALK_SPEED * deltaTime;
    glm::vec3 currentPos = m_camera.getPosition();
    glm::vec3 nextPos = currentPos;

    bool moved = false;
    if (input.moveForward)  { nextPos += cameraSpeed * m_camera.getFrontXZ(); moved = true; }
    if (input.moveBackward) { nextPos -= cameraSpeed * m_camera.getFrontXZ(); moved = true; }
    if (input.moveLeft)     { nextPos -= cameraSpeed * m_camera.getRightXZ(); moved = true; }
    if (input.moveRight)    { nextPos += cameraSpeed * m_camera.getRightXZ(); moved = true; }

    if (moved) {
        m_isWalking = true;
        m_walkTime += deltaTime;

        // Check collision separately for X and Z axes (sliding along walls)
        bool collX = checkCollision(glm::vec3(nextPos.x, currentPos.y, currentPos.z));
        bool collZ = checkCollision(glm::vec3(currentPos.x, currentPos.y, nextPos.z));

        if (!collX) m_camera.setPositionX(nextPos.x);
        if (!collZ) m_camera.setPositionZ(nextPos.z);
    } else {
        m_isWalking = false;
    }

    // Lock Y position to player height
    m_camera.setPositionY(PLAYER_HEIGHT);
}

bool Scene::checkCollision(glm::vec3 newPos) {
    float halfBlock = BLOCK_SIZE / 2.0f;

    // Check wall collisions
    for (int z = 0; z < static_cast<int>(m_mapLevel.size()); z++) {
        for (int x = 0; x < static_cast<int>(m_mapLevel[z].size()); x++) {
            if (m_mapLevel[z][x] == '1') {
                glm::vec3 wallCenter(x * BLOCK_SIZE, 0.0f, z * BLOCK_SIZE);
                float distX = std::abs(newPos.x - wallCenter.x);
                float distZ = std::abs(newPos.z - wallCenter.z);
                if (distX < halfBlock + PLAYER_RADIUS && distZ < halfBlock + PLAYER_RADIUS) {
                    return true;
                }
            }
        }
    }

    // Check door collisions
    return m_doorManager.checkCollision(newPos, PLAYER_RADIUS);
}

void Scene::render(int windowWidth, int windowHeight) {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Shaders& shaders = m_resources.getShader();
    shaders.useShaders();

    float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
    glm::mat4 P = glm::perspective(glm::radians(60.0f), aspect, 0.1f, 150.0f);
    glm::mat4 V;

    if (m_topViewMode) {
        // Top-down view showing entire maze
        float tileSize = BLOCK_SIZE;
        int gridSizeX = 20;
        int gridSizeZ = 18;

        float mapCenterX = (gridSizeX * tileSize) / 2.0f;
        float mapCenterZ = (gridSizeZ * tileSize) / 2.0f;

        float mapWidth = gridSizeX * tileSize;
        float mapHeight = gridSizeZ * tileSize;
        float maxDimension = glm::max(mapWidth / aspect, mapHeight);
        float cameraHeight = maxDimension / (2.0f * tan(glm::radians(30.0f)));

        glm::vec3 topViewPos = glm::vec3(mapCenterX, cameraHeight, mapCenterZ);
        glm::vec3 topViewTarget = glm::vec3(mapCenterX, 0.0f, mapCenterZ);
        V = glm::lookAt(topViewPos, topViewTarget, glm::vec3(0.0f, 0.0f, -1.0f));
    } else {
        V = m_camera.getViewMatrix();
    }

    shaders.setVec3("ueye", m_camera.getPosition());
    setLights(P, V);

    // Get map dimensions
    int gridSizeZ = static_cast<int>(m_mapLevel.size());
    int gridSizeX = (gridSizeZ > 0) ? static_cast<int>(m_mapLevel[0].size()) : 0;
    float scaleFloor = BLOCK_SIZE / 2.0f;

    // Draw floor
    Model& planeModel = m_resources.getModel("plane");
    Textures& texFloor = m_resources.getTextureGroup("floor");
    drawTiledPlane(planeModel, texFloor, P, V, gridSizeX, gridSizeZ, BLOCK_SIZE, 0.0f, scaleFloor, false);

    // Draw ceiling (not in top view)
    if (!m_topViewMode) {
        Textures& texCeiling = m_resources.getTextureGroup("ceiling");
        drawTiledPlane(planeModel, texCeiling, P, V, gridSizeX, gridSizeZ, BLOCK_SIZE, BLOCK_HEIGHT, scaleFloor, true);
    }

    // Draw walls
    Model& cubeModel = m_resources.getModel("cube");
    Textures& texWall = m_resources.getTextureGroup("wall");
    float scX = BLOCK_SIZE / 2.0f;
    float scY = BLOCK_HEIGHT / 2.0f;
    float scZ = BLOCK_SIZE / 2.0f;

    for (int z = 0; z < static_cast<int>(m_mapLevel.size()); z++) {
        for (int x = 0; x < static_cast<int>(m_mapLevel[z].size()); x++) {
            if (m_mapLevel[z][x] == '1') {
                glm::vec3 pos(x * BLOCK_SIZE, BLOCK_HEIGHT / 2.0f, z * BLOCK_SIZE);
                glm::mat4 M = glm::translate(I, pos);
                M = glm::scale(M, glm::vec3(scX, scY, scZ));
                if (m_topViewMode) {
                    drawObjectMat(cubeModel, m_mBlackWall, P, V, M);
                } else {
                    drawObjectTex(cubeModel, texWall, P, V, M);
                }
            }
        }
    }

    // Render doors
    renderDoors(P, V);

    // Render flashlight (first person only)
    renderFlashlight(P, V);
}

void Scene::setLights(glm::mat4 P, glm::mat4 V) {
    Shaders& shaders = m_resources.getShader();

    Light modifiedLightG = m_lightG;
    if (m_topViewMode) {
        modifiedLightG.ambient = glm::vec3(0.6f, 0.6f, 0.65f);
    }
    shaders.setLight("ulightG", modifiedLightG);

    for (int i = 0; i < 1; i++) {
        Light light = m_lightD[i];
        if (m_topViewMode) {
            light.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
            light.diffuse = glm::vec3(0.8f, 0.8f, 0.9f);
            light.specular = glm::vec3(0.3f, 0.3f, 0.3f);
        }
        shaders.setLight("ulightD[" + std::to_string(i) + "]", light);
    }

    for (int i = 0; i < 1; i++) {
        Light light = m_lightP[i];
        light.position = glm::vec3(V * glm::vec4(light.position, 1.0f));
        shaders.setLight("ulightP[" + std::to_string(i) + "]", light);
    }

    for (int i = 0; i < 2; i++) {
        Light light = m_lightF[i];
        light.position = glm::vec3(V * glm::vec4(light.position, 1.0f));
        light.direction = glm::vec3(V * glm::vec4(light.direction, 0.0f));
        if (!m_flashlightOn) {
            light.diffuse = glm::vec3(0.0f, 0.0f, 0.0f);
            light.specular = glm::vec3(0.0f, 0.0f, 0.0f);
        }
        shaders.setLight("ulightF[" + std::to_string(i) + "]", light);
    }
}

void Scene::drawObjectMat(Model& model, Material& material, glm::mat4 P, glm::mat4 V, glm::mat4 M) {
    Shaders& shaders = m_resources.getShader();
    shaders.setMat4("uN", glm::transpose(glm::inverse(V * M)));
    shaders.setMat4("uM", V * M);
    shaders.setMat4("uPVM", P * V * M);
    shaders.setBool("uWithMaterials", 1);
    shaders.setMaterial("umaterial", material);
    model.renderModel(GL_FILL);
}

void Scene::drawObjectTex(Model& model, Textures& textures, glm::mat4 P, glm::mat4 V, glm::mat4 M, float alpha) {
    Shaders& shaders = m_resources.getShader();
    shaders.setMat4("uN", glm::transpose(glm::inverse(V * M)));
    shaders.setMat4("uM", V * M);
    shaders.setMat4("uPVM", P * V * M);
    shaders.setBool("uWithMaterials", 0);
    shaders.setBool("uWithNormals", textures.normal != 0);
    shaders.setFloat("uAlpha", alpha);
    shaders.setTextures("utextures", textures);
    if (textures.diffuse != 0) model.renderModel(GL_FILL);
}

void Scene::drawTiledPlane(Model& model, Textures& tex, glm::mat4 P, glm::mat4 V,
                           int gridX, int gridZ, float tileSize, float yPos, float scale, bool flip) {
    for (int z = 0; z < gridZ; z++) {
        for (int x = 0; x < gridX; x++) {
            glm::vec3 pos(x * tileSize, yPos, z * tileSize);
            glm::mat4 M = glm::translate(I, pos);
            if (flip) M = glm::rotate(M, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            M = glm::scale(M, glm::vec3(scale, 1.0f, scale));
            drawObjectTex(model, tex, P, V, M);
        }
    }
}

// Helper for door rendering
glm::mat4 createDoorPartTransform(glm::vec3 doorPos, float rotation, float scaleX, float scaleY, float scaleZ,
                                   glm::vec3 partCenter, float partHeight, float doorOpenAngle = 0.0f, bool isMovingPart = false) {
    glm::mat4 M = glm::translate(I, doorPos);
    M = glm::rotate(M, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));

    if (isMovingPart && doorOpenAngle != 0.0f) {
        float doorWidth = partCenter.x * 2.0f;
        M = glm::translate(M, glm::vec3(-doorWidth * scaleX * 0.5f, 0.0f, 0.0f));
        M = glm::rotate(M, glm::radians(doorOpenAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        M = glm::translate(M, glm::vec3(doorWidth * scaleX * 0.5f, 0.0f, 0.0f));
    }

    M = glm::scale(M, glm::vec3(scaleX, scaleY, scaleZ));
    M = glm::translate(M, -partCenter);
    M = glm::translate(M, glm::vec3(0.0f, partHeight / 2.0f, 0.0f));
    return M;
}

void Scene::renderDoors(glm::mat4 P, glm::mat4 V) {
    glDisable(GL_CULL_FACE);

    Textures& texDoorFrame = m_resources.getTextureGroup("doorFrame");
    Textures& texDoorGlass = m_resources.getTextureGroup("doorGlass");

    for (const auto& door : m_doorManager.getDoors()) {
        glm::vec3 doorPos(door.gridPos.x * BLOCK_SIZE, 0.0f, door.gridPos.y * BLOCK_SIZE);

        if (door.type == DoorType::NORMAL) {
            Model& doorFrame = m_resources.getModel("doorNormalFrame");
            Model& doorDoor = m_resources.getModel("doorNormalDoor");
            Model& doorGlass = m_resources.getModel("doorNormalGlass");

            glm::vec3 fSize = doorFrame.getSize();
            glm::vec3 scale((BLOCK_SIZE * 0.95f) / fSize.x, BLOCK_HEIGHT / fSize.y, (BLOCK_SIZE * 0.95f) / fSize.x);
            float rot = door.rotation + 180.0f;

            // Frame (static)
            glm::mat4 M = createDoorPartTransform(doorPos, rot, scale.x, scale.y, scale.z, doorFrame.getCenter(), doorFrame.getSize().y, 0.0f, false);
            drawObjectTex(doorFrame, texDoorFrame, P, V, M);

            // Door (moving)
            M = createDoorPartTransform(doorPos, rot, scale.x, scale.y, scale.z, doorDoor.getCenter(), doorDoor.getSize().y, door.openAngle, true);
            drawObjectTex(doorDoor, texDoorFrame, P, V, M);

            // Glass (moving, transparent)
            M = createDoorPartTransform(doorPos, rot, scale.x, scale.y, scale.z, doorGlass.getCenter(), doorGlass.getSize().y, door.openAngle, true);
            glDepthMask(GL_FALSE);
            drawObjectTex(doorGlass, texDoorGlass, P, V, M, 0.35f);
            glDepthMask(GL_TRUE);

        } else { // FIRE_EXIT
            Model& doorFrame = m_resources.getModel("doorExitFrame");
            Model& doorDoor = m_resources.getModel("doorExitDoor");
            Model& doorExtra1 = m_resources.getModel("doorExitExtra1");
            Model& doorExtra2 = m_resources.getModel("doorExitExtra2");

            glm::vec3 fSize = doorFrame.getSize();
            glm::vec3 scale((BLOCK_SIZE * 0.95f) / fSize.x, BLOCK_HEIGHT / fSize.y, (BLOCK_SIZE * 0.95f) / fSize.x);

            // Frame (static)
            glm::mat4 M = createDoorPartTransform(doorPos, door.rotation, scale.x, scale.y, scale.z, doorFrame.getCenter(), doorFrame.getSize().y, 0.0f, false);
            drawObjectTex(doorFrame, texDoorFrame, P, V, M);

            // Door (moving)
            M = createDoorPartTransform(doorPos, door.rotation, scale.x, scale.y, scale.z, doorDoor.getCenter(), doorDoor.getSize().y, door.openAngle, true);
            drawObjectTex(doorDoor, texDoorFrame, P, V, M);

            // Extra parts (moving)
            M = createDoorPartTransform(doorPos, door.rotation, scale.x, scale.y, scale.z, doorExtra1.getCenter(), doorExtra1.getSize().y, door.openAngle, true);
            drawObjectTex(doorExtra1, texDoorFrame, P, V, M);

            M = createDoorPartTransform(doorPos, door.rotation, scale.x, scale.y, scale.z, doorExtra2.getCenter(), doorExtra2.getSize().y, door.openAngle, true);
            drawObjectTex(doorExtra2, texDoorFrame, P, V, M);
        }
    }

    glEnable(GL_CULL_FACE);
}

glm::mat4 Scene::calculateFlashlightTransform(glm::vec3& flashlightTipOut) {
    glm::vec3 cameraFront = m_camera.getFront();
    glm::vec3 cameraUp = m_camera.getUp();
    glm::vec3 cameraPos = m_camera.getPosition();

    glm::vec3 rightVector = glm::normalize(glm::cross(cameraFront, cameraUp));
    glm::vec3 upVector = glm::normalize(glm::cross(rightVector, cameraFront));

    // Walking sway effect
    float swayOffsetX = 0.0f;
    float swayOffsetY = 0.0f;
    if (m_isWalking) {
        float swaySpeed = 8.0f;
        float swayAmount = 0.015f;
        swayOffsetX = sin(m_walkTime * swaySpeed) * swayAmount;
        swayOffsetY = abs(cos(m_walkTime * swaySpeed * 0.5f)) * swayAmount * 0.5f;
    }

    // Position in lower right
    glm::vec3 flashlightPos = cameraPos
        + cameraFront * 0.25f
        + rightVector * (0.1f + swayOffsetX)
        - upVector * (0.10f - swayOffsetY);

    // Orientation matrix
    glm::mat4 MFlashlight(1.0f);
    MFlashlight[0] = glm::vec4(cameraFront, 0.0f);
    MFlashlight[1] = glm::vec4(upVector, 0.0f);
    MFlashlight[2] = glm::vec4(rightVector, 0.0f);
    MFlashlight[3] = glm::vec4(flashlightPos, 1.0f);

    MFlashlight = glm::scale(MFlashlight, glm::vec3(0.15f, 0.15f, 0.15f));

    // Calculate tip position for light
    float flashlightLength = 0.15f;
    flashlightTipOut = flashlightPos + cameraFront * flashlightLength;

    return MFlashlight;
}

void Scene::updateFlashlightLight(const glm::vec3& flashlightTip) {
    m_lightF[0].position = flashlightTip;
    m_lightF[0].direction = m_camera.getFront();
}

void Scene::renderFlashlight(glm::mat4 P, glm::mat4 V) {
    glm::vec3 flashlightTip;
    glm::mat4 MFlashlight = calculateFlashlightTransform(flashlightTip);

    Model& flashlightModel = m_resources.getModel("flashlight");
    Textures& texFlashlight = m_resources.getTextureGroup("flashlight");
    drawObjectTex(flashlightModel, texFlashlight, P, V, MFlashlight);

    updateFlashlightLight(flashlightTip);
}
