#include "Model.h"
#include <cfloat>  // For FLT_MAX

//-----------------------------------------------------------------------------------------------------
// Helper function to process a single mesh and create its VAO
//-----------------------------------------------------------------------------------------------------
void Model::processMesh(aiMesh *mesh, unsigned int materialIndex, const std::string& materialName) {
    
    std::vector<glm::vec3>      positions;
    std::vector<glm::vec3>      normals;
    std::vector<glm::vec2>      textureCoords;
    std::vector<unsigned int>   indices;
    
    // Extract vertex attributes
    for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
        positions.push_back(glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z));
        normals.push_back(glm::normalize(glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z)));
        if(mesh->mTextureCoords[0]) 
            textureCoords.push_back(glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y));
        else                        
            textureCoords.push_back(glm::vec2(0.0f, 0.0f));
    }
    
    // Extract indices
    for(unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++) 
            indices.push_back(face.mIndices[j]);
    }
    
    // Create MeshData and VAO
    MeshData meshData;
    meshData.indexCount = indices.size();
    meshData.materialIndex = materialIndex;
    meshData.materialName = materialName;
    meshData.useUnsignedInt = true;  // Using GL_UNSIGNED_INT for large models
    
    glGenVertexArrays(1, &meshData.vao);
    glGenBuffers(1, &meshData.vboPositions);
    glGenBuffers(1, &meshData.vboNormals);
    glGenBuffers(1, &meshData.vboTextureCoords);
    glGenBuffers(1, &meshData.eboIndices);
    
    glBindVertexArray(meshData.vao);
        // Positions
        glBindBuffer(GL_ARRAY_BUFFER, meshData.vboPositions);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * positions.size(), &(positions.front()), GL_STATIC_DRAW);  
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); 
        glEnableVertexAttribArray(0);
        // Normals   
        glBindBuffer(GL_ARRAY_BUFFER, meshData.vboNormals);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * normals.size(), &(normals.front()), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0); 
        glEnableVertexAttribArray(1);
        // Texture coords
        glBindBuffer(GL_ARRAY_BUFFER, meshData.vboTextureCoords);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * textureCoords.size(), &(textureCoords.front()), GL_STATIC_DRAW);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0); 
        glEnableVertexAttribArray(2);
        // Indices
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshData.eboIndices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), &(indices.front()), GL_STATIC_DRAW);
    glBindVertexArray(0);
    
    meshes.push_back(meshData);
}

//-----------------------------------------------------------------------------------------------------
// Loads all meshes from a model file using Assimp
//-----------------------------------------------------------------------------------------------------
void Model::initModel(const char *modelFile) {
   
    // Import model using Assimp
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(modelFile,  
        aiProcess_Triangulate |     
        aiProcess_GenSmoothNormals | 
        aiProcess_CalcTangentSpace | 
        aiProcess_GenUVCoords |
        aiProcess_SortByPType);  // Removed PreTransformVertices and JoinIdenticalVertices to preserve separate meshes
    
    if(!scene) {
        std::cout << "El fichero " << modelFile << " no se puede abrir." << std::endl;
        std::cin.get();
        exit(1);
    }
    
    // Initialize bounds to extreme values
    boundsMin = glm::vec3(FLT_MAX);
    boundsMax = glm::vec3(-FLT_MAX);
    
    // Process all meshes in the scene
    for(unsigned int m = 0; m < scene->mNumMeshes; m++) {
        aiMesh *mesh = scene->mMeshes[m];
        
        // Get mesh name (OBJ files use 'usemtl' which becomes mesh name)
        std::string matName = mesh->mName.C_Str();
        
        // If mesh name is empty, try material name
        if(matName.empty() && mesh->mMaterialIndex < scene->mNumMaterials) {
            aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
            aiString name;
            if(mat->Get(AI_MATKEY_NAME, name) == AI_SUCCESS) {
                matName = name.C_Str();
            }
        }
        
        if(matName.empty()) matName = "unknown";
        
        // Update bounding box
        for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
            glm::vec3 pos(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
            boundsMin = glm::min(boundsMin, pos);
            boundsMax = glm::max(boundsMax, pos);
        }
        
        processMesh(mesh, mesh->mMaterialIndex, matName);
        
        std::cout << "  Mesh " << m << ": name='" << matName << "' (material index " << mesh->mMaterialIndex << ")" << std::endl;
    }
    
    glm::vec3 size = getSize();
    glm::vec3 center = getCenter();
    std::cout << "Loaded model: " << modelFile << " with " << meshes.size() << " mesh(es)" << std::endl;
    std::cout << "  Bounds: min(" << boundsMin.x << ", " << boundsMin.y << ", " << boundsMin.z << ")" << std::endl;
    std::cout << "          max(" << boundsMax.x << ", " << boundsMax.y << ", " << boundsMax.z << ")" << std::endl;
    std::cout << "  Size: (" << size.x << ", " << size.y << ", " << size.z << ")" << std::endl;
    std::cout << "  Center: (" << center.x << ", " << center.y << ", " << center.z << ")" << std::endl;
}

//--------------------------------
// Render all meshes in the model
//--------------------------------
void Model::renderModel(unsigned long mode) {
    
    glPolygonMode(GL_FRONT_AND_BACK, mode);
    for(const auto& mesh : meshes) {
        glBindVertexArray(mesh.vao);
        GLenum indexType = mesh.useUnsignedInt ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;
        glDrawElements(GL_TRIANGLES, mesh.indexCount, indexType, (void *)0);
    }
    glBindVertexArray(0);
}

//--------------------------------
// Render a specific mesh by index
//--------------------------------
void Model::renderMesh(unsigned int meshIndex, unsigned long mode) {
    
    if(meshIndex >= meshes.size()) return;
    
    glPolygonMode(GL_FRONT_AND_BACK, mode);
    glBindVertexArray(meshes[meshIndex].vao);
    GLenum indexType = meshes[meshIndex].useUnsignedInt ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;
    glDrawElements(GL_TRIANGLES, meshes[meshIndex].indexCount, indexType, (void *)0);
    glBindVertexArray(0);
}

//--------------------------------
// Get number of meshes
//--------------------------------
unsigned int Model::getMeshCount() const {
    return meshes.size();
}

//--------------------------------
// Get material index for a mesh
//--------------------------------
unsigned int Model::getMeshMaterialIndex(unsigned int meshIndex) const {
    if(meshIndex >= meshes.size()) return 0;
    return meshes[meshIndex].materialIndex;
}

//--------------------------------
// Get material name for a mesh
//--------------------------------
std::string Model::getMeshMaterialName(unsigned int meshIndex) const {
    if(meshIndex >= meshes.size()) return "";
    return meshes[meshIndex].materialName;
}

//-----------------------------------
// Destructor - cleanup all meshes
//-----------------------------------
Model::~Model() {
    for(auto& mesh : meshes) {
        glDeleteVertexArrays(1, &mesh.vao);
        glDeleteBuffers(1, &mesh.vboPositions);
        glDeleteBuffers(1, &mesh.vboNormals);
        glDeleteBuffers(1, &mesh.vboTextureCoords);
        glDeleteBuffers(1, &mesh.eboIndices);
    }
}
