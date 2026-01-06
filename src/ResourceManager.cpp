#include "ResourceManager.h"
#include <iostream>
#include <stdexcept>

void ResourceManager::loadAllResources() {
    loadModels();
    loadTextures();
    setupTextureGroups();
    loadShaders();
}

void ResourceManager::loadModels() {
    // Basic geometry
    m_models["cube"].initModel("resources/models/cube.obj");
    m_models["plane"].initModel("resources/models/plane.obj");
    m_models["flashlight"].initModel("resources/models/flashlight.obj");
    
    // Door03 (Normal/Bloody hand door)
    m_models["doorNormalFrame"].initModel("resources/models/doors/normal_door/Door03_Frame.obj");
    m_models["doorNormalDoor"].initModel("resources/models/doors/normal_door/Door03_Door.obj");
    m_models["doorNormalGlass"].initModel("resources/models/doors/normal_door/Door03_Glass.obj");
    
    // Door04 (Fire exit door)
    m_models["doorExitFrame"].initModel("resources/models/doors/fire_exit/Door04_Frame.obj");
    m_models["doorExitDoor"].initModel("resources/models/doors/fire_exit/Door04_Door.obj");
    m_models["doorExitExtra1"].initModel("resources/models/doors/fire_exit/Door04_Extra01.obj");
    m_models["doorExitExtra2"].initModel("resources/models/doors/fire_exit/Door04_Extra02.obj");
    
    std::cout << "ResourceManager: Loaded " << m_models.size() << " models" << std::endl;
}

void ResourceManager::loadTextures() {
    // Wall textures
    m_textures["wallDiffuse"].initTexture("resources/textures/wall/wall_diffuse.png");
    m_textures["wallNormal"].initTexture("resources/textures/wall/wall_normal.jpg");
    m_textures["wallSpecular"].initTexture("resources/textures/wall/wall_specular.jpg");
    m_textures["wallEmissive"].initTexture("resources/textures/wall/wall_emissive.jpg");

    // Floor textures (reusing wall textures as in original)
    m_textures["floorDiffuse"].initTexture("resources/textures/wall/wall_diffuse.png");
    m_textures["floorSpecular"].initTexture("resources/textures/wall/wall_specular.jpg");
    m_textures["floorNormal"].initTexture("resources/textures/wall/wall_normal.jpg");
    m_textures["floorEmissive"].initTexture("resources/textures/wall/wall_emissive.jpg");

    // Ceiling textures (reusing wall textures as in original)
    m_textures["ceilingDiffuse"].initTexture("resources/textures/wall/wall_diffuse.png");
    m_textures["ceilingSpecular"].initTexture("resources/textures/wall/wall_specular.jpg");
    m_textures["ceilingNormal"].initTexture("resources/textures/wall/wall_normal.jpg");
    m_textures["ceilingEmissive"].initTexture("resources/textures/wall/wall_emissive.jpg");

    // Flashlight textures
    m_textures["flashlightDiffuse"].initTexture("resources/textures/flashlight/DefaultMaterial.png");
    m_textures["flashlightSpecular"].initTexture("resources/textures/flashlight/metalnessMap1.png");
    m_textures["flashlightNormal"].initTexture("resources/textures/flashlight/normalMap1.png");
    m_textures["flashlightEmissive"].initTexture("resources/textures/flashlight/emissiveMap1.png");

    // Door textures (DoorPack atlas - shared by all doors)
    m_textures["doorDiffuse"].initTexture("resources/textures/doors/DoorPack_Base.jpeg");
    m_textures["doorSpecular"].initTexture("resources/textures/doors/DoorPack_Metallic.jpeg");
    m_textures["doorNormal"].initTexture("resources/textures/doors/DoorPack_Normal.png");
    m_textures["doorGlassDiffuse"].initTexture("resources/textures/doors/DoorPack_Opacity.jpeg");

    std::cout << "ResourceManager: Loaded " << m_textures.size() << " textures" << std::endl;
}

