#include "Scene.h"
#include "ExitSignManager.h"
#include "SpiderwebManager.h"
#include "TorchManager.h"
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
    // Initialize lights - PITCH BLACK for horror atmosphere
    // Only torches and flashlight provide light
    m_lightG.ambient = glm::vec3(0.0f, 0.0f, 0.0f);  // No ambient light

    // Disable directional light
    m_lightD[0].direction = glm::vec3(-1.0f, -1.0f, 0.0f);
    m_lightD[0].ambient   = glm::vec3(0.0f, 0.0f, 0.0f);
    m_lightD[0].diffuse   = glm::vec3(0.0f, 0.0f, 0.0f);
    m_lightD[0].specular  = glm::vec3(0.0f, 0.0f, 0.0f);

    // Disable first point light (unused)
    m_lightP[0].position = glm::vec3(0.0f, 0.0f, 0.0f);
    m_lightP[0].ambient  = glm::vec3(0.0f, 0.0f, 0.0f);
    m_lightP[0].diffuse  = glm::vec3(0.0f, 0.0f, 0.0f);
    m_lightP[0].specular = glm::vec3(0.0f, 0.0f, 0.0f);
    m_lightP[0].c0 = 1.0f; m_lightP[0].c1 = 0.0f; m_lightP[0].c2 = 0.0f;
    
    // Initialize all torch point lights (slots 2-11) as disabled
    for (int i = 2; i < 12; i++) {
        m_lightP[i].position = glm::vec3(0.0f, 0.0f, 0.0f);
        m_lightP[i].ambient  = glm::vec3(0.0f, 0.0f, 0.0f);
        m_lightP[i].diffuse  = glm::vec3(0.0f, 0.0f, 0.0f);
        m_lightP[i].specular = glm::vec3(0.0f, 0.0f, 0.0f);
        m_lightP[i].c0 = 1.0f; m_lightP[i].c1 = 0.14f; m_lightP[i].c2 = 0.07f;
    }

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
    m_exitSignManager.clear();
    m_pendulumManager.clear();
    m_spiderwebManager.clear();
    m_torchManager.clear();
    
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
                    // Also add exit sign for fire exit doors
                    if (line[colIndex] == 'E') {
                        m_exitSignManager.addExitSign(glm::ivec2(colIndex, rowIndex));
                    }
                }
                // Detect pendulums - 'P'
                if (line[colIndex] == 'P') {
                    m_pendulumManager.addPendulum(glm::ivec2(colIndex, rowIndex));
                    line[colIndex] = '0';  // Replace with empty space for rendering
                }
                // Detect torches - 'T'
                if (line[colIndex] == 'T') {
                    m_torchManager.addTorch(glm::ivec2(colIndex, rowIndex));
                    line[colIndex] = '0';  // Replace with empty space for rendering
                }
            }
            m_mapLevel.push_back(line);
            rowIndex++;
        }
    }
    file.close();

    // Calculate door orientations based on adjacent walls
    m_doorManager.calculateOrientations(m_mapLevel);

    // Calculate pendulum orientations based on adjacent walls
    m_pendulumManager.calculateOrientations(m_mapLevel);

    // Detect and place spiderwebs at concave corners
    m_spiderwebManager.detectAndAddSpiderwebs(m_mapLevel);

    // Calculate torch orientations based on adjacent walls
    m_torchManager.calculateOrientations(m_mapLevel);

    std::cout << "Map loaded: " << m_mapLevel.size() << " rows, " 
              << m_doorManager.getDoorCount() << " doors, "
              << m_exitSignManager.getExitSigns().size() << " exit signs, "
              << m_pendulumManager.getPendulumCount() << " pendulums, "
              << m_spiderwebManager.getSpiderwebCount() << " spiderwebs, "
              << m_torchManager.getTorchCount() << " torches" << std::endl;
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

    // Update pendulum physics
    m_pendulumManager.update(deltaTime);

    // Update game time for torch flicker
    m_gameTime += deltaTime;
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
    if (m_doorManager.checkCollision(newPos, PLAYER_RADIUS)) {
        return true;
    }

    // Check pendulum collisions
    return m_pendulumManager.checkCollision(newPos, PLAYER_RADIUS);
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

    shaders.setVec3("ueye", glm::vec3(0.0f, 0.0f, 0.0f));  // In view space, camera is at origin
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

    // Render exit signs
    renderExitSigns(P, V);

    // Render pendulums
    renderPendulums(P, V);

    // Render spiderwebs
    renderSpiderwebs(P, V);

    // Render torches
    renderTorches(P, V);

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

    // Point lights
    Light light0 = m_lightP[0];
    light0.position = glm::vec3(V * glm::vec4(light0.position, 1.0f));
    shaders.setLight("ulightP[0]", light0);

    // Exit sign green light
    if (!m_exitSignManager.getExitSigns().empty()) {
        const auto& sign = m_exitSignManager.getExitSigns()[0];
        glm::vec3 signPos;
        signPos.x = sign.gridPos.x * BLOCK_SIZE + BLOCK_SIZE * -0.5f;
        signPos.y = BLOCK_HEIGHT - 0.3f;  // Slightly below ceiling
        signPos.z = sign.gridPos.y * BLOCK_SIZE + BLOCK_SIZE * 0.5f;
        
        Light exitLight;
        exitLight.position = glm::vec3(V * glm::vec4(signPos, 1.0f));
        exitLight.ambient  = glm::vec3(0.02f, 0.05f, 0.02f);
        exitLight.diffuse  = glm::vec3(0.2f, 0.8f, 0.2f);  // Green light with reduced intensity
        exitLight.specular = glm::vec3(0.1f, 0.2f, 0.1f);
        exitLight.c0 = 1.0f;
        exitLight.c1 = 0.35f;   // Increased linear attenuation
        exitLight.c2 = 0.44f;   // Increased quadratic attenuation for faster falloff
        shaders.setLight("ulightP[1]", exitLight);
    } else {
        // No exit sign, disable light slot 1
        Light light1;
        light1.position = glm::vec3(0.0f);
        light1.ambient  = glm::vec3(0.0f);
        light1.diffuse  = glm::vec3(0.0f);
        light1.specular = glm::vec3(0.0f);
        light1.c0 = 1.0f; light1.c1 = 0.0f; light1.c2 = 0.0f;
        shaders.setLight("ulightP[1]", light1);
    }

    // Torch point lights (slots 2-11) - dim warm fire color for horror atmosphere
    const auto& torches = m_torchManager.getTorches();
    for (int i = 0; i < 10; i++) {
        Light torchLight;
        if (i < static_cast<int>(torches.size())) {
            const Torch& torch = torches[i];
            
            // Calculate torch light position (at flame, matching renderTorches)
            glm::vec3 torchPos;
            torchPos.x = torch.gridPos.x * BLOCK_SIZE;
            torchPos.y = 3.5f;  // Flame height
            torchPos.z = torch.gridPos.y * BLOCK_SIZE;
            
            // Offset towards wall (matching renderTorches)
            float wallOffset = BLOCK_SIZE * 0.45f;
            if (torch.rotation == 0.0f) {
                torchPos.z += wallOffset;
            } else if (torch.rotation == 180.0f) {
                torchPos.z -= wallOffset;
            } else if (torch.rotation == 90.0f) {
                torchPos.x -= wallOffset;
            } else if (torch.rotation == 270.0f) {
                torchPos.x += wallOffset;
            }
            
            // Get flicker intensity for organic fire effect
            float flicker = m_torchManager.getFlickerIntensity(i, m_gameTime);
            
            // Brighter warm fire color - more illumination
            glm::vec3 fireColor = glm::vec3(0.7f, 0.3f, 0.1f) * flicker;
            
            torchLight.position = glm::vec3(V * glm::vec4(torchPos, 1.0f));
            torchLight.ambient  = glm::vec3(0.02f, 0.008f, 0.002f) * flicker;
            torchLight.diffuse  = fireColor;
            torchLight.specular = glm::vec3(0.15f, 0.08f, 0.03f) * flicker;
            torchLight.c0 = 1.0f;
            torchLight.c1 = 0.14f;  // Lower attenuation = larger range
            torchLight.c2 = 0.10f;  // Much slower falloff (~6-8 units)
        } else {
            // No torch for this slot, disable it
            torchLight.position = glm::vec3(0.0f);
            torchLight.ambient  = glm::vec3(0.0f);
            torchLight.diffuse  = glm::vec3(0.0f);
            torchLight.specular = glm::vec3(0.0f);
            torchLight.c0 = 1.0f; torchLight.c1 = 0.0f; torchLight.c2 = 0.0f;
        }
        shaders.setLight("ulightP[" + std::to_string(i + 2) + "]", torchLight);
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

void Scene::drawObjectTex(Model& model, Textures& textures, glm::mat4 P, glm::mat4 V, glm::mat4 M, float alpha, bool useAlpha) {
    Shaders& shaders = m_resources.getShader();
    shaders.setMat4("uN", glm::transpose(glm::inverse(V * M)));
    shaders.setMat4("uM", V * M);
    shaders.setMat4("uPVM", P * V * M);
    shaders.setBool("uWithMaterials", 0);
    shaders.setBool("uWithNormals", textures.normal != 0);
    shaders.setFloat("uAlpha", alpha);
    shaders.setTextures("utextures", textures);

    if (textures.diffuse != 0) {
        if (useAlpha) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_FALSE);
            glDisable(GL_CULL_FACE);
        }

        model.renderModel(GL_FILL);

        if (useAlpha) {
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
            glEnable(GL_CULL_FACE);
        }
    }
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

