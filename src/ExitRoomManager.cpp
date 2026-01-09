#include "ExitRoomManager.h"
#include <iostream>
#include <cmath>

void ExitRoomManager::clear() {
    m_exitRooms.clear();
}

void ExitRoomManager::addExitRoom(glm::ivec2 gridPos) {
    ExitRoom room;
    room.gridPos = gridPos;
    m_exitRooms.push_back(room);
}

void ExitRoomManager::findExitDoors(const std::vector<std::string>& mapLevel) {
    // For each exit room, find the adjacent 'E' (exit door)
    for (auto& room : m_exitRooms) {
        int x = room.gridPos.x;
        int z = room.gridPos.y;
        
        // Check all 4 adjacent positions for 'E'
        const int dx[] = {0, 0, -1, 1};
        const int dz[] = {-1, 1, 0, 0};
        
        bool found = false;
        for (int i = 0; i < 4; i++) {
            int nx = x + dx[i];
            int nz = z + dz[i];
            
            if (nz >= 0 && nz < static_cast<int>(mapLevel.size()) &&
                nx >= 0 && nx < static_cast<int>(mapLevel[nz].size())) {
                if (mapLevel[nz][nx] == 'E') {
                    room.exitDoorPos = glm::ivec2(nx, nz);
                    found = true;
                    std::cout << "Exit room at (" << x << "," << z << ") has exit door at (" 
                              << nx << "," << nz << ")" << std::endl;
                    break;
                }
            }
        }
        
        if (!found) {
            std::cout << "Warning: Exit room at (" << x << "," << z 
                      << ") has no adjacent exit door!" << std::endl;
        }
    }
}

bool ExitRoomManager::checkCollision(glm::vec3 position, float playerRadius) const {
    float halfBlock = BLOCK_SIZE / 2.0f;
    
    for (const auto& room : m_exitRooms) {
        // Exit room world center
        glm::vec3 roomCenter(room.gridPos.x * BLOCK_SIZE, 0.0f, room.gridPos.y * BLOCK_SIZE);
        
        // Check collision with each of the 5 walls (all except the one facing the exit door)
        // We need to check 6 faces of the cube, but skip the face that connects to the exit door
        
        // Determine which face to skip based on exit door direction
        int doorDX = room.exitDoorPos.x - room.gridPos.x;
        int doorDZ = room.exitDoorPos.y - room.gridPos.y;
        
        // Check each face of the cube
        // The cube is centered at roomCenter with size BLOCK_SIZE
        
        // -X face (left wall)
        if (doorDX != -1) {
            float faceX = roomCenter.x - halfBlock;
            if (std::abs(position.x - faceX) < playerRadius &&
                position.z > roomCenter.z - halfBlock && position.z < roomCenter.z + halfBlock) {
                return true;
            }
        }
        
        // +X face (right wall)
        if (doorDX != 1) {
            float faceX = roomCenter.x + halfBlock;
            if (std::abs(position.x - faceX) < playerRadius &&
                position.z > roomCenter.z - halfBlock && position.z < roomCenter.z + halfBlock) {
                return true;
            }
        }
        
        // -Z face (front wall)
        if (doorDZ != -1) {
            float faceZ = roomCenter.z - halfBlock;
            if (std::abs(position.z - faceZ) < playerRadius &&
                position.x > roomCenter.x - halfBlock && position.x < roomCenter.x + halfBlock) {
                return true;
            }
        }
        
        // +Z face (back wall)
        if (doorDZ != 1) {
            float faceZ = roomCenter.z + halfBlock;
            if (std::abs(position.z - faceZ) < playerRadius &&
                position.x > roomCenter.x - halfBlock && position.x < roomCenter.x + halfBlock) {
                return true;
            }
        }
        
        // Floor and ceiling are always present, but we don't need to check them for player collision
        // (player walks on the floor)
    }
    
    return false;
}
