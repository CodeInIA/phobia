#include "ExitSignManager.h"

void ExitSignManager::clear() {
    m_exitSigns.clear();
}

void ExitSignManager::addExitSign(glm::ivec2 gridPos) {
    ExitSign sign;
    sign.gridPos = gridPos;
    m_exitSigns.push_back(sign);
}
