#include "render/GameRenderer.hpp"

#include "core/Game.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace arena {
namespace {

constexpr Color kInk = {12, 14, 25, 255};
constexpr Color kMist = {109, 124, 160, 255};
constexpr Color kVolt = {70, 220, 255, 255};
constexpr Color kVoltGlow = {108, 109, 255, 255};
constexpr Color kEmber = {255, 92, 56, 255};
constexpr Color kEmberGlow = {255, 190, 67, 255};

void DrawCenteredText(const char* text, int centerX, int y, int fontSize, Color color) {
    DrawText(text, centerX - MeasureText(text, fontSize) / 2, y, fontSize, color);
}

void DrawArenaBackdrop(std::uint64_t simulationFrame) {
    DrawRectangleGradientV(0, 0, kLogicalWidth, kLogicalHeight,
                           Color{9, 12, 30, 255}, Color{35, 19, 46, 255});

    const float pulse = 0.5F + 0.5F * std::sin(static_cast<float>(simulationFrame) * 0.025F);
    DrawCircleGradient(Vector2{640.0F, 270.0F}, 255.0F + pulse * 8.0F,
                       Color{75, 52, 133, 90}, Color{15, 12, 35, 0});

    for (int i = 0; i < 18; ++i) {
        const int x = 34 + i * 74;
        const int height = 65 + (i % 5) * 24;
        DrawRectangle(x, 420 - height, 48, height, Color{17, 20, 39, 255});
        DrawRectangle(x + 8, 432 - height, 4, 18, Color{77, 72, 126, 120});
        DrawRectangle(x + 27, 448 - height, 5, 13, Color{212, 90, 109, 85});
    }

    DrawRectangleGradientV(0, 420, kLogicalWidth, 220,
                           Color{36, 29, 49, 255}, Color{13, 15, 25, 255});
    DrawLineEx(Vector2{0.0F, 420.0F}, Vector2{1280.0F, 420.0F}, 3.0F,
               Color{135, 105, 187, 130});

    for (int x = 80; x < kLogicalWidth; x += 160) {
        DrawLine(x, 420, 640, 626, Color{86, 65, 112, 75});
    }
    for (int y = 452; y < 640; y += 42) {
        DrawLine(0, y, kLogicalWidth, y, Color{85, 67, 105, 75});
    }

    DrawRectangle(0, 634, kLogicalWidth, 86, Color{8, 10, 18, 255});
    DrawRectangleGradientH(170, 629, 940, 5,
                           Color{45, 212, 255, 90}, Color{255, 104, 64, 90});
    DrawRectangle(430, 629, 420, 5, Color{184, 115, 238, 145});

    DrawCircleLines(640, 286, 95.0F, Color{139, 110, 204, 75});
    DrawCircleLines(640, 286, 69.0F, Color{139, 110, 204, 45});
    DrawLine(640, 186, 640, 386, Color{139, 110, 204, 45});
    DrawLine(540, 286, 740, 286, Color{139, 110, 204, 45});
}

void DrawFighter(const Fighter& fighter) {
    const bool isVolt = fighter.id == FighterId::Volt;
    Color primary = isVolt ? kVolt : kEmber;
    const Color secondary = isVolt ? kVoltGlow : kEmberGlow;
    const float x = fighter.position.x;
    const float floorY = fighter.position.y - 12.0F;
    const float facing = fighter.facing == FacingDirection::Right ? 1.0F : -1.0F;
    if (fighter.hitFlashFrames > 0) {
        primary = RAYWHITE;
    }

    Vector2 rearHand = {x - facing * 46.0F, floorY - 66.0F};
    Vector2 frontHand = {x + facing * 48.0F, floorY - 90.0F};
    if (fighter.combatState == CombatState::Blocking) {
        rearHand = {x + facing * 39.0F, floorY - 111.0F};
        frontHand = {x + facing * 44.0F, floorY - 78.0F};
    } else if (fighter.combatState == CombatState::Attacking) {
        const bool heavy = fighter.currentMove == MoveId::Heavy;
        if (fighter.attackPhase == AttackPhase::Startup) {
            frontHand = {x - facing * (heavy ? 34.0F : 18.0F),
                         floorY - (heavy ? 124.0F : 108.0F)};
        } else if (fighter.attackPhase == AttackPhase::Active) {
            frontHand = {x + facing * (heavy ? 105.0F : 82.0F),
                         floorY - (heavy ? 75.0F : 98.0F)};
            rearHand = {x + facing * (heavy ? 61.0F : 26.0F),
                        floorY - (heavy ? 54.0F : 72.0F)};
        } else {
            frontHand = {x + facing * 33.0F, floorY - 61.0F};
        }
    }

    DrawEllipse(static_cast<int>(x), static_cast<int>(floorY + 22.0F), 72.0F, 15.0F,
                Color{0, 0, 0, 115});
    DrawCircleGradient(Vector2{x, floorY - 80.0F}, 78.0F,
                       Color{primary.r, primary.g, primary.b, 45}, Color{primary.r, primary.g, primary.b, 0});

    DrawLineEx(Vector2{x - 16.0F, floorY - 47.0F}, Vector2{x - 27.0F, floorY + 12.0F},
               17.0F, kInk);
    DrawLineEx(Vector2{x + 15.0F, floorY - 47.0F}, Vector2{x + 29.0F, floorY + 12.0F},
               17.0F, kInk);
    DrawLineEx(Vector2{x - facing * 12.0F, floorY - 115.0F},
               rearHand, 15.0F, kInk);
    DrawLineEx(Vector2{x + facing * 12.0F, floorY - 112.0F},
               frontHand, fighter.currentMove == MoveId::Heavy ? 20.0F : 16.0F, primary);

    DrawRectangleRounded(Rectangle{x - 29.0F, floorY - 128.0F, 58.0F, 88.0F},
                         0.25F, 5, kInk);
    DrawRectangleRoundedLinesEx(Rectangle{x - 29.0F, floorY - 128.0F, 58.0F, 88.0F},
                                0.25F, 5, 4.0F, primary);
    DrawCircle(static_cast<int>(x), static_cast<int>(floorY - 153.0F), 28.0F, kInk);
    DrawCircleLines(static_cast<int>(x), static_cast<int>(floorY - 153.0F), 29.0F, primary);

    const float chestX = x + facing * 2.0F;
    if (isVolt) {
        DrawTriangle(Vector2{chestX - 7.0F, floorY - 109.0F},
                     Vector2{chestX + 10.0F, floorY - 109.0F},
                     Vector2{chestX - 2.0F, floorY - 82.0F}, secondary);
        DrawTriangle(Vector2{chestX - 2.0F, floorY - 91.0F},
                     Vector2{chestX + 13.0F, floorY - 91.0F},
                     Vector2{chestX - 2.0F, floorY - 67.0F}, secondary);
    } else {
        DrawCircleGradient(Vector2{chestX, floorY - 91.0F}, 14.0F,
                           kEmberGlow, Color{255, 75, 40, 0});
        DrawTriangle(Vector2{chestX, floorY - 113.0F},
                     Vector2{chestX - 15.0F, floorY - 83.0F},
                     Vector2{chestX + 15.0F, floorY - 83.0F}, kEmber);
    }

    if (fighter.combatState == CombatState::Blocking) {
        DrawCircleLines(static_cast<int>(x + facing * 52.0F),
                        static_cast<int>(floorY - 93.0F), 42.0F,
                        fighter.blockFlashFrames > 0 ? RAYWHITE : secondary);
    }
    if (fighter.combatState == CombatState::Attacking &&
        fighter.attackPhase == AttackPhase::Active) {
        DrawCircleLines(static_cast<int>(frontHand.x), static_cast<int>(frontHand.y),
                        fighter.currentMove == MoveId::Heavy ? 28.0F : 18.0F, secondary);
    }
    if (fighter.hitFlashFrames > 0) {
        const float effectRadius = 18.0F + static_cast<float>(fighter.hitFlashFrames) * 4.0F;
        DrawCircleLines(static_cast<int>(x), static_cast<int>(floorY - 92.0F),
                        effectRadius, RAYWHITE);
        DrawLineEx(Vector2{x - 30.0F, floorY - 122.0F},
                   Vector2{x + 32.0F, floorY - 62.0F}, 4.0F, secondary);
    }
    if (fighter.combatState == CombatState::KnockedOut) {
        DrawCenteredText("KO", static_cast<int>(x), static_cast<int>(floorY - 205.0F),
                         30, Color{255, 234, 150, 255});
    }
}

void DrawDebugBoxes(const GameState& state) {
    for (const Fighter& fighter : state.fighters) {
        const Rectanglef hurtbox = GetHurtbox(fighter);
        DrawRectangleLinesEx(Rectangle{hurtbox.x, hurtbox.y, hurtbox.width, hurtbox.height},
                             2.0F, Color{82, 255, 130, 220});
        if (fighter.combatState == CombatState::Attacking &&
            fighter.attackPhase == AttackPhase::Active) {
            const Rectanglef hitbox =
                GetAttackHitbox(fighter, GetMoveDefinition(fighter.currentMove));
            DrawRectangleLinesEx(Rectangle{hitbox.x, hitbox.y, hitbox.width, hitbox.height},
                                 3.0F, Color{255, 77, 96, 235});
        }
    }
}

void DrawHealthBar(const Fighter& fighter, int x, bool alignRight) {
    constexpr int kBarWidth = 438;
    constexpr int kBarHeight = 24;
    const float healthRatio = std::clamp(
        static_cast<float>(fighter.health) / static_cast<float>(fighter.maximumHealth), 0.0F, 1.0F);
    const int fillWidth = static_cast<int>((kBarWidth - 8) * healthRatio);
    const Color accent = fighter.id == FighterId::Volt ? kVolt : kEmber;
    const char* name = fighter.id == FighterId::Volt ? "VOLT" : "EMBER";

    DrawText(name, alignRight ? x + kBarWidth - MeasureText(name, 28) : x, 33, 28, RAYWHITE);
    DrawRectangleRounded(Rectangle{static_cast<float>(x), 72.0F, static_cast<float>(kBarWidth),
                                   static_cast<float>(kBarHeight)}, 0.35F, 6, Color{5, 8, 18, 230});
    const int fillX = alignRight ? x + kBarWidth - 4 - fillWidth : x + 4;
    DrawRectangle(fillX, 76, fillWidth, kBarHeight - 8, accent);
    DrawRectangleLinesEx(Rectangle{static_cast<float>(x), 72.0F, static_cast<float>(kBarWidth),
                                   static_cast<float>(kBarHeight)}, 2.0F, Color{182, 190, 218, 175});

    char healthText[16];
    std::snprintf(healthText, sizeof(healthText), "%d / %d", fighter.health,
                  fighter.maximumHealth);
    const int textX = alignRight ? x + kBarWidth - MeasureText(healthText, 15) : x;
    DrawText(healthText, textX, 101, 15, Color{216, 220, 236, 220});
}

void DrawHud(const GameState& state, const char* platformName, int framesPerSecond) {
    DrawHealthBar(state.fighters[0], 64, false);
    DrawHealthBar(state.fighters[1], 778, true);

    DrawCircle(640, 69, 46.0F, Color{8, 10, 22, 245});
    DrawCircleLines(640, 69, 46.0F, Color{185, 151, 236, 190});
    DrawCenteredText("ROUND 1", 640, 41, 18, Color{222, 213, 240, 255});
    DrawCenteredText("99", 640, 62, 30, RAYWHITE);

    DrawCenteredText("VOLT: A/D MOVE  W JUMP  S BLOCK  F LIGHT  G HEAVY",
                     kLogicalWidth / 2, 646, 15, Color{204, 210, 231, 220});
    DrawCenteredText("EMBER: ARROWS MOVE/JUMP  DOWN BLOCK  K LIGHT  L HEAVY  |  F1 BOXES",
                     kLogicalWidth / 2, 664, 14, Color{182, 190, 218, 210});

    if (state.fighters[0].combatState == CombatState::KnockedOut ||
        state.fighters[1].combatState == CombatState::KnockedOut) {
        DrawCenteredText("K.O.", kLogicalWidth / 2, 135, 72, Color{255, 232, 145, 245});
    }

    char debugLine[128];
    std::snprintf(debugLine, sizeof(debugLine), "60 Hz SIM  |  TICK %llu  |  %d FPS",
                  static_cast<unsigned long long>(state.simulationFrame), framesPerSecond);
    DrawText(debugLine, 20, 697, 14, kMist);

    const int platformWidth = MeasureText(platformName, 15);
    DrawText(platformName, kLogicalWidth - platformWidth - 20, 697, 15, kMist);
}

}  // namespace