void ResourceManager::setupTextureGroups() {
    // Wall texture group
    Textures& texWall = m_textureGroups["wall"];
    texWall.diffuse   = m_textures["wallDiffuse"].getTexture();
    texWall.specular  = m_textures["wallSpecular"].getTexture();
    texWall.emissive  = m_textures["wallEmissive"].getTexture();
    texWall.normal    = m_textures["wallNormal"].getTexture();
    texWall.shininess = 64.0f;

    // Floor texture group
    Textures& texFloor = m_textureGroups["floor"];
    texFloor.diffuse   = m_textures["floorDiffuse"].getTexture();
    texFloor.specular  = m_textures["floorSpecular"].getTexture();
    texFloor.emissive  = m_textures["floorEmissive"].getTexture();
    texFloor.normal    = m_textures["floorNormal"].getTexture();
    texFloor.shininess = 64.0f;

    // Ceiling texture group
    Textures& texCeiling = m_textureGroups["ceiling"];
    texCeiling.diffuse   = m_textures["ceilingDiffuse"].getTexture();
    texCeiling.specular  = m_textures["ceilingSpecular"].getTexture();
    texCeiling.emissive  = m_textures["ceilingEmissive"].getTexture();
    texCeiling.normal    = m_textures["ceilingNormal"].getTexture();
    texCeiling.shininess = 64.0f;

    // Flashlight texture group
    Textures& texFlashlight = m_textureGroups["flashlight"];
    texFlashlight.diffuse   = m_textures["flashlightDiffuse"].getTexture();
    texFlashlight.specular  = m_textures["flashlightSpecular"].getTexture();
    texFlashlight.emissive  = m_textures["flashlightEmissive"].getTexture();
    texFlashlight.normal    = m_textures["flashlightNormal"].getTexture();
    texFlashlight.shininess = 32.0f;

    // Door frame texture group
    Textures& texDoorFrame = m_textureGroups["doorFrame"];
    texDoorFrame.diffuse   = m_textures["doorDiffuse"].getTexture();
    texDoorFrame.specular  = m_textures["doorSpecular"].getTexture();
    texDoorFrame.emissive  = 0;
    texDoorFrame.normal    = m_textures["doorNormal"].getTexture();
    texDoorFrame.shininess = 32.0f;

    // Door glass texture group (for transparency)
    Textures& texDoorGlass = m_textureGroups["doorGlass"];
    texDoorGlass.diffuse   = m_textures["doorGlassDiffuse"].getTexture();
    texDoorGlass.specular  = m_textures["doorSpecular"].getTexture();
    texDoorGlass.emissive  = 0;
    texDoorGlass.normal    = 0;
    texDoorGlass.shininess = 96.0f;

    std::cout << "ResourceManager: Created " << m_textureGroups.size() << " texture groups" << std::endl;
}

void ResourceManager::loadShaders() {
    m_shaders.initShaders("resources/shaders/vshader.glsl", "resources/shaders/fshader.glsl");
    std::cout << "ResourceManager: Shaders loaded" << std::endl;
}

// Model access
Model& ResourceManager::getModel(const std::string& name) {
    auto it = m_models.find(name);
    if (it == m_models.end()) {
        throw std::runtime_error("Model not found: " + name);
    }
    return it->second;
}

const Model& ResourceManager::getModel(const std::string& name) const {
    auto it = m_models.find(name);
    if (it == m_models.end()) {
        throw std::runtime_error("Model not found: " + name);
    }
    return it->second;
}

bool ResourceManager::hasModel(const std::string& name) const {
    return m_models.find(name) != m_models.end();
}

// Texture access
Texture& ResourceManager::getTexture(const std::string& name) {
    auto it = m_textures.find(name);
    if (it == m_textures.end()) {
        throw std::runtime_error("Texture not found: " + name);
    }
    return it->second;
}

const Texture& ResourceManager::getTexture(const std::string& name) const {
    auto it = m_textures.find(name);
    if (it == m_textures.end()) {
        throw std::runtime_error("Texture not found: " + name);
    }
    return it->second;
}

bool ResourceManager::hasTexture(const std::string& name) const {
    return m_textures.find(name) != m_textures.end();
}

// Texture group access
Textures& ResourceManager::getTextureGroup(const std::string& name) {
    auto it = m_textureGroups.find(name);
    if (it == m_textureGroups.end()) {
        throw std::runtime_error("Texture group not found: " + name);
    }
    return it->second;
}

const Textures& ResourceManager::getTextureGroup(const std::string& name) const {
    auto it = m_textureGroups.find(name);
    if (it == m_textureGroups.end()) {
        throw std::runtime_error("Texture group not found: " + name);
    }
    return it->second;
}

bool ResourceManager::hasTextureGroup(const std::string& name) const {
    return m_textureGroups.find(name) != m_textureGroups.end();
}

// Shader access
Shaders& ResourceManager::getShader() {
    return m_shaders;
}

const Shaders& ResourceManager::getShader() const {
    return m_shaders;
}
