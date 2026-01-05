#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "Shaders.h"
#include "Model.h"
#include "Texture.h"

using namespace std;

// --- PROTOTIPOS ---
void loadMap(const std::string& filename);
// --- PROTOTIPOS ---
void loadMap(const std::string& filename);
void configScene();
void renderScene();
void setLights(glm::mat4 P, glm::mat4 V);
void drawObjectMat(Model &model, Material &material, glm::mat4 P, glm::mat4 V, glm::mat4 M);
void drawObjectTex(Model &model, Textures &textures, glm::mat4 P, glm::mat4 V, glm::mat4 M, float alpha = 0.0f);
void funFramebufferSize(GLFWwindow* window, int width, int height);
void funKey(GLFWwindow* window, int key , int scancode, int action, int mods);
void funScroll(GLFWwindow* window, double xoffset, double yoffset);
void funCursorPos(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow *window, float deltaTime);
bool checkCollision(glm::vec3 newPos);
void renderDoors(glm::mat4 P, glm::mat4 V);
void updateDoors(float deltaTime);
glm::mat4 calculateFlashlightTransform(glm::vec3 &flashlightTipOut);
void updateFlashlightLight(const glm::vec3 &flashlightTip);
void renderFlashlight(glm::mat4 P, glm::mat4 V);

// --- VARIABLES GLOBALES ---
Shaders shaders;
#define I glm::mat4(1.0)

// --- JUEGO ---
glm::vec3 cameraPos   = glm::vec3(4.0f, 2.0f,  4.0f); 
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
float yaw   = -90.0f;
float pitch =  0.0f;
float lastX =  500.0f;
float lastY =  500.0f;
bool firstMouse = true;

// MAPA
std::vector<std::string> mapLevel;

// PUERTAS - tipos de puertas disponibles
enum DoorType {
    NORMAL,      // Door03 - bloody hand door
    FIRE_EXIT    // Door04 - fire exit door
};

// PUERTAS - struct para rastrear posición, orientación y estado
struct Door {
    glm::ivec2 gridPos;    // Posición en el grid del mapa
    float rotation;         // Rotación en Y (0 = orientada en Z, 90 = orientada en X)
    bool isOpen;            // Estado de la puerta
    float openAngle;        // Ángulo actual de apertura (para animación)
    DoorType type;          // Tipo de puerta (NORMAL o FIRE_EXIT)
};
std::vector<Door> doors;

// Door function prototypes (after Door struct)
Door* findNearestDoor(float maxDistance);

// MODELOS
Model cubeModel;
Model planeModel;
Model flashlightModel;

// Door03 (Normal/Bloody hand door)
Model doorNormalFrame;
Model doorNormalDoor;
Model doorNormalGlass;

// Door04 (Fire exit door)
Model doorExitFrame;
Model doorExitDoor;
Model doorExitExtra1;  // Fire exit sign
Model doorExitExtra2;  // Push bar

// TEXTURAS (OBJETOS) - SEPARADAS PARA PARED Y SUELO
Texture imgWallDiffuse;
Texture imgWallNormal;
Texture imgWallSpecular;
Texture imgWallEmissive;

Texture imgFloorDiffuse;
Texture imgFloorSpecular;
Texture imgFloorNormal;
Texture imgFloorEmissive;

Texture imgCeilingDiffuse;
Texture imgCeilingSpecular;
Texture imgCeilingNormal;
Texture imgCeilingEmissive;

Texture imgFlashlightDiffuse;
Texture imgFlashlightSpecular;
Texture imgFlashlightNormal;
Texture imgFlashlightEmissive;

// TEXTURAS PUERTA
Texture imgDoorDiffuse;
Texture imgDoorSpecular;
Texture imgDoorNormal;
Texture imgDoorGlassDiffuse;

// TEXTURAS (STRUCTS SHADER)
Textures texWall;
Textures texFloor;
Textures texCeiling;
Textures texFlashlight;
Textures texDoorFrame;
Textures texDoorGlass;

// LUCES
#define NLD 1
#define NLP 1
#define NLF 2
Light lightG;
Light lightD[NLD];
Light lightP[NLP];
Light lightF[NLF];
Material mluz;
Material mBlackWall;

int w = 1000;
int h = 1000;

