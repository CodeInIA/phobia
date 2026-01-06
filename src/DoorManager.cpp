#include "DoorManager.h"
#include <iostream>
#include <cmath>

void DoorManager::clear() {
    m_doors.clear();
}

void DoorManager::addDoor(glm::ivec2 gridPos, DoorType type) {
    Door door;
    door.gridPos = gridPos;
    door.isOpen = false;
    door.openAngle = 0.0f;
    door.rotation = 0.0f;  // Will be calculated later
    door.type = type;
    m_doors.push_back(door);

    std::cout << "Door added at grid(" << gridPos.x << ", " << gridPos.y << ") - Type: "
              << (type == DoorType::NORMAL ? "Normal" : "Fire Exit") << std::endl;
}

void DoorManager::calculateOrientations(const std::vector<std::string>& mapLevel) {
    for (auto& door : m_doors) {
        int x = door.gridPos.x;
        int z = door.gridPos.y;

        // Check adjacent walls
        bool wallLeft  = (x > 0 && mapLevel[z][x - 1] == '1');
        bool wallRight = (x < static_cast<int>(mapLevel[z].size()) - 1 && mapLevel[z][x + 1] == '1');
        bool wallUp    = (z > 0 && mapLevel[z - 1][x] == '1');
        bool wallDown  = (z < static_cast<int>(mapLevel.size()) - 1 && mapLevel[z + 1][x] == '1');

        // If walls above/below, door oriented horizontally (X axis)
        if (wallUp || wallDown) {
            door.rotation = 90.0f;
        }
        // If walls left/right, door oriented vertically (Z axis)
        else if (wallLeft || wallRight) {
            door.rotation = 0.0f;
        }

        std::cout << "Door at (" << x << ", " << z << ") orientation: " 
                  << door.rotation << " degrees" << std::endl;
    }
}

void DoorManager::update(float deltaTime) {
    for (auto& door : m_doors) {
        if (door.isOpen && door.openAngle > MAX_OPEN_ANGLE) {
            // Opening
            door.openAngle -= OPEN_SPEED * deltaTime;
            if (door.openAngle < MAX_OPEN_ANGLE) {
                door.openAngle = MAX_OPEN_ANGLE;
            }
        } else if (!door.isOpen && door.openAngle < 0.0f) {
            // Closing
            door.openAngle += OPEN_SPEED * deltaTime;
            if (door.openAngle > 0.0f) {
                door.openAngle = 0.0f;
            }
        }
    }
}

Door* DoorManager::findNearestDoor(glm::vec3 worldPos, float maxDistance) {
    Door* nearestDoor = nullptr;
    float minDistance = maxDistance;

    for (auto& door : m_doors) {
        float doorCenterX = door.gridPos.x * BLOCK_SIZE;
        float doorCenterZ = door.gridPos.y * BLOCK_SIZE;

        float dist = glm::distance(worldPos, glm::vec3(doorCenterX, worldPos.y, doorCenterZ));

        if (dist < minDistance) {
            minDistance = dist;
            nearestDoor = &door;
        }
    }

    return nearestDoor;
}

bool DoorManager::toggleNearestDoor(glm::vec3 worldPos, float maxDistance) {
    Door* door = findNearestDoor(worldPos, maxDistance);
    if (door) {
        door->isOpen = !door->isOpen;
        std::cout << "Door " << (door->isOpen ? "opened" : "closed") << std::endl;
        return true;
    }
    return false;
}

bool DoorManager::checkCollision(glm::vec3 position, float playerRadius) const {
    float halfBlock = BLOCK_SIZE / 2.0f;

    for (const auto& door : m_doors) {
        if (!door.isOpen) {  // Only collide with closed doors
            float doorCenterX = door.gridPos.x * BLOCK_SIZE;
            float doorCenterZ = door.gridPos.y * BLOCK_SIZE;

            float distX = std::abs(position.x - doorCenterX);
            float distZ = std::abs(position.z - doorCenterZ);

            if (distX < halfBlock + playerRadius && distZ < halfBlock + playerRadius) {
                return true;
            }
        }
    }

    return false;
}
