#include "TorchManager.h"
#include <cmath>
#include <cstdlib>

void TorchManager::clear() {
    m_torches.clear();
}

void TorchManager::addTorch(glm::ivec2 gridPos) {
    Torch torch;
    torch.gridPos = gridPos;
    torch.rotation = 0.0f;
    // Random flicker offset for independent flickering (0 to 2*PI)
    torch.flickerOffset = static_cast<float>(m_torches.size()) * 1.7f;
    m_torches.push_back(torch);
}

void TorchManager::calculateOrientations(const std::vector<std::string>& mapLevel) {
    int mapHeight = static_cast<int>(mapLevel.size());
    
    for (auto& torch : m_torches) {
        int x = torch.gridPos.x;
        int z = torch.gridPos.y;
        
        int mapWidth = (z >= 0 && z < mapHeight) ? static_cast<int>(mapLevel[z].size()) : 0;
        
        // Check adjacent cells for walls (1 = wall)
        bool wallNorth = (z > 0 && x < static_cast<int>(mapLevel[z-1].size()) && mapLevel[z-1][x] == '1');
        bool wallSouth = (z < mapHeight-1 && x < static_cast<int>(mapLevel[z+1].size()) && mapLevel[z+1][x] == '1');
        bool wallEast  = (x < mapWidth-1 && mapLevel[z][x+1] == '1');
        bool wallWest  = (x > 0 && mapLevel[z][x-1] == '1');
        
        // Face torch away from the wall it's mounted on
        // Priority: North wall > South wall > East wall > West wall
        if (wallNorth) {
            torch.rotation = 180.0f;  // Mounted on north wall, face south
        } else if (wallSouth) {
            torch.rotation = 0.0f;    // Mounted on south wall, face north
        } else if (wallEast) {
            torch.rotation = 270.0f;  // Mounted on east wall, face west
        } else if (wallWest) {
            torch.rotation = 90.0f;   // Mounted on west wall, face east
        } else {
            // No adjacent wall found, default facing
            torch.rotation = 0.0f;
        }
    }
}

float TorchManager::getFlickerIntensity(size_t torchIndex, float time) const {
    if (torchIndex >= m_torches.size()) return 1.0f;
    
    const Torch& torch = m_torches[torchIndex];
    float offset = torch.flickerOffset;
    
    // Multi-frequency flicker for organic fire effect
    // Primary slow wave
    float flicker1 = std::sin(time * 2.5f + offset) * 0.08f;
    // Secondary faster wave
    float flicker2 = std::sin(time * 4.1f + offset * 1.3f) * 0.05f;
    // Tertiary high-frequency shimmer
    float flicker3 = std::sin(time * 7.3f + offset * 0.7f) * 0.03f;
    
    // Base intensity 0.88, with total variation of ±0.16
    return 0.88f + flicker1 + flicker2 + flicker3;
}
