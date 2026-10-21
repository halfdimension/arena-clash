#pragma once

#include "raylib.h"

#include <memory>

namespace arena {

struct GameState;

class GameRenderer {
public:
    GameRenderer();
    ~GameRenderer();

    GameRenderer(const GameRenderer&) = delete;
    GameRenderer& operator=(const GameRenderer&) = delete;

    void draw(const GameState& state, const char* platformName, int framesPerSecond,
              bool drawDebugHitboxes);

private:
    struct AnimationResources;

    RenderTexture2D sceneTarget_{};
    std::unique_ptr<AnimationResources> animationResources_;
};

}  // namespace arena
