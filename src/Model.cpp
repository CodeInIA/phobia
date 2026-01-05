#include "Model.h"

//-----------------------------------------------------------------------------------------------------
// Lee los atributos del modelo de un fichero de texto y los almacena en los vectores correspondientes
//-----------------------------------------------------------------------------------------------------
void Model::initModel(const char *modelFile) {
   
 // Importa el modelo mediante la librería Assimp
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(modelFile,  
        aiProcess_Triangulate |     
        aiProcess_JoinIdenticalVertices | 
        aiProcess_PreTransformVertices  |
        aiProcess_GenSmoothNormals | 
        aiProcess_CalcTangentSpace | 
        aiProcess_GenUVCoords);
    if(!scene) {
        std::cout << "El fichero " << modelFile << " no se puede abrir." << std::endl;
        std::cin.get();
        exit(1);
    }
  
 // Procesar todas las meshes de la escena
    for(unsigned int i = 0; i < scene->mNumMeshes; i++) {
        processMesh(scene, scene->mMeshes[i]);
    }
    
}

//--------------------------------
// Procesa una mesh individual
//--------------------------------
void Model::processMesh(const aiScene *scene, aiMesh *mesh) {
    Mesh newMesh;
    
    // Extraer vértices, normales y coordenadas de textura
    for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
        newMesh.positions.push_back(glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z));
        newMesh.normals.push_back(glm::normalize(glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z)));
        if(mesh->mTextureCoords[0]) 
            newMesh.textureCoords.push_back(glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y));
        else 
            newMesh.textureCoords.push_back(glm::vec2(0.0f, 0.0f));
    }
    
    // Extraer índices
    for(unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++) 
            newMesh.indices.push_back(face.mIndices[j]);
    }
    
    // Crear VAO y buffers
    glGenVertexArrays(1, &newMesh.vao);
    glGenBuffers(1, &newMesh.vboPositions);
    glGenBuffers(1, &newMesh.vboNormals);
    glGenBuffers(1, &newMesh.vboTextureCoords);
    glGenBuffers(1, &newMesh.eboIndices);
    
    glBindVertexArray(newMesh.vao);
    
    // Posiciones
    glBindBuffer(GL_ARRAY_BUFFER, newMesh.vboPositions);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * newMesh.positions.size(), &(newMesh.positions.front()), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    
    // Normales
    glBindBuffer(GL_ARRAY_BUFFER, newMesh.vboNormals);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * newMesh.normals.size(), &(newMesh.normals.front()), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
    
    // Coordenadas de textura
    glBindBuffer(GL_ARRAY_BUFFER, newMesh.vboTextureCoords);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * newMesh.textureCoords.size(), &(newMesh.textureCoords.front()), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);
    
    // Índices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, newMesh.eboIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * newMesh.indices.size(), &(newMesh.indices.front()), GL_STATIC_DRAW);
    
    glBindVertexArray(0);
    
    meshes.push_back(newMesh);
}

//--------------------------------
// Renderiza el VAO con el modelo
//--------------------------------
void Model::renderModel(unsigned long mode) {
    glPolygonMode(GL_FRONT_AND_BACK, mode);
    for(auto& mesh : meshes) {
        glBindVertexArray(mesh.vao);
        glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, (void *)0);
        glBindVertexArray(0);
    }
}

//--------------------------------
// Renderiza un mesh específico
//--------------------------------
void Model::renderMesh(unsigned long mode, int meshIndex) {
    if(meshIndex >= 0 && meshIndex < meshes.size()) {
        glPolygonMode(GL_FRONT_AND_BACK, mode);
        glBindVertexArray(meshes[meshIndex].vao);
        glDrawElements(GL_TRIANGLES, meshes[meshIndex].indices.size(), GL_UNSIGNED_INT, (void *)0);
        glBindVertexArray(0);
    }
}

//--------------------------------
// Obtiene el número de meshes
//--------------------------------
int Model::getMeshCount() {
    return meshes.size();
}

//-----------------------------------
// Destructor de la clase
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
