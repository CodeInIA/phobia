#ifndef EXIT_ROOM_MANAGER_H
#define EXIT_ROOM_MANAGER_H

#include <vector>
#include <string>
#include <glm/glm.hpp>

// Individual exit room data
struct ExitRoom {
    glm::ivec2 gridPos;     // Grid position in map (where 'L' was)
    glm::ivec2 exitDoorPos; // Grid position of the adjacent 'E' exit door

    ExitRoom()
        : gridPos(0, 0)
        , exitDoorPos(0, 0)
    {}
};

class ExitRoomManager {
public:
    ExitRoomManager() = default;
    ~ExitRoomManager() = default;

    // Clear all exit rooms
    void clear();

    // Add an exit room at grid position
    void addExitRoom(glm::ivec2 gridPos);

    // Find adjacent exit door position for each room
    void findExitDoors(const std::vector<std::string>& mapLevel);

    // Get all exit rooms (for rendering)
    const std::vector<ExitRoom>& getExitRooms() const { return m_exitRooms; }

    // Check if position collides with any exit room wall
    bool checkCollision(glm::vec3 position, float playerRadius) const;

    // Get exit room count
    size_t getExitRoomCount() const { return m_exitRooms.size(); }

private:
    std::vector<ExitRoom> m_exitRooms;
    
    static constexpr float BLOCK_SIZE = 4.0f;
    static constexpr float BLOCK_HEIGHT = 5.0f;
};

#endif // EXIT_ROOM_MANAGER_H
