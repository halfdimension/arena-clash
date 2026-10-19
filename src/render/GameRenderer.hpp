#pragma once

#include "raylib.h"

namespace arena {

struct GameState;

class GameRenderer {
public:
    GameRenderer();
    ~GameRenderer();

    GameRenderer(const GameRenderer&) = delete;
    GameRenderer& operator=(const GameRenderer&) = delete;

    void draw(const GameState& state, const char* platformName, int framesPerSecond) const;

private:
    RenderTexture2D sceneTarget_{};
};

}  // namespace arena
