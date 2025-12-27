#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include "Shaders.h"
#include "Model.h"
#include "Texture.h"

using namespace std;

// --- PROTOTIPOS DE FUNCIONES ---
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

// --- VARIABLES GLOBALES DEL MOTOR ---
Shaders shaders;

// --- VARIABLES DEL JUEGO ---

// 1. CÁMARA FPS
// CORRECCIÓN: (4.0, 2.0, 4.0) corresponde a la casilla [1][1] (la 'N')
// porque cada bloque mide 4.0 unidades.
glm::vec3 cameraPos   = glm::vec3(4.0f, 2.0f,  4.0f); 
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
float yaw   = -90.0f;
float pitch =  0.0f;
float lastX =  500.0f;
float lastY =  500.0f;
bool firstMouse = true;

// 2. MAPA DEL NIVEL (20x20)
// 1 = Pared, 0 = Pasillo, N = Inicio
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

// 3. MODELOS Y TEXTURAS
Model cubeModel;
Model planeModel;

Textures texWall;
Textures texFloor;
Texture  auxLoader; 

// 4. LUCES Y MATERIALES
#define NLD 1
#define NLP 1
#define NLF 2

Light lightG;
Light lightD[NLD];
Light lightP[NLP];
Light lightF[NLF];
Material mluz;

// Viewport
int w = 1000;
int h = 1000;

