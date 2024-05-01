#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <include/raylib.h>
#include <imgui/imgui.h>
#include <imgui/rlImGui.h>
#include <imgui/imgui_stdlib.h>

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void) 
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 900;
    const int screenHeight = 900;

    //initialize OpenGL window through raylib
    InitWindow(screenWidth, screenHeight, "Minimal Project");
    //initialize ImGui support with raylib
    rlImGuiSetup(true);

    // Set our game to run at 60 frames-per-second
    SetTargetFPS(60);               

    //load the megaman texture using the relative path from the exe
    Texture2D tex =  LoadTexture("assets/megaStand.png");
                                        
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            //use black background
            ClearBackground(BLACK);

            //draw the megaman texture - The size is scaled by a factor of 4 and it is drawn in the center of the screen.
            int scale=4;
            DrawTexturePro(tex, {0,0,(float)tex.width,(float)tex.height}, {(float)screenWidth/2,(float)screenHeight/2,(float)tex.width*scale,(float)tex.height*scale}, {(float)tex.width/2*scale,(float)tex.height/2*scale}, 0, WHITE);

            //draw fps at window pixel point (20,20)
            DrawFPS(20, 20);


            //********** ImGUI Content *********
            //Draw imgui stuff last so it is over the top of everything else
            rlImGuiBegin();

                //show ImGui Demo Content if you want to see it
                bool open = true;
                ImGui::ShowDemoWindow(&open);

            // end ImGui Content
            rlImGuiEnd();

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(tex);   // unload megaman texture from memory
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

