#include "SpiderwebManager.h"
#include <iostream>

void SpiderwebManager::clear() {
    m_spiderwebs.clear();
}

bool SpiderwebManager::isWall(const std::vector<std::string>& mapLevel, int x, int z) const {
    if (z < 0 || z >= static_cast<int>(mapLevel.size())) return true;
    if (x < 0 || x >= static_cast<int>(mapLevel[z].size())) return true;
    return mapLevel[z][x] == '1';
}

bool SpiderwebManager::isConcaveCorner(const std::vector<std::string>& mapLevel, int x, int z, float& outRotation) const {
    // A concave corner is where we have an empty space with walls forming an internal corner
    // We check the 8 surrounding positions to detect the corner pattern
    
    // Current position must be empty (not a wall)
    if (isWall(mapLevel, x, z)) {
        return false;
    }
    
    // Check the 4 cardinal directions and 4 diagonal directions
    bool N  = isWall(mapLevel, x, z - 1);  // North
    bool S  = isWall(mapLevel, x, z + 1);  // South
    bool E  = isWall(mapLevel, x + 1, z);  // East
    bool W  = isWall(mapLevel, x - 1, z);  // West
    bool NE = isWall(mapLevel, x + 1, z - 1);  // Northeast
    bool NW = isWall(mapLevel, x - 1, z - 1);  // Northwest
    bool SE = isWall(mapLevel, x + 1, z + 1);  // Southeast
    bool SW = isWall(mapLevel, x - 1, z + 1);  // Southwest
    
    // Concave corner patterns (interior corners):
    // Each pattern has walls on two adjacent cardinal directions
    
    // Northwest concave corner: walls to the North AND West, but open to NE or SW diagonal
    if (N && W && !E && !S) {
        outRotation = 180.0f;  // Face southeast (into the corner)
        return true;
    }
    
    // Northeast concave corner: walls to the North AND East, but open to NW or SE diagonal
    if (N && E && !W && !S) {
        outRotation = 90.0f;  // Adjusted to match NW orientation
        return true;
    }
    
    // Southwest concave corner: walls to the South AND West, but open to SE or NW diagonal
    if (S && W && !E && !N) {
        outRotation = 270.0f;  // Adjusted to match NW orientation
        return true;
    }
    
    // Southeast concave corner: walls to the South AND East, but open to SW or NE diagonal
    if (S && E && !W && !N) {
        outRotation = 0.0f;  // Face northwest (into the corner)
        return true;
    }
    
    return false;
}

void SpiderwebManager::detectAndAddSpiderwebs(const std::vector<std::string>& mapLevel) {
    clear();
    
    // Scan the entire map for concave corners
    for (int z = 0; z < static_cast<int>(mapLevel.size()); z++) {
        for (int x = 0; x < static_cast<int>(mapLevel[z].size()); x++) {
            float rotation;
            if (isConcaveCorner(mapLevel, x, z, rotation)) {
                Spiderweb web;
                web.gridPos = glm::ivec2(x, z);
                web.rotation = rotation;
                m_spiderwebs.push_back(web);
                
                std::cout << "Spiderweb added at grid(" << x << ", " << z 
                          << ") with rotation " << rotation << std::endl;
            }
        }
    }
    
    std::cout << "Total spiderwebs placed: " << m_spiderwebs.size() << std::endl;
}
