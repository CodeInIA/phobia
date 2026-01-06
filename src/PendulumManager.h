#ifndef PENDULUM_MANAGER_H
#define PENDULUM_MANAGER_H

#include <vector>
#include <string>
#include <glm/glm.hpp>

// Individual pendulum data
struct Pendulum {
    glm::ivec2 gridPos;        // Grid position in map
    float swingAngle;          // Current swing angle in degrees
    float swingVelocity;       // Angular velocity
    float phase;               // Phase offset for variation
    float rotation;            // Y-axis rotation (0 or 90 degrees for perpendicular to corridor)

    Pendulum()
        : gridPos(0, 0)
        , swingAngle(0.0f)
        , swingVelocity(0.0f)
        , phase(0.0f)
        , rotation(0.0f)
    {}
};

class PendulumManager {
public:
    PendulumManager() = default;
    ~PendulumManager() = default;

    // Clear all pendulums
    void clear();

    // Add a pendulum at grid position
    void addPendulum(glm::ivec2 gridPos);

    // Calculate pendulum orientations based on adjacent walls
    void calculateOrientations(const std::vector<std::string>& mapLevel);

    // Update pendulum physics (oscillation)
    void update(float deltaTime);

    // Get all pendulums (for rendering)
    const std::vector<Pendulum>& getPendulums() const { return m_pendulums; }

    // Check if position collides with any pendulum blade
    bool checkCollision(glm::vec3 position, float playerRadius) const;

    // Get pendulum count
    size_t getPendulumCount() const { return m_pendulums.size(); }

private:
    std::vector<Pendulum> m_pendulums;
    
    static constexpr float BLOCK_SIZE = 4.0f;
    static constexpr float BLOCK_HEIGHT = 5.0f;
    static constexpr float MAX_SWING_ANGLE = 45.0f;  // Maximum swing angle (degrees)
    static constexpr float GRAVITY = 9.81f;
    static constexpr float PENDULUM_LENGTH = 2.5f;   // Approximate length from pivot to blade center
    static constexpr float BLADE_RADIUS = 1.0f;      // Collision radius for blade
};

#endif // PENDULUM_MANAGER_H
