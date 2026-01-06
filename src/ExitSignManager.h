#ifndef EXIT_SIGN_MANAGER_H
#define EXIT_SIGN_MANAGER_H

#include <vector>
#include <glm/glm.hpp>

// Individual exit sign data
struct ExitSign {
    glm::ivec2 gridPos;     // Grid position in map

    ExitSign()
        : gridPos(0, 0)
    {}
};

class ExitSignManager {
public:
    ExitSignManager() = default;
    ~ExitSignManager() = default;

    // Clear all exit signs
    void clear();

    // Add an exit sign at grid position
    void addExitSign(glm::ivec2 gridPos);

    // Get all exit signs (for rendering)
    const std::vector<ExitSign>& getExitSigns() const { return m_exitSigns; }

private:
    std::vector<ExitSign> m_exitSigns;
};

#endif // EXIT_SIGN_MANAGER_H
