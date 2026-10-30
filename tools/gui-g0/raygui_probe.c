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

#include <string.h>
#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

int
main(int argc, char **argv)
{
    int smoke = argc == 2 && strcmp(argv[1], "--smoke") == 0;
    SetConfigFlags(FLAG_WINDOW_HIGHDPI | FLAG_WINDOW_RESIZABLE);
    InitWindow(900, 600, "Malvado G0: raygui");
    if (!IsWindowReady())
        return 1;
    Image image = GenImageChecked(32, 32, 8, 8, RED, GREEN);
    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);
    if (!texture.id) {
        CloseWindow();
        return 1;
    }
    SetTargetFPS(60);
    int expanded = 1;
    int selected = 0;
    int frames = 0;
    while (!WindowShouldClose()) {
        Vector2 dpi = GetWindowScaleDPI();
        BeginDrawing();
        ClearBackground((Color){35, 38, 45, 255});
        GuiLabel((Rectangle){20, 15, 500, 25}, "G0: process tree and atlas preview");
        GuiLabel((Rectangle){20, 42, 500, 25}, TextFormat("Window DPI scale: %.2f x %.2f", dpi.x, dpi.y));
        GuiPanel((Rectangle){20, 80, 250, 420}, "Processes");
        if (GuiButton((Rectangle){30, 120, 225, 28}, expanded ? "- Root (1)" : "+ Root (1)"))
            expanded = !expanded;
        if (expanded) {
            if (GuiButton((Rectangle){50, 155, 205, 28}, "Worker (2)")) selected = 1;
            if (GuiButton((Rectangle){50, 190, 205, 28}, "Worker (3)")) selected = 2;
        }
        GuiLabel((Rectangle){30, 235, 210, 25}, TextFormat("Selected: %d", selected));
        GuiPanel((Rectangle){290, 80, 350, 420}, "Atlas");
        DrawTexturePro(texture, (Rectangle){0, 0, 16, 16}, (Rectangle){320, 130, 192, 192},
                       (Vector2){0, 0}, 0, WHITE);
        DrawRectangleLines(320, 130, 192, 192, WHITE);
        GuiLabel((Rectangle){320, 340, 280, 25}, "Region: 0, 0, 16, 16");
        EndDrawing();
        if (smoke && ++frames >= 3)
            break;
    }
    UnloadTexture(texture);
    CloseWindow();
    return 0;
}