// Control de tiempo (para velocidad constante)
float lastFrame = 0.0f;
float walkTime = 0.0f;
bool isWalking = false;
bool flashlightOn = true;
bool topViewMode = false;

// --- MAIN ---
int main() {
    if(!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(w,h, "Proyecto Final", NULL, NULL);
    if(!window){ glfwTerminate(); return -1; }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glewExperimental = GL_TRUE;
    if(GLEW_OK != glewInit()) return -1;
    
    glfwSetFramebufferSizeCallback(window, funFramebufferSize);
    glfwSetKeyCallback(window, funKey);
    glfwSetScrollCallback (window, funScroll);
    glfwSetCursorPosCallback(window, funCursorPos);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    configScene();

    while(!glfwWindowShouldClose(window)){
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, deltaTime);
        updateDoors(deltaTime);
        renderScene();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void loadMap(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo " << filename << std::endl;
        return;
    }
    
    mapLevel.clear();
    doors.clear();  // Limpiar puertas anteriores
    std::string line;
    int rowIndex = 0;
    
    while (std::getline(file, line)) {
        if (!line.empty()) {
            for (int colIndex = 0; colIndex < line.size(); colIndex++) {
                if (line[colIndex] == 'N') {
                    float bSize = 4.0f;
                    cameraPos.x = colIndex * bSize;
                    cameraPos.z = rowIndex * bSize;
                    cameraPos.y = 6.0f;
                    line[colIndex] = '0';
                    std::cout << "Posicion inicial: grid(" << colIndex << ", " << rowIndex << ") -> world(" << cameraPos.x << ", " << cameraPos.z << ")" << std::endl;
                }
                // Detectar puertas - 'D' para normal, 'E' para fire exit
                if (line[colIndex] == 'D' || line[colIndex] == 'E') {
                    Door door;
                    door.gridPos = glm::ivec2(colIndex, rowIndex);
                    door.isOpen = false;
                    door.openAngle = 0.0f;
                    door.type = (line[colIndex] == 'D') ? NORMAL : FIRE_EXIT;
                    
                    // Determinar orientación basada en paredes adyacentes
                    // Si hay paredes arriba/abajo, la puerta está orientada en X (rotación 90)
                    // Si hay paredes izquierda/derecha, la puerta está orientada en Z (rotación 0)
                    door.rotation = 0.0f; // Por defecto orientada en Z
                    
                    doors.push_back(door);
                    std::cout << "Puerta encontrada en grid(" << colIndex << ", " << rowIndex << ") - Tipo: " 
                              << (door.type == NORMAL ? "Normal" : "Fire Exit") << std::endl;
                }
            }
            mapLevel.push_back(line);
            rowIndex++;
        }
    }
    file.close();
    
    // Segunda pasada para determinar orientación de puertas
    for (auto& door : doors) {
        int x = door.gridPos.x;
        int z = door.gridPos.y;
        
        // Verificar paredes adyacentes
        bool wallLeft  = (x > 0 && mapLevel[z][x-1] == '1');
        bool wallRight = (x < mapLevel[z].size()-1 && mapLevel[z][x+1] == '1');
        bool wallUp    = (z > 0 && mapLevel[z-1][x] == '1');
        bool wallDown  = (z < mapLevel.size()-1 && mapLevel[z+1][x] == '1');
        
        // Si hay paredes arriba/abajo, puerta orientada horizontalmente (en eje X)
        if (wallUp || wallDown) {
            door.rotation = 90.0f;
        }
        // Si hay paredes izquierda/derecha, puerta orientada verticalmente (en eje Z) 
        else if (wallLeft || wallRight) {
            door.rotation = 0.0f;
        }
        
        std::cout << "Puerta en (" << x << ", " << z << ") orientacion: " << door.rotation << " grados" << std::endl;
    }
    
    std::cout << "Mapa cargado: " << mapLevel.size() << " filas, " << doors.size() << " puertas" << std::endl;
}

void configScene(){
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Habilitar culling de caras traseras
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    loadMap("resources/models/map.cub");

    shaders.initShaders("resources/shaders/vshader.glsl","resources/shaders/fshader.glsl");

    cubeModel.initModel("resources/models/cube.obj");
    planeModel.initModel("resources/models/plane.obj");
    flashlightModel.initModel("resources/models/flashlight.obj");
    
    // Load Door03 (Normal/Bloody hand door)
    doorNormalFrame.initModel("resources/models/doors/normal_door/Door03_Frame.obj");
    doorNormalDoor.initModel("resources/models/doors/normal_door/Door03_Door.obj");
    doorNormalGlass.initModel("resources/models/doors/normal_door/Door03_Glass.obj");
    
    // Load Door04 (Fire exit door)
    doorExitFrame.initModel("resources/models/doors/fire_exit/Door04_Frame.obj");
    doorExitDoor.initModel("resources/models/doors/fire_exit/Door04_Door.obj");
    doorExitExtra1.initModel("resources/models/doors/fire_exit/Door04_Extra01.obj");
    doorExitExtra2.initModel("resources/models/doors/fire_exit/Door04_Extra02.obj");

    // --- CARGA DE TEXTURAS DE PARED ---
    imgWallDiffuse.initTexture("resources/textures/wall/wall_diffuse.png");
    imgWallNormal.initTexture("resources/textures/wall/wall_normal.jpg");
    imgWallSpecular.initTexture("resources/textures/wall/wall_specular.jpg");
    imgWallEmissive.initTexture("resources/textures/wall/wall_emissive.jpg");

    // --- CARGA DE TEXTURAS DE SUELO
    imgFloorDiffuse.initTexture("resources/textures/wall/wall_diffuse.png");
    imgFloorSpecular.initTexture("resources/textures/wall/wall_specular.jpg");
    imgFloorNormal.initTexture("resources/textures/wall/wall_normal.jpg");
    imgFloorEmissive.initTexture("resources/textures/wall/wall_emissive.jpg");

    // --- CARGA DE TEXTURAS DE TECHO
    imgCeilingDiffuse.initTexture("resources/textures/wall/wall_diffuse.png");
    imgCeilingSpecular.initTexture("resources/textures/wall/wall_specular.jpg");
    imgCeilingNormal.initTexture("resources/textures/wall/wall_normal.jpg");
    imgCeilingEmissive.initTexture("resources/textures/wall/wall_emissive.jpg");

    // --- CARGA DE TEXTURAS DE LINTERNA
    imgFlashlightDiffuse.initTexture("resources/textures/flashlight/DefaultMaterial.png");
    imgFlashlightSpecular.initTexture("resources/textures/flashlight/metalnessMap1.png");
    imgFlashlightNormal.initTexture("resources/textures/flashlight/normalMap1.png");
    imgFlashlightEmissive.initTexture("resources/textures/flashlight/emissiveMap1.png");

    // --- CARGA DE TEXTURAS DE PUERTA (DoorPack atlas - shared by all doors)
    imgDoorDiffuse.initTexture("resources/textures/doors/DoorPack_Base.jpeg");
    imgDoorSpecular.initTexture("resources/textures/doors/DoorPack_Metallic.jpeg");
    imgDoorNormal.initTexture("resources/textures/doors/DoorPack_Normal.png");
    imgDoorGlassDiffuse.initTexture("resources/textures/doors/DoorPack_Opacity.jpeg");

    // Configurar PARED
    texWall.diffuse   = imgWallDiffuse.getTexture();
    texWall.specular  = imgWallSpecular.getTexture();
    texWall.emissive  = imgWallEmissive.getTexture(); 
    texWall.normal    = imgWallNormal.getTexture();
    texWall.shininess = 64.0;

    // Configurar SUELO
    texFloor.diffuse   = imgFloorDiffuse.getTexture();
    texFloor.specular  = imgFloorSpecular.getTexture();
    texFloor.emissive  = imgFloorEmissive.getTexture();
    texFloor.normal    = imgFloorNormal.getTexture();
    texFloor.shininess = 64.0;

    // Configurar TECHO
    texCeiling.diffuse   = imgCeilingDiffuse.getTexture();
    texCeiling.specular  = imgCeilingSpecular.getTexture();
    texCeiling.emissive  = imgCeilingEmissive.getTexture();
    texCeiling.normal    = imgCeilingNormal.getTexture();
    texCeiling.shininess = 64.0;

    // Configurar LINTERNA
    texFlashlight.diffuse   = imgFlashlightDiffuse.getTexture();
    texFlashlight.specular  = imgFlashlightSpecular.getTexture();
    texFlashlight.emissive  = imgFlashlightEmissive.getTexture();
    texFlashlight.normal    = imgFlashlightNormal.getTexture();
    texFlashlight.shininess = 32.0;

    // Configurar PUERTA (marco)
    texDoorFrame.diffuse   = imgDoorDiffuse.getTexture();
    texDoorFrame.specular  = imgDoorSpecular.getTexture();
    texDoorFrame.emissive  = 0;  // Sin emisión
    texDoorFrame.normal    = imgDoorNormal.getTexture();
    texDoorFrame.shininess = 32.0;

    // Configurar PUERTA (cristal) - para transparencia
    texDoorGlass.diffuse   = imgDoorGlassDiffuse.getTexture();
    texDoorGlass.specular  = imgDoorSpecular.getTexture();
    texDoorGlass.emissive  = 0;
    texDoorGlass.normal    = 0;  // Sin normal map para cristal
    texDoorGlass.shininess = 96.0;  // Más brillante

    // --- LUCES ---
    lightG.ambient = glm::vec3(0.1, 0.1, 0.12);

    lightD[0].direction = glm::vec3(-1.0, -1.0, 0.0);
    lightD[0].ambient   = glm::vec3( 0.03, 0.03, 0.03);
    lightD[0].diffuse   = glm::vec3( 0.2, 0.2, 0.25);
    lightD[0].specular  = glm::vec3( 0.08, 0.08, 0.08);

    lightP[0].position = glm::vec3(40.0, 10.0, 40.0);
    lightP[0].ambient  = glm::vec3(0.01, 0.01, 0.01);
    lightP[0].diffuse  = glm::vec3(0.15, 0.15, 0.2);  // Muy reducida
    lightP[0].specular = glm::vec3(0.05, 0.05, 0.05); // Brillos mínimos
    lightP[0].c0 = 1.00; lightP[0].c1 = 0.05; lightP[0].c2 = 0.01; // Atenuación más rápida

    lightF[0].position    = glm::vec3(0.0, 0.0, 0.0);
    lightF[0].direction   = glm::vec3(0.0, 0.0, -1.0);
    lightF[0].ambient     = glm::vec3(0.0, 0.0, 0.0);
    lightF[0].diffuse     = glm::vec3(0.8, 0.7, 0.6);
    lightF[0].specular    = glm::vec3(0.3, 0.3, 0.3);
    lightF[0].innerCutOff = 10.0;
    lightF[0].outerCutOff = 15.0;
    lightF[0].c0 = 1.0; lightF[0].c1 = 0.09; lightF[0].c2 = 0.032;
    
    lightF[1] = lightF[0]; 

    mluz.ambient = glm::vec4(0); mluz.diffuse = glm::vec4(0); 
    mluz.specular = glm::vec4(0); mluz.emissive = glm::vec4(1); mluz.shininess = 1.0;
    
    mBlackWall.ambient = glm::vec4(0.05, 0.05, 0.05, 1.0);
    mBlackWall.diffuse = glm::vec4(0.05, 0.05, 0.05, 1.0);
    mBlackWall.specular = glm::vec4(0.0, 0.0, 0.0, 1.0);
    mBlackWall.emissive = glm::vec4(0.0, 0.0, 0.0, 1.0);
    mBlackWall.shininess = 1.0;
}

void drawTiledPlane(Model &model, Textures &tex, glm::mat4 P, glm::mat4 V, int gridX, int gridZ, float tileSize, float yPos, float scale, bool flip) {
    for(int z = 0; z < gridZ; z++) {
        for(int x = 0; x < gridX; x++) {
            glm::vec3 pos(x * tileSize, yPos, z * tileSize);
            glm::mat4 M = glm::translate(I, pos);
            if(flip) M = glm::rotate(M, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            M = glm::scale(M, glm::vec3(scale, 1.0f, scale));
            drawObjectTex(model, tex, P, V, M);
        }
    }
}

void renderScene(){
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shaders.useShaders();

    float aspect = (float)w/(float)h;
    glm::mat4 P = glm::perspective(glm::radians(60.0f), aspect, 0.1f, 150.0f);
    glm::mat4 V;
    
    if (topViewMode) {
        // Vista cenital: cámara desde arriba mostrando todo el laberinto
        float tileSize = 4.0f;
        int gridSizeX = 20;
        int gridSizeZ = 18;
        
        // Centro del mapa
        float mapCenterX = (gridSizeX * tileSize) / 2.0f;
        float mapCenterZ = (gridSizeZ * tileSize) / 2.0f;
        
        // Calcular altura necesaria para que quepa todo el mapa
        float mapWidth = gridSizeX * tileSize;
        float mapHeight = gridSizeZ * tileSize;
        float maxDimension = glm::max(mapWidth / aspect, mapHeight);
        float cameraHeight = maxDimension / (2.0f * tan(glm::radians(30.0f))); // 60 grados FOV / 2
        
        glm::vec3 topViewPos = glm::vec3(mapCenterX, cameraHeight, mapCenterZ);
        glm::vec3 topViewTarget = glm::vec3(mapCenterX, 0.0f, mapCenterZ);
        V = glm::lookAt(topViewPos, topViewTarget, glm::vec3(0.0f, 0.0f, -1.0f));
    } else {
        // Vista primera persona normal
        V = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    }

    shaders.setVec3("ueye", cameraPos);
    setLights(P, V);

    // --- DIBUJAR SUELO CON TILES (REJILLA) ---
    float tileSize = 4.0f;
    int gridSizeZ = mapLevel.size();
    int gridSizeX = (gridSizeZ > 0) ? mapLevel[0].size() : 0;

    float scaleFloor = tileSize / 2.0f; // Dividir por 2 porque plane.obj mide 2x2

    drawTiledPlane(planeModel, texFloor, P, V, gridSizeX, gridSizeZ, tileSize, 0.0f, scaleFloor, false);

    // --- DIBUJAR TECHO CON TILES (REJILLA) ---
    if (!topViewMode) {
        float ceilingHeight = 5.0f;
        drawTiledPlane(planeModel, texCeiling, P, V, gridSizeX, gridSizeZ, tileSize, ceilingHeight, scaleFloor, true);
    }

    // --- DIBUJAR PAREDES ---
    float bSize = 4.0f; 
    float bH    = 5.0f; 
    float scX = bSize / 2.0f;
    float scY = bH    / 2.0f;
    float scZ = bSize / 2.0f;

    for(int z = 0; z < mapLevel.size(); z++) {
        for(int x = 0; x < mapLevel[z].size(); x++) {
            if(mapLevel[z][x] == '1') {
                glm::vec3 pos(x * bSize, bH / 2.0f, z * bSize);
                glm::mat4 M = glm::translate(I, pos);
                M = glm::scale(M, glm::vec3(scX, scY, scZ));
                if (topViewMode) {
                    drawObjectMat(cubeModel, mBlackWall, P, V, M);
                } else {
                    drawObjectTex(cubeModel, texWall, P, V, M);
                }
            }
        }
    }

    // --- RENDERIZAR PUERTAS ---
    renderDoors(P, V);

    // --- RENDERIZAR LINTERNA EN PRIMERA PERSONA ---
    renderFlashlight(P, V);
}

// --- FUNCIONES DE LINTERNA ---
glm::mat4 calculateFlashlightTransform(glm::vec3 &flashlightTipOut) {
    // Calcular vectores de orientación
    glm::vec3 rightVector = glm::normalize(glm::cross(cameraFront, cameraUp));
    glm::vec3 upVector = glm::normalize(glm::cross(rightVector, cameraFront));
    
    // Efecto de balanceo al caminar
    float swayOffsetX = 0.0f;
    float swayOffsetY = 0.0f;
    if (isWalking) {
        float swaySpeed = 8.0f;
        float swayAmount = 0.015f;
        swayOffsetX = sin(walkTime * swaySpeed) * swayAmount;
        swayOffsetY = abs(cos(walkTime * swaySpeed * 0.5f)) * swayAmount * 0.5f;
    }
    
    // Offset para brazo derecho - esquina inferior derecha con balanceo
    glm::vec3 flashlightPos = cameraPos 
        + cameraFront * 0.25f           // Adelante (reducido para que no atraviese paredes)
        + rightVector * (0.1f + swayOffsetX)  // A la derecha + balanceo horizontal
        - upVector * (0.10f - swayOffsetY);   // Abajo + balanceo vertical
    
    // Crear matriz de orientación usando los vectores de la cámara
    glm::mat4 MFlashlight(1.0f);
    MFlashlight[0] = glm::vec4(cameraFront, 0.0f);   // X axis -> hacia adelante
    MFlashlight[1] = glm::vec4(upVector, 0.0f);      // Y axis -> arriba
    MFlashlight[2] = glm::vec4(rightVector, 0.0f);   // Z axis -> derecha
    MFlashlight[3] = glm::vec4(flashlightPos, 1.0f); // Posición
    
    // Escalar
    MFlashlight = glm::scale(MFlashlight, glm::vec3(0.15f, 0.15f, 0.15f));
    
    // Calcular posición de la punta de la linterna para la luz
    float flashlightLength = 0.15f; // 1.0 * escala
    flashlightTipOut = flashlightPos + cameraFront * flashlightLength;
    
    return MFlashlight;
}

void updateFlashlightLight(const glm::vec3 &flashlightTip) {
    lightF[0].position = flashlightTip;
    lightF[0].direction = cameraFront;
}

void renderFlashlight(glm::mat4 P, glm::mat4 V) {
    glm::vec3 flashlightTip;
    glm::mat4 MFlashlight = calculateFlashlightTransform(flashlightTip);
    
    drawObjectTex(flashlightModel, texFlashlight, P, V, MFlashlight);
    
    updateFlashlightLight(flashlightTip);
}

// --- RENDERIZADO DE PUERTAS ---
// Helper to create transform for a door part
glm::mat4 createDoorPartTransform(glm::vec3 doorPos, float rotation, float scaleX, float scaleY, float scaleZ, 
                                   glm::vec3 partCenter, float partHeight, float doorOpenAngle = 0.0f, bool isMovingPart = false) {
    glm::mat4 M = glm::translate(I, doorPos);
    M = glm::rotate(M, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
    
    // Apply door opening rotation for moving parts (door leaf, glass, extras)
    if (isMovingPart && doorOpenAngle != 0.0f) {
        // The door rotates around its left edge (negative X in local space)
        // Move pivot to the edge, rotate, then move back
        float doorWidth = partCenter.x * 2.0f; // Approximate door width from center
        M = glm::translate(M, glm::vec3(-doorWidth * scaleX * 0.5f, 0.0f, 0.0f)); // Move to hinge edge
        M = glm::rotate(M, glm::radians(doorOpenAngle), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate around hinge
        M = glm::translate(M, glm::vec3(doorWidth * scaleX * 0.5f, 0.0f, 0.0f)); // Move back
    }
    
    M = glm::scale(M, glm::vec3(scaleX, scaleY, scaleZ));
    M = glm::translate(M, -partCenter);
    M = glm::translate(M, glm::vec3(0.0f, partHeight / 2.0f, 0.0f));
    return M;
}

void drawDoorComponent(Model &model, Textures &tex, glm::mat4 P, glm::mat4 V, glm::vec3 pos, float rot, glm::vec3 scale, float openAngle, bool moving, float alpha = 0.0f) {
    float h = model.getSize().y;
    glm::mat4 M = createDoorPartTransform(pos, rot, scale.x, scale.y, scale.z, model.getCenter(), h, openAngle, moving);
    if(alpha > 0.0f) glDepthMask(GL_FALSE);
    drawObjectTex(model, tex, P, V, M, alpha);
    if(alpha > 0.0f) glDepthMask(GL_TRUE);
}

void renderDoors(glm::mat4 P, glm::mat4 V) {
    float bSize = 4.0f;
    float bH = 5.0f;
    
    glDisable(GL_CULL_FACE);
    
    for (const auto& door : doors) {
        glm::vec3 doorPos(door.gridPos.x * bSize, 0.0f, door.gridPos.y * bSize);
        
        if (door.type == NORMAL) {
            glm::vec3 fSize = doorNormalFrame.getSize();
            glm::vec3 scale( (bSize * 0.95f) / fSize.x, bH / fSize.y, (bSize * 0.95f) / fSize.x );
            float rot = door.rotation + 180.0f;
            
            drawDoorComponent(doorNormalFrame, texDoorFrame, P, V, doorPos, rot, scale, 0.0f, false);
            drawDoorComponent(doorNormalDoor, texDoorFrame, P, V, doorPos, rot, scale, door.openAngle, true);
            drawDoorComponent(doorNormalGlass, texDoorGlass, P, V, doorPos, rot, scale, door.openAngle, true, 0.35f);
            
        } else { // FIRE_EXIT
            glm::vec3 fSize = doorExitFrame.getSize();
            glm::vec3 scale( (bSize * 0.95f) / fSize.x, bH / fSize.y, (bSize * 0.95f) / fSize.x );
            
            drawDoorComponent(doorExitFrame, texDoorFrame, P, V, doorPos, door.rotation, scale, 0.0f, false);
            drawDoorComponent(doorExitDoor, texDoorFrame, P, V, doorPos, door.rotation, scale, door.openAngle, true);
            drawDoorComponent(doorExitExtra1, texDoorFrame, P, V, doorPos, door.rotation, scale, door.openAngle, true);
            drawDoorComponent(doorExitExtra2, texDoorFrame, P, V, doorPos, door.rotation, scale, door.openAngle, true);
        }
    }
    
    glEnable(GL_CULL_FACE);
}

// --- COLISIONES ---
// Helper for collision vs rectangle (wall/device)
bool checkRectOverlap(glm::vec3 pos, glm::vec3 center, float halfSize, float radius) {
    float distX = abs(pos.x - center.x);
    float distZ = abs(pos.z - center.z);
    return (distX < halfSize + radius && distZ < halfSize + radius);
}

bool checkCollision(glm::vec3 newPos) {
    float blockSize = 4.0f;
    float halfBlock = blockSize / 2.0f;
    float playerRadius = 0.4f;

    // Check wall collisions
    for (int z = 0; z < mapLevel.size(); z++) {
        for (int x = 0; x < mapLevel[z].size(); x++) {
            if (mapLevel[z][x] == '1') {
                if(checkRectOverlap(newPos, glm::vec3(x * blockSize, 0.0f, z * blockSize), halfBlock, playerRadius))
                    return true;
            }
        }
    }
    
    // Check closed door collisions
    for (const auto& door : doors) {
        if (!door.isOpen) {  // Only collide with closed doors
            if(checkRectOverlap(newPos, glm::vec3(door.gridPos.x * blockSize, 0.0f, door.gridPos.y * blockSize), halfBlock, playerRadius))
                return true;
        }
    }
    
    return false;
}

// --- DOOR INTERACTION ---
Door* findNearestDoor(float maxDistance) {
    Door* nearestDoor = nullptr;
    float minDistance = maxDistance;
    float blockSize = 4.0f;
    
    for (auto& door : doors) {
        float doorCenterX = door.gridPos.x * blockSize;
        float doorCenterZ = door.gridPos.y * blockSize;
        
        float dist = glm::distance(cameraPos, glm::vec3(doorCenterX, cameraPos.y, doorCenterZ));
        
        if (dist < minDistance) {
            minDistance = dist;
            nearestDoor = &door;
        }
    }
    
    return nearestDoor;
}

// --- DOOR ANIMATION ---
void updateDoors(float deltaTime) {
    const float openSpeed = 90.0f; // degrees per second
    const float maxOpenAngle = -90.0f; // Negative for opening the other way
    
    for (auto& door : doors) {
        if (door.isOpen && door.openAngle > maxOpenAngle) {
            door.openAngle -= openSpeed * deltaTime;
            if (door.openAngle < maxOpenAngle) {
                door.openAngle = maxOpenAngle;
            }
        } else if (!door.isOpen && door.openAngle < 0.0f) {
            door.openAngle += openSpeed * deltaTime;
            if (door.openAngle > 0.0f) {
                door.openAngle = 0.0f;
            }
        }
    }
}

void processInput(GLFWwindow *window, float deltaTime) {
    float walkSpeed = 7.0f;
    float cameraSpeed = walkSpeed * deltaTime;
    
    glm::vec3 frontXZ = glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z));
    glm::vec3 rightXZ = glm::normalize(glm::cross(cameraFront, cameraUp));
    glm::vec3 nextPos = cameraPos;

    bool moved = false;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { nextPos += cameraSpeed * frontXZ; moved = true; }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) { nextPos -= cameraSpeed * frontXZ; moved = true; }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { nextPos -= cameraSpeed * rightXZ; moved = true; }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { nextPos += cameraSpeed * rightXZ; moved = true; }
    
    if (moved) {
        isWalking = true;
        walkTime += deltaTime;
        bool collX = checkCollision(glm::vec3(nextPos.x, cameraPos.y, cameraPos.z));
        bool collZ = checkCollision(glm::vec3(cameraPos.x, cameraPos.y, nextPos.z));
        
        if (!collX) cameraPos.x = nextPos.x;
        if (!collZ) cameraPos.z = nextPos.z;
    } else {
        isWalking = false;
    }
    cameraPos.y = 3.2f; 
}

void funCursorPos(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float xoffset = (xpos - lastX) * 0.1f;
    float yoffset = (lastY - ypos) * 0.1f; 
    lastX = xpos; lastY = ypos;
    yaw += xoffset; pitch += yoffset;
    if(pitch > 89.0f) pitch = 89.0f; if(pitch < -89.0f) pitch = -89.0f;
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void funKey(GLFWwindow* window, int key , int scancode, int action, int mods) {
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
    if(key == GLFW_KEY_F && action == GLFW_PRESS) flashlightOn = !flashlightOn;
    if(key == GLFW_KEY_U && action == GLFW_PRESS) topViewMode = !topViewMode;
    
    // E key to interact with nearest door
    if(key == GLFW_KEY_E && action == GLFW_PRESS) {
        Door* nearestDoor = findNearestDoor(6.0f);
        if (nearestDoor) {
            nearestDoor->isOpen = !nearestDoor->isOpen;
            std::cout << "Door " << (nearestDoor->isOpen ? "opened" : "closed") << std::endl;
        }
    }
}

void setLights(glm::mat4 P, glm::mat4 V) {
    // En modo vista cenital, aumentar el brillo
    Light modifiedLightG = lightG;
    if (topViewMode) {
        modifiedLightG.ambient = glm::vec3(0.6, 0.6, 0.65);
    }
    shaders.setLight("ulightG", modifiedLightG);
    
    for(int i=0; i<NLD; i++) {
        Light light = lightD[i];
        if (topViewMode) {
            light.ambient = glm::vec3(0.2, 0.2, 0.2);
            light.diffuse = glm::vec3(0.8, 0.8, 0.9);
            light.specular = glm::vec3(0.3, 0.3, 0.3);
        }
        shaders.setLight("ulightD["+to_string(i)+"]", light);
    }
    
    for(int i=0; i<NLP; i++) {
        Light light = lightP[i];
        light.position = glm::vec3(V * glm::vec4(light.position, 1.0));
        shaders.setLight("ulightP["+to_string(i)+"]",light);
    }
    for(int i=0; i<NLF; i++) {
        Light light = lightF[i];
        light.position = glm::vec3(V * glm::vec4(light.position, 1.0));
        light.direction = glm::vec3(V * glm::vec4(light.direction, 0.0));
        if (!flashlightOn) {
            light.diffuse = glm::vec3(0.0, 0.0, 0.0);
            light.specular = glm::vec3(0.0, 0.0, 0.0);
        }
        shaders.setLight("ulightF["+to_string(i)+"]",light);
    }
}

void drawObjectMat(Model &model, Material &material, glm::mat4 P, glm::mat4 V, glm::mat4 M) {
    shaders.setMat4("uN"  ,glm::transpose(glm::inverse(V*M)));
    shaders.setMat4("uM"  ,V*M);
    shaders.setMat4("uPVM",P*V*M);
    shaders.setBool("uWithMaterials",1);
    shaders.setMaterial("umaterial",material);
    model.renderModel(GL_FILL);
}

void drawObjectTex(Model &model, Textures &textures, glm::mat4 P, glm::mat4 V, glm::mat4 M, float alpha) {
    shaders.setMat4("uN"  ,glm::transpose(glm::inverse(V*M)));
    shaders.setMat4("uM"  ,V*M);
    shaders.setMat4("uPVM",P*V*M);
    shaders.setBool("uWithMaterials",0);
    shaders.setBool("uWithNormals", textures.normal!=0);
    shaders.setFloat("uAlpha", alpha);  // Usar alpha de textura por defecto
    shaders.setTextures("utextures",textures);
    if(textures.diffuse!=0) model.renderModel(GL_FILL);
}

void funFramebufferSize(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height); w = width; h = height;
}

void funScroll(GLFWwindow* window, double xoffset, double yoffset) {}