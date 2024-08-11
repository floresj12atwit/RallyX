#include "scenemenu.hpp"
#include "scenestart.hpp"
#include "sceneplay.hpp"
#include <iostream>

// SceneMenu.cpp
SceneMenu::SceneMenu(GameEngine* gameEngine, int highScore) : Scene(gameEngine), highscore(highScore) {
    init();
}

/**
 * Initializes actions and sets strings for level paths and menu items
 */
void SceneMenu::init(){
    title="RALLY X";
    nameText = "By Joshua Flores";

    backgroundTexture = LoadTexture("assets/textures/background.png");

    menuStrings.push_back("Course 1");
    menuStrings.push_back("Course 2");
    menuStrings.push_back("Course 3");
    menuText = "Up: W      Down: S      Play: Space      ESC: Quit";

    levelPaths.push_back("level1.txt");
    levelPaths.push_back("level2.txt");
    levelPaths.push_back("level3.txt");

    //register input
    registerAction(KEY_S, "DOWN");
    registerAction(KEY_W, "UP");
    registerAction(KEY_SPACE, "PLAY");
    registerAction(KEY_ESCAPE, "QUIT");

    gameEngine->stopMusic("GAMEPLAYMUSIC");
    //gameEngine->playMusic("MENUMUSIC");
     // Initialize course positions
    coursePositions = {
        {GetScreenWidth() * 0.25f + 75, GetScreenHeight() * 0.75f+50},  // Course 1: bottom middle left
        {GetScreenWidth() * 0.75f-30, GetScreenHeight() * 0.75f},  // Course 2: bottom middle right
        {GetScreenWidth() * 0.75f -10, GetScreenHeight() * 0.5f-230}    // Course 3: middle right
    };

    // Initialize selection box positions (to the left of course text)
    selectionBoxSize = 40;  // Size of the selection box
    for (const auto& pos : coursePositions) {
        selectionBoxPositions.push_back({ pos.x - selectionBoxSize - 10, pos.y - selectionBoxSize / 2 });
    }
}

/**
 * Renders menu scene
 */
void SceneMenu::sRender(){
    BeginDrawing();

    // Calculate scaling factors
    float scaleX = (float)GetScreenWidth() / backgroundTexture.width;
    float scaleY = (float)GetScreenHeight() / backgroundTexture.height;

    // Destination rectangle, scaled to fit the screen
    Rectangle destRec = { 0, 0, backgroundTexture.width * scaleX, backgroundTexture.height * scaleY };

    // Source rectangle (entire texture)
    Rectangle srcRec = { 0, 0, backgroundTexture.width, backgroundTexture.height };

    // Origin point (top-left corner)
    Vector2 origin = { 0, 0 };

    // Draw the scaled texture
    DrawTexturePro(backgroundTexture, srcRec, destRec, origin, 0.0f, WHITE);

        // Update positions
        if (titlePosX < 50) titlePosX += moveSpeed; // Move towards the desired position
        if (namePosX > 100) namePosX -= moveSpeed;   // Move towards the desired position THESE ARE NOT USED FOR SCENEMENU

        // Move the high score text towards its target position
        if (highScorePosX < highScoreTargetX) {
            highScorePosX += highScoreSpeed;
            if (highScorePosX > highScoreTargetX) {
                highScorePosX = highScoreTargetX;  // Ensure it doesn't overshoot
            }
        }



        //********** Raylib Drawing Content **********
        const Font& font = gameEngine->getAssets().getFont("arcade");

        // Draw instructional text at the top left
        std::string instructionText = "Select a \n\n\n\nLevel!";
        Vector2 instructionPosition = { 40, 40 };  // Top left position
        DrawTextEx(font, instructionText.c_str(), instructionPosition, 64, 1, WHITE);

        // Draw high score under the instructional text
        std::string highScoreText = "High Score: " + std::to_string(highscore);  // Replace with actual high score variable
        Vector2 highScorePosition = { highScorePosX, 210 };  // Moving position
        DrawTextEx(font, highScoreText.c_str(), highScorePosition, 40, 1, WHITE);


      
        // Draw course selections and selection boxes
        for (int i = 0; i < menuStrings.size(); i++) {
            Color textColor = (i == selectedMenuItem) ? RED : BLACK;
            Vector2 position = coursePositions[i];
            position.x -= 140;
            position.y += 60;
            Vector2 boxPosition = selectionBoxPositions[i];

            // Draw black selection box (normal size)
            DrawRectangle(boxPosition.x, boxPosition.y, selectionBoxSize, selectionBoxSize, BLACK);

            // If the item is selected, draw a smaller red box inside the black box
            if (i == selectedMenuItem) {
                int smallerBoxSize = selectionBoxSize - 15;  // Make the red box smaller
                float padding = (selectionBoxSize - smallerBoxSize) / 2.0f;  // Center the red box
                DrawRectangle(boxPosition.x + padding, boxPosition.y + padding, smallerBoxSize, smallerBoxSize, RED);
            }

            // Draw course text
            DrawTextEx(font, menuStrings[i].c_str(), position, 40, 1, textColor);
        }
        // Draw the moving texts
        //DrawTextEx(font, title.c_str(), Vector2(titlePosX, 50), 64, 1, BLACK);
       //DrawTextEx(font, nameText.c_str(), Vector2(namePosX, 100), 64, 1, BLACK);

    EndDrawing();
}

/**
 * Performs PRESS and RELEASE actions that come from gameEngine
 * 
 * @param action Action that gets sent from gameEngine user input
 */
void SceneMenu::sDoAction(const Action& action){
    if((action.getType()=="PRESS")){
        if(action.getName()=="UP"){
            selectedMenuItem--;
            if(selectedMenuItem<0) selectedMenuItem=menuStrings.size()-1;
        }
        if(action.getName()=="DOWN"){
            selectedMenuItem++;
            if(selectedMenuItem>menuStrings.size()-1) selectedMenuItem=0;
        }
        if(action.getName()=="PLAY"){
            gameEngine->changeScene("PLAY",std::make_shared<ScenePlay>(gameEngine, levelPaths[selectedMenuItem]));
        }
        if(action.getName()=="QUIT"){
            gameEngine->changeScene("START", std::make_shared<SceneStart>(gameEngine));
            //gameEngine->quit();
        }
    }

}

/**
 * Updates the scene (rendering and Music stream updating only for the Menu Scene)
 */
void SceneMenu::update(){
    sMusic();
    sRender();
}