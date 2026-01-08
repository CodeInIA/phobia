#ifndef TORCH_MANAGER_H
#define TORCH_MANAGER_H

#include <vector>
#include <string>
#include <glm/glm.hpp>

// Individual torch data
struct Torch {
    glm::ivec2 gridPos;     // Grid position in map
    float rotation;         // Rotation angle (0, 90, 180, 270) - facing direction
    float flickerOffset;    // Random offset for independent flicker

    Torch()
        : gridPos(0, 0)
        , rotation(0.0f)
        , flickerOffset(0.0f)
    {}
};

class TorchManager {
public:
    TorchManager() = default;
    ~TorchManager() = default;

    // Clear all torches
    void clear();

    // Add a torch at grid position
    void addTorch(glm::ivec2 gridPos);

    // Calculate orientations based on adjacent walls
    void calculateOrientations(const std::vector<std::string>& mapLevel);

    // Get all torches (for rendering)
    const std::vector<Torch>& getTorches() const { return m_torches; }

    // Get torch count
    size_t getTorchCount() const { return m_torches.size(); }

    // Calculate flicker intensity for a torch at given time
    float getFlickerIntensity(size_t torchIndex, float time) const;

private:
    std::vector<Torch> m_torches;
};

#endif // TORCH_MANAGER_H
