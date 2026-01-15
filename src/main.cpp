#include <iostream>
#define CLAY_IMPLEMENTATION
#include "../external/clay.h"
#include "../external/renderer.h"

Clay_LayoutConfig layoutElement = Clay_LayoutConfig { .padding = {5} };

void HandleClayErrors(Clay_ErrorData errorData) {
    // Clay_String may not be null terminated; print with length
    printf("Clay error: %.*s\n", errorData.errorText.length, errorData.errorText.chars);
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
        CLAY_AUTO_ID({ .layout = layoutElement, .backgroundColor = {255,255,255,255} }) {
            CLAY_TEXT(CLAY_STRING("HELLO"), CLAY_TEXT_CONFIG({ .textColor = {100,200,255,255}, .fontId = 0, .fontSize = 48 }));
        }
        Clay_RenderCommandArray renderCommands = Clay_EndLayout();

       
        BeginDrawing();
        ClearBackground(BLACK);
        Clay_Raylib_Render(renderCommands,fonts);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}