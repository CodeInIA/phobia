#ifndef MODEL_H
#define MODEL_H

#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define I glm::mat4(1.0)

// Per-mesh data structure for multi-mesh support
struct MeshData {
    unsigned int vao;
    unsigned int vboPositions;
    unsigned int vboNormals;
    unsigned int vboTextureCoords;
    unsigned int eboIndices;
    unsigned int indexCount;      // Number of indices for this mesh
    unsigned int materialIndex;   // Material index from Assimp
    std::string  materialName;    // Material name from Assimp
    bool useUnsignedInt;          // True if using GL_UNSIGNED_INT indices, false for GL_UNSIGNED_SHORT
};

class Model {
    
    public:
                        
        void initModel  (const char *modelFile);
        void renderModel(unsigned long mode);                    // Render all meshes
        void renderMesh (unsigned int meshIndex, unsigned long mode);  // Render specific mesh
        
        unsigned int getMeshCount() const;                       // Get number of meshes
        unsigned int getMeshMaterialIndex(unsigned int meshIndex) const; // Get material index for mesh
        std::string  getMeshMaterialName(unsigned int meshIndex) const;  // Get material name for mesh
        
        // Bounding box getters
        glm::vec3 getBoundsMin() const { return boundsMin; }
        glm::vec3 getBoundsMax() const { return boundsMax; }
        glm::vec3 getSize() const { return boundsMax - boundsMin; }
        glm::vec3 getCenter() const { return (boundsMin + boundsMax) * 0.5f; }
               
        virtual ~Model();
               
    private:
        
        std::vector<MeshData> meshes;  // Vector of mesh data for multi-mesh support
        glm::vec3 boundsMin;           // Minimum bounds
        glm::vec3 boundsMax;           // Maximum bounds
        
        void processMesh(aiMesh *mesh, unsigned int materialIndex, const std::string& materialName);  // Helper to process single mesh

};

#endif /* MODEL_H */