void Scene::renderExitSigns(glm::mat4 P, glm::mat4 V) {
    if (m_exitSignManager.getExitSigns().empty()) return;

    Model& exitSignModel = m_resources.getModel("exitSign");
    Textures& texExitSign = m_resources.getTextureGroup("exitSign");

    for (const auto& sign : m_exitSignManager.getExitSigns()) {
        // Calculate position - at the right edge of the tile
        glm::vec3 signPos;
        signPos.x = sign.gridPos.x * BLOCK_SIZE + BLOCK_SIZE * -0.5f;  // Right edge
        signPos.y = BLOCK_HEIGHT;  // At ceiling level
        signPos.z = sign.gridPos.y * BLOCK_SIZE + BLOCK_SIZE * 0.5f;   // Center of tile

        // Get model size to scale appropriately
        glm::vec3 modelSize = exitSignModel.getSize();
        
        // Scale the model to fit nicely at ceiling (make it smaller than a full block)
        float desiredWidth = BLOCK_SIZE * 0.6f;  // 60% of block size
        float scale = desiredWidth / modelSize.x;
        
        // Create transformation matrix
        glm::mat4 M = glm::translate(I, signPos);
        // Rotate 90 degrees around Y axis
        M = glm::rotate(M, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        // The sign should hang from the ceiling, so we offset it down slightly
        M = glm::translate(M, glm::vec3(0.0f, -modelSize.y * scale * 0.5f, 0.0f));
        // Apply uniform scale
        M = glm::scale(M, glm::vec3(scale, scale, scale));

        // Draw the exit sign
        drawObjectTex(exitSignModel, texExitSign, P, V, M);
    }
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

void Scene::renderPendulums(glm::mat4 P, glm::mat4 V) {
    if (m_pendulumManager.getPendulumCount() == 0) return;

    Model& pendulumModel = m_resources.getModel("pendulumBlade");
    
    // Get texture groups for the three materials
    Textures& texWood = m_resources.getTextureGroup("pendulumWood");
    Textures& texBlade = m_resources.getTextureGroup("pendulumBlade");
    Textures& texDirection = m_resources.getTextureGroup("pendulumDirection");

    for (const auto& pendulum : m_pendulumManager.getPendulums()) {
        // Calculate pendulum world position (centered on ceiling tile)
        glm::vec3 pivotPos;
        pivotPos.x = pendulum.gridPos.x * BLOCK_SIZE;
        pivotPos.y = BLOCK_HEIGHT;  // At ceiling
        pivotPos.z = pendulum.gridPos.y * BLOCK_SIZE;

        // The model is centered around Y=3.38 approximately (top of holder)
        // We need to position it so the holder is at the ceiling
        float modelTopY = 3.384f;  // Top of the holder in model space
        
        // Base transformation for the static holder (Mesh 0)
        // First rotate the entire model 90 degrees around Y axis
        glm::mat4 MHolder = glm::translate(I, pivotPos);
        MHolder = glm::rotate(MHolder, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        MHolder = glm::translate(MHolder, glm::vec3(0.0f, -modelTopY, 0.0f));

        // Render Mesh 0: PendulumHolder_0 (WoodMat) - STATIC, no swing
        Shaders& shaders = m_resources.getShader();
        shaders.setMat4("uN", glm::transpose(glm::inverse(V * MHolder)));
        shaders.setMat4("uM", V * MHolder);
        shaders.setMat4("uPVM", P * V * MHolder);
        shaders.setBool("uWithMaterials", 0);
        shaders.setBool("uWithNormals", 0);
        shaders.setFloat("uAlpha", 0.0f);
        shaders.setTextures("utextures", texWood);
        pendulumModel.renderMesh(0, GL_FILL);

        // Transformation for the swinging parts (Mesh 1 and 2)
        // Apply the same 90 degree rotation, then the swing
        glm::mat4 MSwing = glm::translate(I, pivotPos);
        MSwing = glm::rotate(MSwing, glm::radians(90.0f + pendulum.rotation), glm::vec3(0.0f, 1.0f, 0.0f));
        MSwing = glm::translate(MSwing, glm::vec3(0.0f, -modelTopY, 0.0f));
        
        // Apply swing rotation around the pivot (Z-axis after 90º Y rotation)
        // Translate to pivot point with X offset to align with holder
        MSwing = glm::translate(MSwing, glm::vec3(-0.15f, modelTopY, 0.0f));
        // Scale the pole to make it longer
        MSwing = glm::scale(MSwing, glm::vec3(1.0f, 1.15f, 1.0f));
        // Compensate for the model's initial tilt to make it vertical
        MSwing = glm::rotate(MSwing, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        // Apply the oscillating swing angle
        MSwing = glm::rotate(MSwing, glm::radians(pendulum.swingAngle), glm::vec3(0.0f, 0.0f, 1.0f));
        // Return to origin
        MSwing = glm::translate(MSwing, glm::vec3(0.0f, -modelTopY, 0.0f));

        // Render Mesh 1: PendulumBlade_0 (PendulumBladeMat) - SWINGING
        shaders.setMat4("uN", glm::transpose(glm::inverse(V * MSwing)));
        shaders.setMat4("uM", V * MSwing);
        shaders.setMat4("uPVM", P * V * MSwing);
        shaders.setTextures("utextures", texBlade);
        pendulumModel.renderMesh(1, GL_FILL);

        // Render Mesh 2: PendulumBlade_1 (DirectionMat) - SWINGING
        shaders.setTextures("utextures", texDirection);
        pendulumModel.renderMesh(2, GL_FILL);
    }
}

void Scene::renderSpiderwebs(glm::mat4 P, glm::mat4 V) {
    if (m_spiderwebManager.getSpiderwebCount() == 0) return;

    Model& spiderwebModel = m_resources.getModel("spiderweb");
    
    // Create materials that respond to lighting (no self-illumination)
    Material webMaterial;
    webMaterial.ambient = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
    webMaterial.diffuse = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
    webMaterial.specular = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);
    webMaterial.emissive = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);  // No self-illumination
    webMaterial.shininess = 20.0f;

    Material spiderMaterial;
    spiderMaterial.ambient = glm::vec4(0.2f, 0.15f, 0.15f, 1.0f);
    spiderMaterial.diffuse = glm::vec4(0.3f, 0.25f, 0.25f, 1.0f);
    spiderMaterial.specular = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
    spiderMaterial.emissive = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);  // No self-illumination
    spiderMaterial.shininess = 15.0f;

    Material preyMaterial;
    preyMaterial.ambient = glm::vec4(0.4f, 0.35f, 0.3f, 1.0f);
    preyMaterial.diffuse = glm::vec4(0.6f, 0.55f, 0.5f, 1.0f);
    preyMaterial.specular = glm::vec4(0.25f, 0.25f, 0.25f, 1.0f);
    preyMaterial.emissive = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);  // No self-illumination
    preyMaterial.shininess = 18.0f;

    Shaders& shaders = m_resources.getShader();
    
    // Disable face culling to ensure all faces are visible
    glDisable(GL_CULL_FACE);

    for (const auto& web : m_spiderwebManager.getSpiderwebs()) {
        // Calculate spiderweb world position - offset to corner based on rotation
        glm::vec3 webPos;
        float halfBlock = BLOCK_SIZE / 2.0f;
        
        // Base position at grid cell center
        webPos.x = web.gridPos.x * BLOCK_SIZE;
        webPos.y = 0.0f;  // Position at lower corner, very near the floor
        webPos.z = web.gridPos.y * BLOCK_SIZE;
        
        // Adjust position to corner based on rotation
        // NE and SW need larger offset to reach the corner properly
        float cornerOffset = halfBlock * 0.45f;
        
        if (web.rotation == 0.0f) {          // SE corner (facing NW)
            webPos.x += cornerOffset;
            webPos.z += cornerOffset;
        } else if (web.rotation == 90.0f) {  // NE corner (was SW, now adjusted)
            webPos.x += cornerOffset;  // Increased offset to reach corner
            webPos.z -= cornerOffset;
        } else if (web.rotation == 180.0f) { // NW corner (reference - looks perfect)
            webPos.x -= cornerOffset;
            webPos.z -= cornerOffset;
        } else if (web.rotation == 270.0f) { // SW corner (was NE, now adjusted)
            webPos.x -= cornerOffset;  // Increased offset to reach corner
            webPos.z += cornerOffset;
        }

        // Base transformation - position and rotation
        // NW corner (180°) at position (1,1) is the reference that looks good
        // Adjust other corners to match that orientation relative to their walls
        glm::mat4 M = glm::translate(I, webPos);
        
        float finalRotation = web.rotation + -30.0f;  // Base adjustment for model orientation
        
        M = glm::rotate(M, glm::radians(finalRotation), glm::vec3(0.0f, 1.0f, 0.0f));
        
        // Scale to fit nicely in the corner - adjusted size
        float scale = 0.18f;
        M = glm::scale(M, glm::vec3(scale, scale, scale));

        // The spiderweb model has multiple meshes (node_3 to node_10):
        // - Mesh 0 (node_3): Web_opaque (material index 1)
        // - Mesh 1,2 (node_4,5): Spider_opaque (material index 2)
        // - Mesh 3-7 (node_6-10): Prey_opaque (material index 3)

        // Render all meshes - the model should have 8 submeshes
        unsigned int meshCount = spiderwebModel.getMeshCount();
        
        // Render each mesh individually with its corresponding material
        for (unsigned int meshIndex = 0; meshIndex < meshCount; meshIndex++) {
            unsigned int matIndex = spiderwebModel.getMeshMaterialIndex(meshIndex);
            
            // Select appropriate material based on material index
            // Material index 1 = Web_opaque, 2 = Spider_opaque, 3 = Prey_opaque
            Material* currentMaterial = &webMaterial;
            if (matIndex == 2) {
                currentMaterial = &spiderMaterial;
            } else if (matIndex == 3) {
                currentMaterial = &preyMaterial;
            }

            shaders.setMat4("uN", glm::transpose(glm::inverse(V * M)));
            shaders.setMat4("uM", V * M);
            shaders.setMat4("uPVM", P * V * M);
            shaders.setBool("uWithMaterials", 1);
            shaders.setMaterial("umaterial", *currentMaterial);
            spiderwebModel.renderMesh(meshIndex, GL_FILL);
        }
    }
    
    // Re-enable face culling
    glEnable(GL_CULL_FACE);
}

