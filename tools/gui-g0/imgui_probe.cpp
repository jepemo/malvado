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

#define GL_SILENCE_DEPRECATION
#include <cstdint>
#include <cstring>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

int
main(int argc, char **argv)
{
    bool smoke = argc == 2 && std::strcmp(argv[1], "--smoke") == 0;
    if (!glfwInit())
        return 1;
#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
    GLFWwindow *window = glfwCreateWindow(900, 600, "Malvado G0: ImGui", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __APPLE__
    ImGui_ImplOpenGL3_Init("#version 150");
#else
    ImGui_ImplOpenGL3_Init("#version 130");
#endif
    unsigned char pixels[32 * 32 * 4];
    for (int y = 0; y < 32; ++y) {
        for (int x = 0; x < 32; ++x) {
            int offset = (y * 32 + x) * 4;
            bool red = ((x / 8 + y / 8) % 2) == 0;
            pixels[offset] = red ? 230 : 0;
            pixels[offset + 1] = red ? 41 : 228;
            pixels[offset + 2] = red ? 55 : 48;
            pixels[offset + 3] = 255;
        }
    }
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 32, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    int selected = 0;
    int frames = 0;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        float xscale, yscale;
        glfwGetWindowContentScale(window, &xscale, &yscale);
        ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(620, 480), ImGuiCond_FirstUseEver);
        ImGui::Begin("G0: process tree and atlas preview");
        ImGui::Text("Window DPI scale: %.2f x %.2f", xscale, yscale);
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
            ImGui::Image((ImTextureID)(intptr_t)texture, ImVec2(192, 192), ImVec2(0, 0), ImVec2(0.5f, 0.5f));
            ImGui::EndTable();
        }
        ImGui::End();
        ImGui::Render();
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClearColor(0.14f, 0.15f, 0.18f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        if (smoke && ++frames >= 3)
            break;
    }
    glDeleteTextures(1, &texture);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
