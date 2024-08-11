#include "scenegameover.hpp"
#include "scenemenu.hpp"
#include "sceneplay.hpp"
#include <iostream>

SceneGameOver::SceneGameOver(GameEngine* gameEngine, int finalScore, std::string levelPath)
    : Scene(gameEngine), score(finalScore), currentLevelPath(levelPath) {
    init();
}

/**
 * Initializes actions and sets strings for the game over options
 */
void SceneGameOver::init() {
    title = "GAME OVER";
    menuStrings.push_back("Retry");
    menuStrings.push_back("Main Menu");
    menuText = "Up: W   Down: S   Select: Space   ESC: Quit";

    // Register input
    registerAction(KEY_S, "DOWN");
    registerAction(KEY_W, "UP");
    registerAction(KEY_SPACE, "SELECT");
    registerAction(KEY_ESCAPE, "QUIT");
}

/**
 * Renders the game over scene
 */
void SceneGameOver::sRender() {
    BeginDrawing();

    // Set background color to black
    ClearBackground(BLACK);

    const Font& font = gameEngine->getAssets().getFont("arcade");

    // Draw title at the top middle of the screen in white
    Vector2 titlePosition = Vector2((GetScreenWidth() - MeasureTextEx(font, title.c_str(), 48, 1).x) / 2, 50);
    DrawTextEx(font, title.c_str(), titlePosition, 48, 1, WHITE);

    // Draw score below the title in white
    std::string scoreText = "Score: " + std::to_string(score);
    Vector2 scorePosition = Vector2((GetScreenWidth() - MeasureTextEx(font, scoreText.c_str(), 36, 1).x) / 2, 120);
    DrawTextEx(font, scoreText.c_str(), scorePosition, 36, 1, WHITE);

    // Calculate menu position
    float menuItemHeight = 60;
    float totalMenuHeight = menuStrings.size() * menuItemHeight;
    Vector2 menuPosition = Vector2((GetScreenWidth() - MeasureTextEx(font, menuStrings[0].c_str(), 36, 1).x) / 2, (GetScreenHeight() - totalMenuHeight) / 2);

    // Draw menu options
    for (int i = 0; i < menuStrings.size(); i++) {
        Color textColor = WHITE;
        if (i == selectedMenuItem) {
            textColor = RED;
        }
        DrawTextEx(font, menuStrings[i].c_str(), Vector2(menuPosition.x, menuPosition.y + i * menuItemHeight), 36, 1, textColor);
    }

    // Draw additional menu text (instructions) at the bottom of the screen in white
    DrawTextEx(font, menuText.c_str(), Vector2((GetScreenWidth() - MeasureTextEx(font, menuText.c_str(), 24, 1).x) / 2, GetScreenHeight() - 50), 24, 1, WHITE);

    EndDrawing();
}

/**
 * Performs PRESS and RELEASE actions that come from gameEngine
 *
 * @param action Action that gets sent from gameEngine user input
 */
void SceneGameOver::sDoAction(const Action& action) {
    if (action.getType() == "PRESS") {
        if (action.getName() == "UP") {
            selectedMenuItem--;
            if (selectedMenuItem < 0) selectedMenuItem = menuStrings.size() - 1;
        }
        if (action.getName() == "DOWN") {
            selectedMenuItem++;
            if (selectedMenuItem >= menuStrings.size()) selectedMenuItem = 0;
        }
        if (action.getName() == "SELECT") {
            if (selectedMenuItem == 0) {
                // Retry the level
                gameEngine->changeScene("PLAY", std::make_shared<ScenePlay>(gameEngine, currentLevelPath));
            }
            else if (selectedMenuItem == 1) {
                // Go to the main menu
                gameEngine->changeScene("MENU", std::make_shared<SceneMenu>(gameEngine,highscore));
                gameEngine->playMusic("MENUMUSIC");
            }
        }
        if (action.getName() == "QUIT") {
            gameEngine->quit();
        }
    }
}

/**
 * Updates the scene (rendering and Music stream updating only for the Game Over Scene)
 */
void SceneGameOver::update() {
    sRender();
}