void Scene::renderTorches(glm::mat4 P, glm::mat4 V) {
    if (m_torchManager.getTorchCount() == 0) return;

    Model& torchModel = m_resources.getModel("torch");
    Model& planeModel = m_resources.getModel("plane");
    Textures& texTorch = m_resources.getTextureGroup("torch");
    Textures& texFlame = m_resources.getTextureGroup("flame");

    for (size_t i = 0; i < m_torchManager.getTorches().size(); i++) {
        const auto& torch = m_torchManager.getTorches()[i];
        
        // Calculate torch world position - offset towards wall based on rotation
        glm::vec3 torchPos;
        torchPos.x = torch.gridPos.x * BLOCK_SIZE;
        torchPos.y = 3.2f;  // Higher mount position on wall
        torchPos.z = torch.gridPos.y * BLOCK_SIZE;
        
        // Offset torch towards wall based on its facing direction
        float wallOffset = BLOCK_SIZE * 0.45f;  // Close to wall
        if (torch.rotation == 0.0f) {           // Facing north, mounted on south wall
            torchPos.z += wallOffset;
        } else if (torch.rotation == 180.0f) {  // Facing south, mounted on north wall
            torchPos.z -= wallOffset;
        } else if (torch.rotation == 90.0f) {   // Facing east, mounted on west wall
            torchPos.x -= wallOffset;
        } else if (torch.rotation == 270.0f) {  // Facing west, mounted on east wall
            torchPos.x += wallOffset;
        }

        // Get model size to scale appropriately
        glm::vec3 modelSize = torchModel.getSize();
        
        // Scale the model to reasonable size
        float desiredHeight = 1.0f;  // Slightly smaller holder
        float scale = desiredHeight / modelSize.y;
        
        // Create transformation matrix for holder
        glm::mat4 M = glm::translate(I, torchPos);
        // Rotate to face away from wall
        M = glm::rotate(M, glm::radians(torch.rotation), glm::vec3(0.0f, 1.0f, 0.0f));
        // Apply uniform scale
        M = glm::scale(M, glm::vec3(scale, scale, scale));
        // Center the model
        M = glm::translate(M, -torchModel.getCenter());

        // Draw the torch holder
        drawObjectTex(torchModel, texTorch, P, V, M);

        // Calculate flame position (at the torch cup/bowl area)
        glm::vec3 flamePos = torchPos;
        flamePos.y += 0.35f;  // At the cup of the torch
        
        // Offset flame slightly forward from wall (into the room)
        float flameOffset = 0.1f;
        if (torch.rotation == 0.0f) {
            flamePos.z -= flameOffset;
        } else if (torch.rotation == 180.0f) {
            flamePos.z += flameOffset;
        } else if (torch.rotation == 90.0f) {
            flamePos.x += flameOffset;
        } else if (torch.rotation == 270.0f) {
            flamePos.x -= flameOffset;
        }

        // Render flame as vertical billboard (faces camera but stands upright)
        glm::vec3 cameraPos = m_camera.getPosition();
        glm::vec3 toCamera = glm::normalize(cameraPos - flamePos);
        
        // Calculate rotation to face camera (Y-axis billboard)
        float angle = atan2(toCamera.x, toCamera.z);
        
        // Flame scale with slight flicker variation
        float flicker = m_torchManager.getFlickerIntensity(i, m_gameTime);
        float flameScale = 0.22f * (0.95f + flicker * 0.05f);  // Smaller flame
        
        glm::mat4 MFlame = glm::translate(I, flamePos);
        MFlame = glm::rotate(MFlame, angle, glm::vec3(0.0f, 1.0f, 0.0f));  // Face camera horizontally
        MFlame = glm::rotate(MFlame, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));  // Stand upright
        MFlame = glm::scale(MFlame, glm::vec3(flameScale, flameScale, flameScale * 1.3f));  // Taller than wide (Z is now up)

        // Draw flame with texture alpha blending enabled
        drawObjectTex(planeModel, texFlame, P, V, MFlame, 0.0f, true);
    }
}
