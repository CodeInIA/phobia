#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include "Shaders.h"
#include "Model.h"
#include "Texture.h"

using namespace std;

// --- PROTOTIPOS ---
void configScene();
void renderScene();
void setLights(glm::mat4 P, glm::mat4 V);
void drawObjectMat(Model &model, Material &material, glm::mat4 P, glm::mat4 V, glm::mat4 M);
void drawObjectTex(Model &model, Textures &textures, glm::mat4 P, glm::mat4 V, glm::mat4 M);
void funFramebufferSize(GLFWwindow* window, int width, int height);
void funKey(GLFWwindow* window, int key , int scancode, int action, int mods);
void funScroll(GLFWwindow* window, double xoffset, double yoffset);
void funCursorPos(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow *window);
bool checkCollision(glm::vec3 newPos);

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
std::vector<std::string> mapLevel = {
    "11111111111111111111",
    "1N000000000000000001",
    "10111111011111101101",
    "10100001010000101001",
    "10101101010110101001",
    "10101100000110000001",
    "10000000000000111111",
    "11111101111110100001",
    "10000001000000101101",
    "10111101011110101101",
    "10100000010010000001",
    "10101111110011111101",
    "10000000000000000001",
    "11111011111111101111",
    "10000010000000101001",
    "10111110111110101001",
    "10000000100000000001",
    "11111111101111111111"
};

// MODELOS
Model cubeModel;
Model planeModel;

// TEXTURAS (OBJETOS)
Texture imgWallDiffuse;
Texture imgWallNormal;
Texture imgWallSpecular;
Texture imgNoEmissive; // Usaremos una oscura

// TEXTURAS (STRUCTS SHADER)
Textures texWall;
Textures texFloor;

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
        processInput(window);
        renderScene();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void configScene(){
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shaders.initShaders("resources/shaders/vshader.glsl","resources/shaders/fshader.glsl");

    cubeModel.initModel("resources/models/cube.obj");
    planeModel.initModel("resources/models/plane.obj");

    // --- CARGA DE TEXTURAS (Tus archivos nuevos) ---
    imgWallDiffuse.initTexture("resources/textures/wall_diffuse.png");  // Color
    imgWallNormal.initTexture("resources/textures/wall_normal.jpg");   // Normales
    imgWallSpecular.initTexture("resources/textures/wall_specular.jpg"); // Especular
    
    // Truco: Usamos img1 como "negro" provisional si no tienes imgNoEmissive.png
    // (Lo ideal es crear un png negro de 1x1 pixel)
    imgNoEmissive.initTexture("resources/textures/wall_emissive.jpg"); 

    // Configurar PARED
    texWall.diffuse   = imgWallDiffuse.getTexture();
    texWall.specular  = imgWallSpecular.getTexture();
    texWall.emissive  = imgNoEmissive.getTexture(); 
    texWall.normal    = imgWallNormal.getTexture(); // Activa Normal Map
    texWall.shininess = 50.0; // Brillo más definido

    // Configurar SUELO (Reutilizamos)
    texFloor.diffuse   = imgWallDiffuse.getTexture();
    texFloor.specular  = imgWallSpecular.getTexture();
    texFloor.emissive  = imgNoEmissive.getTexture();
    texFloor.normal    = imgWallNormal.getTexture();
    texFloor.shininess = 50.0;

    // --- LUCES ---
    lightG.ambient = glm::vec3(0.5, 0.5, 0.5);

    lightD[0].direction = glm::vec3(-1.0, -1.0, 0.0);
    lightD[0].ambient   = glm::vec3( 0.1, 0.1, 0.1);
    lightD[0].diffuse   = glm::vec3( 0.7, 0.7, 0.7);
    lightD[0].specular  = glm::vec3( 0.7, 0.7, 0.7);

    lightP[0].position = glm::vec3(0.0, 3.0, 3.0);
    lightP[0].ambient  = glm::vec3(0.2, 0.2, 0.2);
    lightP[0].diffuse  = glm::vec3(0.9, 0.9, 0.9);
    lightP[0].specular = glm::vec3(0.9, 0.9, 0.9);
    lightP[0].c0 = 1.00; lightP[0].c1 = 0.22; lightP[0].c2 = 0.20;

    lightF[0].position    = glm::vec3(0.0, 0.0, 0.0);
    lightF[0].direction   = glm::vec3(0.0, 0.0, -1.0);
    lightF[0].ambient     = glm::vec3(0.2, 0.2, 0.2);
    lightF[0].diffuse     = glm::vec3(0.9, 0.9, 0.9);
    lightF[0].specular    = glm::vec3(0.9, 0.9, 0.9);
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

    // SUELO
    glm::mat4 MSuelo = glm::translate(I, glm::vec3(40.0f, 0.0f, 40.0f)); 
    MSuelo = glm::scale(MSuelo, glm::vec3(80.0f, 1.0f, 80.0f)); 
    drawObjectTex(planeModel, texFloor, P, V, MSuelo);

    // LABERINTO
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
}

// --- COLISIONES MEJORADAS (Bounding Box) ---
bool checkCollision(glm::vec3 newPos) {
    float blockSize = 4.0f; 
    float offset    = blockSize / 2.0f; 
    float playerRadius = 0.4f; 

    float minX = newPos.x - playerRadius;
    float maxX = newPos.x + playerRadius;
    float minZ = newPos.z - playerRadius;
    float maxZ = newPos.z + playerRadius;

    int startGridX = (int)((minX + offset) / blockSize);
    int endGridX   = (int)((maxX + offset) / blockSize);
    int startGridZ = (int)((minZ + offset) / blockSize);
    int endGridZ   = (int)((maxZ + offset) / blockSize);

    for (int z = startGridZ; z <= endGridZ; z++) {
        for (int x = startGridX; x <= endGridX; x++) {
            if (z < 0 || z >= mapLevel.size() || x < 0 || x >= mapLevel[z].size()) return true;
            if (mapLevel[z][x] == '1') return true; 
        }
    }
    return false;
}

void processInput(GLFWwindow *window) {
    float cameraSpeed = 0.1f; 
    glm::vec3 frontXZ = glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z));
    glm::vec3 rightXZ = glm::normalize(glm::cross(cameraFront, cameraUp));
    glm::vec3 nextPos = cameraPos;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) nextPos += cameraSpeed * frontXZ;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) nextPos -= cameraSpeed * frontXZ;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) nextPos -= cameraSpeed * rightXZ;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) nextPos += cameraSpeed * rightXZ;
    
    if (!checkCollision(glm::vec3(nextPos.x, cameraPos.y, cameraPos.z))) cameraPos.x = nextPos.x;
    if (!checkCollision(glm::vec3(cameraPos.x, cameraPos.y, nextPos.z))) cameraPos.z = nextPos.z;
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