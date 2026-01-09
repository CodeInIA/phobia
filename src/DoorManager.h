#ifndef DOOR_MANAGER_H
#define DOOR_MANAGER_H

#include <vector>
#include <string>
#include <glm/glm.hpp>

// Door types available in the game
enum class DoorType {
    NORMAL,     // Door03 - bloody hand door
    FIRE_EXIT   // Door04 - fire exit door
};

// Individual door data
struct Door {
    glm::ivec2 gridPos;     // Grid position in map
    float rotation;         // Y-axis rotation (0 = Z-oriented, 90 = X-oriented)
    bool isOpen;            // Current state
    float openAngle;        // Animation angle (0 = closed, -90 = open)
    DoorType type;          // Door type

    Door()
        : gridPos(0, 0)
        , rotation(0.0f)
        , isOpen(false)
        , openAngle(0.0f)
        , type(DoorType::NORMAL)
    {}
};

class DoorManager {
public:
    DoorManager() = default;
    ~DoorManager() = default;

    // Clear all doors
    void clear();

    // Add a door at grid position with type
    void addDoor(glm::ivec2 gridPos, DoorType type);

    // Determine door orientations based on adjacent walls
    void calculateOrientations(const std::vector<std::string>& mapLevel);

    // Update door animations
    void update(float deltaTime);

    // Find nearest door within maxDistance from world position
    // Returns nullptr if no door in range
    Door* findNearestDoor(glm::vec3 worldPos, float maxDistance);

    // Toggle the nearest door to the given position
    // Returns pointer to toggled door, or nullptr if none
    Door* toggleNearestDoor(glm::vec3 worldPos, float maxDistance);

    // Get all doors (for rendering)
    const std::vector<Door>& getDoors() const { return m_doors; }
    std::vector<Door>& getDoors() { return m_doors; }

    // Check if position collides with any closed door
    bool checkCollision(glm::vec3 position, float playerRadius) const;

    // Get door count
    size_t getDoorCount() const { return m_doors.size(); }

private:
    std::vector<Door> m_doors;
    
    static constexpr float BLOCK_SIZE = 4.0f;
    static constexpr float OPEN_SPEED = 90.0f;     // Degrees per second
    static constexpr float MAX_OPEN_ANGLE = -90.0f; // Fully open
};

#endif // DOOR_MANAGER_H
