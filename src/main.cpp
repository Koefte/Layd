#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <cstdio>
#include <memory>
#include <unordered_set>
#include <algorithm>
#define CLAY_IMPLEMENTATION// Sidebar content can go here
#include "../external/clay.h"
#include "../external/renderer.h"

#define NOB_IMPLEMENTATION
#include "../external/nob.h"




static std::string g_cur_path;
static bool g_isClickedLoadButton = false;
std::vector<std::string> g_file_names;
std::vector<Clay_String> g_file_list_cache;

static Clay_TextElementConfig textConfig = {
    .textColor = {255,255,255,255},
    .fontId = 0,
    .fontSize = 20,
};



// Run a command and capture stdout (used for system folder pickers like zenity / kdialog)
static std::string RunCommandCaptureOutput(const char *cmd) {
    std::string result;
    FILE *pipe = popen(cmd, "r");
    if (!pipe) return result;
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), pipe)) result += buffer;
    pclose(pipe);
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r')) result.pop_back();
    return result;
}

// Try platform file pickers to select a directory. Returns empty string on cancel / failure.
static std::string OpenFolderDialog() {
    // Try zenity (common on GNOME)
    std::string out = RunCommandCaptureOutput("zenity --file-selection --directory --title='Select folder' 2>/dev/null");
    if (!out.empty()) return out;
    // Try kdialog (KDE)
    out = RunCommandCaptureOutput("kdialog --getexistingdirectory 2>/dev/null");
    if (!out.empty()) return out;
    // Could add other platform integrations here if needed.
    return std::string();
}

std::string relative(const std::string& base, const std::string& path) {
    if(path.find(base) == 0) {
        return path.substr(base.length()); // +1 to remove the trailing slash
    }
    return path; // return original if not relative
}


bool walkDir(Nob_Walk_Entry entry){
    if(relative(g_cur_path,std::string(entry.path)).rfind("/.git",0) == 0){
        return true; // Stop walking
    }
    if(entry.type == NOB_FILE_DIRECTORY){
        if(g_cur_path == std::string(entry.path)) {
            return true; // Skip root
        }
        g_file_names.push_back(relative(g_cur_path,std::string(entry.path)));
        g_file_list_cache.push_back(Clay_String{
            .isStaticallyAllocated = false,
            .length = (int)g_file_names.back().size(),
            .chars = g_file_names.back().c_str()
        });
    }
    else if(entry.type == NOB_FILE_REGULAR){
        g_file_names.push_back(relative(g_cur_path,std::string(entry.path)));
        g_file_list_cache.push_back(Clay_String{
            .isStaticallyAllocated = false,
            .length = (int)g_file_names.back().size(),
            .chars = g_file_names.back().c_str()
        });
    }
    return true; // Continue walking
}


void HandleClayErrors(Clay_ErrorData errorData) {
    // Clay_String may not be null terminated; print with length
    printf("Clay error: %.*s\n", errorData.errorText.length, errorData.errorText.chars);
}

void HandleLoadButtonClicked(Clay_ElementId elementId,Clay_PointerData pointerData, void* userData) {
    // Ensure both Clay reports a press this frame AND Raylib reports a real mouse-press event
    if(pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        g_isClickedLoadButton = true;
        // Open native folder dialog and list files
        std::string path = OpenFolderDialog();
        if (!path.empty()) {
            printf("Selected folder: %s\n", path.c_str());
            g_cur_path = path;
            g_file_names.clear();
            g_file_list_cache.clear();
            // reset and walk
            walk_dir(g_cur_path.c_str(), walkDir, nullptr);
            for(const std::string& name : g_file_names) {
                printf(" - %s\n", name.c_str());
            }  
            for(Clay_String str : g_file_list_cache) {
                printf(" -- %.*s\n", str.length, str.chars);
            }
        }
        else{
            fprintf(stderr,"No folder selected or dialog failed.\n");
        }
    }
    else{
        g_isClickedLoadButton = false;
    }
}

int main(void) {
    InitWindow(1024,768,"Hello");
    Font fonts[1];
    fonts[0] = LoadFontEx("./fonts/ARIAL.TTF",48,0,400);

    uint64_t totalMemorySize = Clay_MinMemorySize();
    Clay_Arena clayMemory = Clay_CreateArenaWithCapacityAndMemory(totalMemorySize, (char *)malloc(totalMemorySize));
    Clay_Initialize(clayMemory, Clay_Dimensions {1024,768}, Clay_ErrorHandler { HandleClayErrors });
    Clay_SetMeasureTextFunction(Raylib_MeasureText,fonts);


    while(!WindowShouldClose()){
        Clay_BeginLayout();
        // Sidebar: 1/4 width, 100% height
        Clay_LayoutConfig sidebarLayout = Clay_LayoutConfig {
            .sizing = {
                .width = CLAY_SIZING_PERCENT(0.25f),
                .height = CLAY_SIZING_PERCENT(1.0f)
            },
            .padding = {5, 5, 5, 5},
            .childGap = 0,
            .childAlignment = {CLAY_ALIGN_X_LEFT, CLAY_ALIGN_Y_TOP},
            .layoutDirection = CLAY_TOP_TO_BOTTOM
        };
        CLAY_AUTO_ID({ .layout = sidebarLayout, .backgroundColor = {100,100,100,255} }) {
            CLAY_TEXT(CLAY_STRING("Load folder"), &textConfig);
            Clay_OnHover(HandleLoadButtonClicked, nullptr);
            for(const Clay_String& filePath : g_file_list_cache) {
                CLAY_TEXT(filePath, &textConfig);
            }

        }
        Clay_RenderCommandArray renderCommands = Clay_EndLayout();
        // Update pointer state so on-hover and click callbacks work
        Vector2 mp = GetMousePosition();
        Clay_SetPointerState((Clay_Vector2){ mp.x, mp.y }, IsMouseButtonDown(MOUSE_BUTTON_LEFT));
        BeginDrawing();
        ClearBackground(BLACK);
        Clay_Raylib_Render(renderCommands,fonts);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}