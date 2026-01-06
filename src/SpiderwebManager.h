#ifndef SPIDERWEB_MANAGER_H
#define SPIDERWEB_MANAGER_H

#include <vector>
#include <string>
#include <glm/glm.hpp>

// Individual spiderweb data
struct Spiderweb {
    glm::ivec2 gridPos;     // Grid position in map (corner position)
    float rotation;         // Y-axis rotation to align with corner orientation

    Spiderweb()
        : gridPos(0, 0)
        , rotation(0.0f)
    {}
};

class SpiderwebManager {
public:
    SpiderwebManager() = default;
    ~SpiderwebManager() = default;

    // Clear all spiderwebs
    void clear();

    // Detect and add spiderwebs at all concave corners of the maze
    void detectAndAddSpiderwebs(const std::vector<std::string>& mapLevel);

    // Get all spiderwebs (for rendering)
    const std::vector<Spiderweb>& getSpiderwebs() const { return m_spiderwebs; }

    // Get spiderweb count
    size_t getSpiderwebCount() const { return m_spiderwebs.size(); }

private:
    std::vector<Spiderweb> m_spiderwebs;
    
    // Helper to check if a grid position is a wall
    bool isWall(const std::vector<std::string>& mapLevel, int x, int z) const;
    
    // Helper to detect if a position is a concave corner and determine its orientation
    bool isConcaveCorner(const std::vector<std::string>& mapLevel, int x, int z, float& outRotation) const;
    
    static constexpr float BLOCK_SIZE = 4.0f;
};

#endif // SPIDERWEB_MANAGER_H
