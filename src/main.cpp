#include "core/Game.hpp"
#include "input/KeyboardInput.hpp"
#include "render/GameRenderer.hpp"

#include "raylib.h"

#include <algorithm>

#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#endif

namespace {

constexpr double kMaximumFrameDeltaSeconds = 0.25;

#if defined(__EMSCRIPTEN__)
constexpr const char* kPlatformName = "Web (Emscripten)";
#elif defined(_WIN32)
constexpr const char* kPlatformName = "Windows desktop";
#elif defined(__APPLE__)
constexpr const char* kPlatformName = "macOS desktop";
#elif defined(__linux__)
constexpr const char* kPlatformName = "Linux desktop";
#else
constexpr const char* kPlatformName = "Unknown platform";
#endif

struct ApplicationContext {
    arena::Game game;
    arena::KeyboardInput input;
    arena::GameRenderer renderer;
    double accumulatorSeconds = 0.0;
    bool firstFramePending = true;
    bool drawDebugHitboxes = false;
};

void RunFrame(ApplicationContext& application) {
    application.input.sample();
    if (application.input.consumeDebugHitboxToggle()) {
        application.drawDebugHitboxes = !application.drawDebugHitboxes;
    }

    const double frameDeltaSeconds = std::clamp(
        static_cast<double>(GetFrameTime()), 0.0, kMaximumFrameDeltaSeconds);
    application.accumulatorSeconds += frameDeltaSeconds;

    // A stalled render frame may catch up, but the delta clamp bounds the work to 15 ticks.
    while (application.accumulatorSeconds >= arena::kFixedDeltaSeconds) {
        application.game.advanceSimulation(application.input.consumeForSimulationTick());
        application.accumulatorSeconds -= arena::kFixedDeltaSeconds;
    }

    application.renderer.draw(application.game.state(), kPlatformName, GetFPS(),
                              application.drawDebugHitboxes);
    if (application.firstFramePending) {
        TraceLog(LOG_INFO, "APPLICATION: First frame rendered successfully");
        application.firstFramePending = false;
    }
}

#if defined(__EMSCRIPTEN__)
void BrowserMainLoop(void* userData) {
    auto* application = static_cast<ApplicationContext*>(userData);
    RunFrame(*application);
}
#endif

}  // namespace

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(arena::kLogicalWidth, arena::kLogicalHeight, "ARENA CLASH");
    if (!IsWindowReady()) {
        TraceLog(LOG_ERROR, "APPLICATION: Window and graphics context initialization failed");
        return 1;
    }
    SetWindowMinSize(640, 360);

#if !defined(__EMSCRIPTEN__)
    SetTargetFPS(120);
#endif

#if defined(__EMSCRIPTEN__)
    // The browser owns scheduling and application lifetime after this registration.
    auto* application = new ApplicationContext{};
    application->game.initialize();
    TraceLog(LOG_INFO, "APPLICATION: Registering requestAnimationFrame main loop");
    emscripten_set_main_loop_arg(BrowserMainLoop, application, 0, true);
#else
    {
        ApplicationContext application;
        application.game.initialize();
        while (!WindowShouldClose()) {
            RunFrame(application);
        }
    }
    CloseWindow();
#endif

    return 0;
}