// --- MAIN ---
int main() {

    // GLFW init
    if(!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window;
    window = glfwCreateWindow(w,h, "Proyecto Final - Laberinto 3D", NULL, NULL);

    if(!window){
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // GLEW init
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if(GLEW_OK != err) {
        cout << "Error: " << glewGetErrorString(err) << endl;
        return -1;
    }
    
    // Callbacks config
    glfwSetFramebufferSizeCallback(window, funFramebufferSize);
    glfwSetKeyCallback(window, funKey);
    glfwSetScrollCallback (window, funScroll);
    glfwSetCursorPosCallback(window, funCursorPos);
    
    // Capturar ratón (Mouse Capture)
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Config scene
    configScene();

    // Render loop
    while(!glfwWindowShouldClose(window)){
        processInput(window); // Procesar movimiento + Colisiones
        renderScene();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

// -------------------------------------------------------------------------------------
// CONFIGURACIÓN DE LA ESCENA (CARGA DE RECURSOS)
// -------------------------------------------------------------------------------------
void configScene(){

    // Depth test
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Shaders
    shaders.initShaders("resources/shaders/vshader.glsl","resources/shaders/fshader.glsl");

    // --- CARGAR MODELOS ---
    cubeModel.initModel("resources/models/cube.obj");
    planeModel.initModel("resources/models/plane.obj");

    // --- CARGAR TEXTURAS ---
    auxLoader.initTexture("resources/textures/test_wall.png");
    unsigned int idTex = auxLoader.getTexture();

    // Configurar textura Pared
    texWall.diffuse   = idTex;
    texWall.specular  = idTex;
    texWall.emissive  = idTex; 
    texWall.normal    = 0;
    texWall.shininess = 10.0;

    // Configurar textura Suelo
    texFloor.diffuse   = idTex;
    texFloor.specular  = idTex;
    texFloor.emissive  = idTex;
    texFloor.normal    = 0;
    texFloor.shininess = 10.0;

    // --- LUCES ---
    // Global
    lightG.ambient = glm::vec3(0.5, 0.5, 0.5);

    // Directional
    lightD[0].direction = glm::vec3(-1.0, -1.0, 0.0);
    lightD[0].ambient   = glm::vec3( 0.1, 0.1, 0.1);
    lightD[0].diffuse   = glm::vec3( 0.7, 0.7, 0.7);
    lightD[0].specular  = glm::vec3( 0.7, 0.7, 0.7);

    // Positional
    lightP[0].position = glm::vec3(0.0, 3.0, 3.0);
    lightP[0].ambient  = glm::vec3(0.2, 0.2, 0.2);
    lightP[0].diffuse  = glm::vec3(0.9, 0.9, 0.9);
    lightP[0].specular = glm::vec3(0.9, 0.9, 0.9);
    lightP[0].c0 = 1.00;
    lightP[0].c1 = 0.22;
    lightP[0].c2 = 0.20;

    // Focal (Linterna)
    lightF[0].position    = glm::vec3(0.0, 0.0, 0.0);
    lightF[0].direction   = glm::vec3(0.0, 0.0, -1.0);
    lightF[0].ambient     = glm::vec3(0.2, 0.2, 0.2);
    lightF[0].diffuse     = glm::vec3(0.9, 0.9, 0.9);
    lightF[0].specular    = glm::vec3(0.9, 0.9, 0.9);
    lightF[0].innerCutOff = 10.0;
    lightF[0].outerCutOff = 15.0;
    lightF[0].c0 = 1.0;
    lightF[0].c1 = 0.09;
    lightF[0].c2 = 0.032;
    
    lightF[1] = lightF[0]; 

    // Material
    mluz.ambient   = glm::vec4(0.0, 0.0, 0.0, 1.0);
    mluz.diffuse   = glm::vec4(0.0, 0.0, 0.0, 1.0);
    mluz.specular  = glm::vec4(0.0, 0.0, 0.0, 1.0);
    mluz.emissive  = glm::vec4(1.0, 1.0, 1.0, 1.0);
    mluz.shininess = 1.0;
}

// -------------------------------------------------------------------------------------
// BUCLE DE RENDERIZADO
// -------------------------------------------------------------------------------------
void renderScene(){

    // Limpiar buffers
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shaders.useShaders();

    // 1. Matriz PROYECCION y VISTA
    float aspect = (float)w/(float)h;
    glm::mat4 P = glm::perspective(glm::radians(60.0f), aspect, 0.1f, 100.0f);
    glm::mat4 V = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

    shaders.setVec3("ueye", cameraPos);
    setLights(P, V);

    // 2. DIBUJAR SUELO
    glm::mat4 MSuelo = glm::translate(glm::mat4(1.0), glm::vec3(40.0f, 0.0f, 40.0f)); 
    MSuelo = glm::scale(MSuelo, glm::vec3(80.0f, 1.0f, 80.0f)); 
    drawObjectTex(planeModel, texFloor, P, V, MSuelo);

    // 3. DIBUJAR LABERINTO
    float bSize = 4.0f; // Tamaño visual del bloque
    float bH    = 5.0f; // Altura visual de pared

    // Escala para modelos de tamaño 2.0
    float scX = bSize / 2.0f;
    float scY = bH    / 2.0f;
    float scZ = bSize / 2.0f;

    for(int z = 0; z < mapLevel.size(); z++) {
        for(int x = 0; x < mapLevel[z].size(); x++) {
            if(mapLevel[z][x] == '1') {
                glm::vec3 pos(x * bSize, bH / 2.0f, z * bSize);
                glm::mat4 M = glm::translate(glm::mat4(1.0), pos);
                M = glm::scale(M, glm::vec3(scX, scY, scZ));
                drawObjectTex(cubeModel, texWall, P, V, M);
            }
        }
    }
}

// -------------------------------------------------------------------------------------
// SISTEMA DE COLISIONES
// -------------------------------------------------------------------------------------

// Función auxiliar para detectar choque con paredes
bool checkCollision(glm::vec3 newPos) {
    float blockSize = 4.0f; 
    float offset    = blockSize / 2.0f; 
    
    // Aumentamos un poco el radio para asegurar que la cámara (near plane)
    // nunca llegue a cortar la textura. 0.4 es un buen valor.
    float playerRadius = 0.4f; 

    // 1. Calculamos los límites de la "Caja" del jugador en el mundo
    float minX = newPos.x - playerRadius;
    float maxX = newPos.x + playerRadius;
    float minZ = newPos.z - playerRadius;
    float maxZ = newPos.z + playerRadius;

    // 2. Convertimos esos límites a índices del mapa (Grid)
    // Esto nos dice: "¿Desde qué casilla X hasta qué casilla X ocupo?"
    int startGridX = (int)((minX + offset) / blockSize);
    int endGridX   = (int)((maxX + offset) / blockSize);
    int startGridZ = (int)((minZ + offset) / blockSize);
    int endGridZ   = (int)((maxZ + offset) / blockSize);

    // 3. Recorremos TODAS las casillas que toca tu cuerpo
    // (Normalmente serán 1, 2 o 4 casillas como máximo)
    for (int z = startGridZ; z <= endGridZ; z++) {
        for (int x = startGridX; x <= endGridX; x++) {
            
            // Protección de límites del array
            if (z < 0 || z >= mapLevel.size() || 
                x < 0 || x >= mapLevel[z].size()) {
                return true; // Choca con el vacío exterior
            }

            // Si CUALQUIERA de las casillas que tocas es pared...
            if (mapLevel[z][x] == '1') {
                return true; // ...¡Es choque!
            }
        }
    }

    return false;
}



// -------------------------------------------------------------------------------------
// INPUT Y MOVIMIENTO
// -------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    float cameraSpeed = 0.1f; 
    
    // Vectores de dirección
    glm::vec3 frontXZ = glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z));
    glm::vec3 rightXZ = glm::normalize(glm::cross(cameraFront, cameraUp));

    // Variable temporal para donde QUEREMOS ir
    glm::vec3 nextPos = cameraPos;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        nextPos += cameraSpeed * frontXZ;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        nextPos -= cameraSpeed * frontXZ;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        nextPos -= cameraSpeed * rightXZ;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        nextPos += cameraSpeed * rightXZ;
    
    // COLISIONES: Chequear X y Z por separado para permitir deslizarse
    // Solo actualizamos si NO hay colisión
    if (!checkCollision(glm::vec3(nextPos.x, cameraPos.y, cameraPos.z))) {
        cameraPos.x = nextPos.x;
    }
    if (!checkCollision(glm::vec3(cameraPos.x, cameraPos.y, nextPos.z))) {
        cameraPos.z = nextPos.z;
    }

    cameraPos.y = 2.0f; // Altura fija
}

void funCursorPos(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    if(pitch > 89.0f) pitch = 89.0f;
    if(pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void funKey(GLFWwindow* window, int key , int scancode, int action, int mods) {
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// -------------------------------------------------------------------------------------
// FUNCIONES AUXILIARES (LUCES, DIBUJO, ETC)
// -------------------------------------------------------------------------------------

void setLights(glm::mat4 P, glm::mat4 V) {
    shaders.setLight("ulightG",lightG);
    for(int i=0; i<NLD; i++) shaders.setLight("ulightD["+toString(i)+"]",lightD[i]);
    for(int i=0; i<NLP; i++) {
        Light light = lightP[i];
        light.position = glm::vec3(V * glm::vec4(light.position, 1.0));
        shaders.setLight("ulightP["+toString(i)+"]",light);
    }
    for(int i=0; i<NLF; i++) {
        Light light = lightF[i];
        light.position = glm::vec3(V * glm::vec4(light.position, 1.0));
        light.direction = glm::vec3(V * glm::vec4(light.direction, 0.0));
        shaders.setLight("ulightF["+toString(i)+"]",light);
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
    glViewport(0, 0, width, height);
    w = width;
    h = height;
}

void funScroll(GLFWwindow* window, double xoffset, double yoffset) {
    // Zoom si quisieras
}