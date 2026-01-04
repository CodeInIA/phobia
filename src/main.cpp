#include <GL/glew.h>
#include <GLFW/glfw3.h>
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
void configScene();
void renderScene();
void setLights(glm::mat4 P, glm::mat4 V);
void drawObjectMat(Model &model, Material &material, glm::mat4 P, glm::mat4 V, glm::mat4 M);
void drawObjectTex(Model &model, Textures &textures, glm::mat4 P, glm::mat4 V, glm::mat4 M);
void funFramebufferSize(GLFWwindow* window, int width, int height);
void funKey(GLFWwindow* window, int key , int scancode, int action, int mods);
void funScroll(GLFWwindow* window, double xoffset, double yoffset);
void funCursorPos(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow *window, float deltaTime);
bool checkCollision(glm::vec3 newPos);
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

// MODELOS
Model cubeModel;
Model planeModel;
Model flashlightModel;

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

// TEXTURAS (STRUCTS SHADER)
Textures texWall;
Textures texFloor;
Textures texCeiling;
Textures texFlashlight;

// LUCES
#define NLD 1
#define NLP 1
#define NLF 2
Light lightG;
Light lightD[NLD];
Light lightP[NLP];
Light lightF[NLF];
Material mluz;

int w = 1000;
int h = 1000;

// Control de tiempo (para velocidad constante)
float lastFrame = 0.0f;
float walkTime = 0.0f;
bool isWalking = false;
bool flashlightOn = true;

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
    std::string line;
    int rowIndex = 0;
    
    while (std::getline(file, line)) {
        if (!line.empty()) {
            for (int colIndex = 0; colIndex < line.size(); colIndex++) {
                if (line[colIndex] == 'N') {
                    float bSize = 4.0f;
                    cameraPos.x = colIndex * bSize;
                    cameraPos.z = rowIndex * bSize;
                    cameraPos.y = 2.0f;
                    line[colIndex] = '0';
                    std::cout << "Posicion inicial: grid(" << colIndex << ", " << rowIndex << ") -> world(" << cameraPos.x << ", " << cameraPos.z << ")" << std::endl;
                }
            }
            mapLevel.push_back(line);
            rowIndex++;
        }
    }
    file.close();
    
    std::cout << "Mapa cargado: " << mapLevel.size() << " filas" << std::endl;
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

    // --- CARGA DE TEXTURAS DE PARED ---
    imgWallDiffuse.initTexture("resources/textures/wall_diffuse.png");
    imgWallNormal.initTexture("resources/textures/wall_normal.jpg");
    imgWallSpecular.initTexture("resources/textures/wall_specular.jpg");
    imgWallEmissive.initTexture("resources/textures/wall_emissive.jpg");

    // --- CARGA DE TEXTURAS DE SUELO
    imgFloorDiffuse.initTexture("resources/textures/wall_diffuse.png");
    imgFloorSpecular.initTexture("resources/textures/wall_specular.jpg");
    imgFloorNormal.initTexture("resources/textures/wall_normal.jpg");
    imgFloorEmissive.initTexture("resources/textures/wall_emissive.jpg");

    // --- CARGA DE TEXTURAS DE TECHO
    imgCeilingDiffuse.initTexture("resources/textures/wall_diffuse.png");
    imgCeilingSpecular.initTexture("resources/textures/wall_specular.jpg");
    imgCeilingNormal.initTexture("resources/textures/wall_normal.jpg");
    imgCeilingEmissive.initTexture("resources/textures/wall_emissive.jpg");

    // --- CARGA DE TEXTURAS DE LINTERNA
    imgFlashlightDiffuse.initTexture("resources/textures/flashlight/DefaultMaterial.png");
    imgFlashlightSpecular.initTexture("resources/textures/flashlight/metalnessMap1.png");
    imgFlashlightNormal.initTexture("resources/textures/flashlight/normalMap1.png");
    imgFlashlightEmissive.initTexture("resources/textures/flashlight/emissiveMap1.png");

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
}

void renderScene(){
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shaders.useShaders();

    float aspect = (float)w/(float)h;
    glm::mat4 P = glm::perspective(glm::radians(60.0f), aspect, 0.1f, 100.0f);
    glm::mat4 V = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

    shaders.setVec3("ueye", cameraPos);
    setLights(P, V);

    // --- DIBUJAR SUELO CON TILES (REJILLA) ---
    float tileSize = 4.0f;
    int gridSizeX = 20;
    int gridSizeZ = 18;

    float scaleFloor = tileSize / 2.0f; // Dividir por 2 porque plane.obj mide 2x2

    for(int z = 0; z < gridSizeZ; z++) {
        for(int x = 0; x < gridSizeX; x++) {
            glm::vec3 posFloor(x * tileSize, 0.0f, z * tileSize);
            
            glm::mat4 MFloor = glm::translate(I, posFloor);
            MFloor = glm::scale(MFloor, glm::vec3(scaleFloor, 1.0f, scaleFloor));
            
            drawObjectTex(planeModel, texFloor, P, V, MFloor);
        }
    }

    // --- DIBUJAR TECHO CON TILES (REJILLA) ---
    float ceilingHeight = 5.0f;
    
    for(int z = 0; z < gridSizeZ; z++) {
        for(int x = 0; x < gridSizeX; x++) {
            glm::vec3 posCeiling(x * tileSize, ceilingHeight, z * tileSize);
            
            glm::mat4 MCeiling = glm::translate(I, posCeiling);
            MCeiling = glm::rotate(MCeiling, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            MCeiling = glm::scale(MCeiling, glm::vec3(scaleFloor, 1.0f, scaleFloor));
            
            drawObjectTex(planeModel, texCeiling, P, V, MCeiling);
        }
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
                drawObjectTex(cubeModel, texWall, P, V, M);
            }
        }
    }

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

// --- COLISIONES ---
bool checkCollision(glm::vec3 newPos) {
    float blockSize = 4.0f;
    float halfBlock = blockSize / 2.0f;
    float playerRadius = 0.4f;

    for (int z = 0; z < mapLevel.size(); z++) {
        for (int x = 0; x < mapLevel[z].size(); x++) {
            if (mapLevel[z][x] == '1') {
                float wallCenterX = x * blockSize;
                float wallCenterZ = z * blockSize;
                
                float distX = abs(newPos.x - wallCenterX);
                float distZ = abs(newPos.z - wallCenterZ);
                
                if (distX < halfBlock + playerRadius && distZ < halfBlock + playerRadius) {
                    return true;
                }
            }
        }
    }
    return false;
}

void processInput(GLFWwindow *window, float deltaTime) {
    float walkSpeed = 3.5f;
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
    cameraPos.y = 2.0f; 
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
}

void setLights(glm::mat4 P, glm::mat4 V) {
    shaders.setLight("ulightG",lightG);
    for(int i=0; i<NLD; i++) shaders.setLight("ulightD["+to_string(i)+"]",lightD[i]);
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

void drawObjectTex(Model &model, Textures &textures, glm::mat4 P, glm::mat4 V, glm::mat4 M) {
    shaders.setMat4("uN"  ,glm::transpose(glm::inverse(V*M)));
    shaders.setMat4("uM"  ,V*M);
    shaders.setMat4("uPVM",P*V*M);
    shaders.setBool("uWithMaterials",0);
    shaders.setBool("uWithNormals", textures.normal!=0);
    shaders.setTextures("utextures",textures);
    if(textures.diffuse!=0) model.renderModel(GL_FILL);
}

void funFramebufferSize(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height); w = width; h = height;
}

void funScroll(GLFWwindow* window, double xoffset, double yoffset) {}