/*
 *  ______        _                _
 * |  ___ \      | |              | |
 * | | _ | | ____| |_   _ ____  _ | | ___
 * | || || |/ _  | | | | / _  |/ || |/ _ \
 * | || || ( ( | | |\ V ( ( | ( (_| | |_| |
 * |_||_||_|\_||_|_| \_/ \_||_|\____|\___/
 *
 * malvado - A game engine for Lua, inspired by DIV Games Studio.
 *
 * Created by Jeremías Pérez and contributors.
 * SPDX-License-Identifier: Apache-2.0
 * See LICENSE for details.
 */

#include <cstring>
#include "raylib.h"
#include "imgui.h"
#include "rlImGui.h"

int
main(int argc, char **argv)
{
    bool smoke = argc == 2 && std::strcmp(argv[1], "--smoke") == 0;
    SetConfigFlags(FLAG_WINDOW_HIGHDPI | FLAG_WINDOW_RESIZABLE);
    InitWindow(900, 600, "Malvado G0: ImGui and raylib");
    if (!IsWindowReady())
        return 1;
    rlImGuiSetup(true);
    ImGui::GetIO().IniFilename = nullptr;
    Image image = GenImageChecked(32, 32, 8, 8, RED, GREEN);
    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);
    if (!texture.id) {
        rlImGuiShutdown();
        CloseWindow();
        return 1;
    }
    SetTargetFPS(60);
    int selected = 0;
    int frames = 0;
    while (!WindowShouldClose()) {
        Vector2 dpi = GetWindowScaleDPI();
        BeginDrawing();
        ClearBackground(Color{35, 38, 45, 255});
        rlImGuiBegin();
        ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(620, 480), ImGuiCond_FirstUseEver);
        ImGui::Begin("G0: process tree and atlas preview");
        ImGui::Text("Window DPI scale: %.2f x %.2f", dpi.x, dpi.y);
        if (ImGui::BeginTable("comparison", 2, ImGuiTableFlags_Borders)) {
            ImGui::TableNextColumn();
            ImGui::TextUnformatted("Processes");
            if (ImGui::TreeNodeEx("Root (1)", ImGuiTreeNodeFlags_DefaultOpen)) {
                if (ImGui::Selectable("Worker (2)", selected == 1)) selected = 1;
                if (ImGui::Selectable("Worker (3)", selected == 2)) selected = 2;
                ImGui::TreePop();
            }
            ImGui::Text("Selected: %d", selected);
            ImGui::TableNextColumn();
            ImGui::TextUnformatted("Atlas: region 0, 0, 16, 16");
            rlImGuiImageRect(&texture, 192, 192, Rectangle{0, 0, 16, 16});
            ImGui::EndTable();
        }
        ImGui::End();
        rlImGuiEnd();
        EndDrawing();
        if (smoke && ++frames >= 3)
            break;
    }
    UnloadTexture(texture);
    rlImGuiShutdown();
    CloseWindow();
    return 0;
}
