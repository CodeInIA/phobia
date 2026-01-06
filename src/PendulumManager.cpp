#include "PendulumManager.h"
#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

void PendulumManager::clear() {
    m_pendulums.clear();
}

void PendulumManager::addPendulum(glm::ivec2 gridPos) {
    Pendulum pendulum;
    pendulum.gridPos = gridPos;
    
    // Start at maximum angle for immediate visibility
    pendulum.phase = 0.0f;
    pendulum.swingAngle = MAX_SWING_ANGLE;
    pendulum.swingVelocity = 0.0f;
    pendulum.rotation = 0.0f;  // Will be calculated later
    
    m_pendulums.push_back(pendulum);
    
    std::cout << "Added pendulum at grid (" << gridPos.x << ", " << gridPos.y << ")" << std::endl;
}

void PendulumManager::calculateOrientations(const std::vector<std::string>& mapLevel) {
    for (auto& pendulum : m_pendulums) {
        int x = pendulum.gridPos.x;
        int y = pendulum.gridPos.y;
        
        // Check adjacent cells for walls
        bool wallLeft = false, wallRight = false, wallUp = false, wallDown = false;
        
        if (x > 0 && mapLevel[y][x-1] == '1') wallLeft = true;
        if (x < mapLevel[y].size() - 1 && mapLevel[y][x+1] == '1') wallRight = true;
        if (y > 0 && mapLevel[y-1][x] == '1') wallUp = true;
        if (y < mapLevel.size() - 1 && mapLevel[y+1][x] == '1') wallDown = true;
        
        // Determine orientation based on walls
        // If corridor runs North-South (walls on left and right), pendulum swings East-West (rotation = 90)
        // If corridor runs East-West (walls up and down), pendulum swings North-South (rotation = 0)
        
        if (wallLeft && wallRight) {
            // Vertical corridor (N-S), swing perpendicular (E-W)
            pendulum.rotation = 90.0f;
        } else if (wallUp && wallDown) {
            // Horizontal corridor (E-W), swing perpendicular (N-S)
            pendulum.rotation = 0.0f;
        } else {
            // Default: assume vertical corridor
            pendulum.rotation = 90.0f;
        }
        
        std::cout << "Pendulum at (" << x << ", " << y << ") rotation: " << pendulum.rotation << "°" << std::endl;
    }
}

void PendulumManager::update(float deltaTime) {
    // Use simple harmonic motion with constant angular velocity for smooth oscillation
    const float angularFrequency = 1.5f;  // Radians per second (adjust for speed)
    
    for (auto& pendulum : m_pendulums) {
        // Update phase based on time
        pendulum.phase += angularFrequency * deltaTime;
        
        // Calculate swing angle using sine wave for constant speed oscillation
        pendulum.swingAngle = MAX_SWING_ANGLE * std::sin(pendulum.phase);
    }
}

bool PendulumManager::checkCollision(glm::vec3 position, float playerRadius) const {
    for (const auto& pendulum : m_pendulums) {
        // Calculate pendulum world position (same as rendering)
        glm::vec3 pivotPos;
        pivotPos.x = pendulum.gridPos.x * BLOCK_SIZE;
        pivotPos.y = BLOCK_HEIGHT;  // At ceiling
        pivotPos.z = pendulum.gridPos.y * BLOCK_SIZE;

        float modelTopY = 3.384f;
        
        // Apply all transformations to get the blade position (matching rendering exactly)
        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, pivotPos);
        transform = glm::rotate(transform, glm::radians(90.0f + pendulum.rotation), glm::vec3(0.0f, 1.0f, 0.0f));
        transform = glm::translate(transform, glm::vec3(0.0f, -modelTopY, 0.0f));
        
        // Apply swing transformations (matching rendering)
        transform = glm::translate(transform, glm::vec3(-0.15f, modelTopY, 0.0f));
        transform = glm::scale(transform, glm::vec3(1.0f, 1.15f, 1.0f));
        transform = glm::rotate(transform, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        transform = glm::rotate(transform, glm::radians(pendulum.swingAngle), glm::vec3(0.0f, 0.0f, 1.0f));
        
        // Check collision along the entire blade length with more precision
        // The blade extends from the pivot down, check more points for accuracy
        for (float t = 0.2f; t <= 3.0f; t += 0.3f) {
            glm::vec4 bladePointLocal = glm::vec4(0.0f, -t, 0.0f, 1.0f);
            glm::vec4 bladePointWorld = transform * bladePointLocal;
            
            glm::vec3 bladePos = glm::vec3(bladePointWorld);
            
            // Check collision with player (cylinder collision in XZ plane)
            float distanceXZ = glm::length(glm::vec2(position.x - bladePos.x, position.z - bladePos.z));
            
            // Variable blade width depending on position along the blade
            // Thicker near the blade (t > 1.5), thinner at the pole
            float effectiveRadius = (t > 1.5f) ? BLADE_RADIUS * 1.2f : BLADE_RADIUS * 0.8f;
            
            // Check if player is at the right height and within collision distance
            // Use a tighter vertical range for more accurate collision
            if (std::abs(position.y - bladePos.y) < 1.5f) {
                if (distanceXZ < (playerRadius + effectiveRadius)) {
                    return true;  // Collision detected
                }
            }
        }
    }
    
    return false;  // No collision
}
