#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <string>
#include <map>
#include "Model.h"
#include "Texture.h"
#include "Shaders.h"

// Texture group struct for shader uniforms
struct TextureGroup {
    unsigned int diffuse   = 0;
    unsigned int specular  = 0;
    unsigned int emissive  = 0;
    unsigned int normal    = 0;
    float        shininess = 32.0f;
};

class ResourceManager {
public:
    ResourceManager() = default;
    ~ResourceManager() = default;

    // Prevent copying
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    // Initialize all resources
    void loadAllResources();

    // Model access
    Model& getModel(const std::string& name);
    const Model& getModel(const std::string& name) const;
    bool hasModel(const std::string& name) const;

    // Texture access (raw textures)
    Texture& getTexture(const std::string& name);
    const Texture& getTexture(const std::string& name) const;
    bool hasTexture(const std::string& name) const;

    // Texture group access (for shaders)
    Textures& getTextureGroup(const std::string& name);
    const Textures& getTextureGroup(const std::string& name) const;
    bool hasTextureGroup(const std::string& name) const;

    // Shader access
    Shaders& getShader();
    const Shaders& getShader() const;

private:
    void loadModels();
    void loadTextures();
    void setupTextureGroups();
    void loadShaders();

    std::map<std::string, Model>    m_models;
    std::map<std::string, Texture>  m_textures;
    std::map<std::string, Textures> m_textureGroups;
    Shaders m_shaders;
};

#endif // RESOURCE_MANAGER_H
