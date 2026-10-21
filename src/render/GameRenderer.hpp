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

    void draw(const GameState& state, const char* platformName, int framesPerSecond,
              bool drawDebugHitboxes) const;

private:
    RenderTexture2D sceneTarget_{};
    Texture2D voltNeutralTexture_{};
    Texture2D emberNeutralTexture_{};
};

}  // namespace arena
