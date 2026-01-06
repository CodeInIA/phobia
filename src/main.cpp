/**
 * Phobia - Horror Game
 * 
 * Refactored modular architecture:
 * - Game: Application lifecycle and main loop
 * - InputManager: Keyboard and mouse input handling  
 * - ResourceManager: Centralized resource loading (models, textures, shaders)
 * - Scene: Game world management (map, doors, camera, rendering)
 * - Camera: Player view and movement
 * - DoorManager: Door state and animation
 */

#include "Game.h"
#include <iostream>

int main() {
    Game game;
    
    if (!game.init(1000, 1000, "Phobia")) {
        std::cerr << "Failed to initialize game" << std::endl;
        return -1;
    }
    
    game.run();
    
    return 0;
}