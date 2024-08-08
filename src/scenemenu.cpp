#include "scenemenu.hpp"
#include "sceneplay.hpp"
#include <iostream>

SceneMenu::SceneMenu(GameEngine* gameEngine) : Scene(gameEngine){
    init();
    
}

/**
 * Initializes actions and sets strings for level paths and menu items
 */
void SceneMenu::init(){
    title="RALLY X";
    nameText = "By Joshua Flores";

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
    gameEngine->playMusic("MENUMUSIC");
}

/**
 * Renders menu scene
 */
void SceneMenu::sRender(){
    BeginDrawing();

        ClearBackground(Color(252,216,168,255));

        // Update positions
        if (titlePosX < 50) titlePosX += moveSpeed; // Move towards the desired position
        if (namePosX > 100) namePosX -= moveSpeed;   // Move towards the desired position


        //********** Raylib Drawing Content **********
        const Font& font = gameEngine->getAssets().getFont("orbitron");
        for (int i=0;i<menuStrings.size();i++) {
            Color textColor=BLACK;
            if(i==selectedMenuItem){
                textColor=RED;
            }
            DrawTextEx(font, menuStrings[i].c_str(), Vector2(50,75*(i+2)), 48, 1, textColor);
        }
        DrawTextEx(font, menuText.c_str(), Vector2(50,730), 32, 1, BLACK);


        // Draw the moving texts
        DrawTextEx(font, title.c_str(), Vector2(titlePosX, 50), 64, 1, BLACK);
        DrawTextEx(font, nameText.c_str(), Vector2(namePosX, 100), 64, 1, BLACK);

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
            gameEngine->quit();
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