GameRenderer::GameRenderer()
    : sceneTarget_(LoadRenderTexture(kLogicalWidth, kLogicalHeight)) {
    if (!IsRenderTextureValid(sceneTarget_)) {
        TraceLog(LOG_ERROR, "RENDERER: Logical render texture initialization failed");
        return;
    }
    SetTextureFilter(sceneTarget_.texture, TEXTURE_FILTER_BILINEAR);
    TraceLog(LOG_INFO, "RENDERER: Logical render texture initialized (%dx%d)",
             kLogicalWidth, kLogicalHeight);
}

GameRenderer::~GameRenderer() {
    UnloadRenderTexture(sceneTarget_);
}

void GameRenderer::draw(const GameState& state, const char* platformName, int framesPerSecond,
                        bool drawDebugHitboxes) const {
    BeginTextureMode(sceneTarget_);
    ClearBackground(kInk);
    DrawArenaBackdrop(state.simulationFrame);
    DrawFighter(state.fighters[0]);
    DrawFighter(state.fighters[1]);
    if (drawDebugHitboxes) {
        DrawDebugBoxes(state);
    }
    DrawHud(state, platformName, framesPerSecond);
    EndTextureMode();

    const float scale = std::min(static_cast<float>(GetScreenWidth()) / kLogicalWidth,
                                 static_cast<float>(GetScreenHeight()) / kLogicalHeight);
    const float drawWidth = kLogicalWidth * scale;
    const float drawHeight = kLogicalHeight * scale;
    const Rectangle destination = {
        (GetScreenWidth() - drawWidth) * 0.5F,
        (GetScreenHeight() - drawHeight) * 0.5F,
        drawWidth,
        drawHeight,
    };

    BeginDrawing();
    ClearBackground(Color{4, 5, 10, 255});
    DrawTexturePro(sceneTarget_.texture,
                   Rectangle{0.0F, 0.0F, static_cast<float>(kLogicalWidth),
                             -static_cast<float>(kLogicalHeight)},
                   destination, Vector2{0.0F, 0.0F}, 0.0F, WHITE);
    EndDrawing();
}

}  // namespace arena
