#include "scenestart.hpp"
#include "sceneplay.hpp"
#include "scenemenu.hpp"
#include <iostream>

SceneStart::SceneStart(GameEngine* gameEngine)
    : Scene(gameEngine) {
    init();
    gameEngine->playMusic("MENUMUSIC");
}

/**
 * Initializes actions and sets strings for the start screen options
 */
void SceneStart::init() {
    title = "RALLYX";
    description = "By Joshua Flores";
    menuStrings.push_back("Start Game");
    menuStrings.push_back("Quit");
    //menuText = "Use WASD to Navigate + SPACE to Select   ESC: Quit";

    // Register input
    registerAction(KEY_S, "DOWN");
    registerAction(KEY_W, "UP");
    registerAction(KEY_SPACE, "SELECT");
    registerAction(KEY_ESCAPE, "QUIT");

    squareSize = 80;
    squareSpacing = 10;
    baseX = GetScreenWidth() - 4 * (squareSize + squareSpacing) - squareSpacing; // Start position X
    baseY = (GetScreenHeight() / 2 - squareSize / 2)+25; // Center vertically

    // Load textures
    playerTexture = LoadTexture("assets/textures/RallyXPlayerRight.png");
    enemyTexture = LoadTexture("assets/textures/RallyXEnemyRight.png");
    flagTexture = LoadTexture("assets/textures//LuckyFlag.png");
}

/**
 * Renders the start screen
 */
void SceneStart::sRender() {
    BeginDrawing();

    // Set background color to white
    ClearBackground(WHITE);

    const Font& font = gameEngine->getAssets().getFont("arcade");

    // Draw title with animation
    Vector2 titleSize = MeasureTextEx(font, title.c_str(), 100, 1);
    Vector2 titlePosition = Vector2(titleOffsetX, 90);
    DrawTextEx(font, title.c_str(), titlePosition, 100, 1, BLACK);

    // Draw description with animation
    Vector2 descriptionSize = MeasureTextEx(font, description.c_str(), 36, 1);
    Vector2 descriptionPosition = Vector2(descriptionOffsetX, 200);
    DrawTextEx(font, description.c_str(), descriptionPosition, 36, 1, BLACK);

    // Define the margin from the bottom-left corner
    float marginX = 20;
    float marginY = 20;

    // Calculate menu position
    float menuItemHeight = 60;
    float totalMenuHeight = menuStrings.size() * menuItemHeight;
    Vector2 menuPosition = Vector2(marginX, GetScreenHeight() - totalMenuHeight - marginY- 40);

    // Draw menu options
    for (int i = 0; i < menuStrings.size(); i++) {
        Color textColor = BLACK;
        if (i == selectedMenuItem) {
            textColor = RED; // Highlight selected item in red
        }
        Vector2 menuItemSize = MeasureTextEx(font, menuStrings[i].c_str(), 36, 1);
        DrawTextEx(font, menuStrings[i].c_str(), Vector2(menuPosition.x, menuPosition.y + i * menuItemHeight), 36, 1, textColor);
    }

    // Draw additional menu text (instructions) at the bottom of the screen in black
    Vector2 menuTextSize = MeasureTextEx(font, menuText.c_str(), 24, 1);
    Vector2 menuTextPosition = Vector2(marginX, GetScreenHeight() - menuTextSize.y - marginY);
    DrawTextEx(font, menuText.c_str(), menuTextPosition, 24, 1, BLACK);

    /*
    // Draw the squares at the middle right of the screen
    DrawRectangle(baseX, baseY, squareSize, squareSize, BLUE); // Player
    DrawRectangle(baseX - 1.5 * (squareSize + squareSpacing), baseY, squareSize, squareSize, RED);  // Enemy 1
    DrawRectangle(baseX - 2 * (squareSize + squareSpacing) , baseY + 1.25 * (squareSize + squareSpacing), squareSize, squareSize, RED); // Enemy 2
    DrawRectangle(baseX - 2 * (squareSize + squareSpacing), baseY - 1.25 * (squareSize + squareSpacing), squareSize, squareSize, RED); // Enemy 3
    DrawRectangle(baseX + 1.5 * (squareSize + squareSpacing), baseY, squareSize, squareSize, YELLOW); // Flag
    */
    // Draw the sprites with scaling
    float scale = 5;
    DrawTextureEx(playerTexture, Vector2(baseX, baseY), 0.0f, scale, WHITE); // Player
    DrawTextureEx(enemyTexture, Vector2(baseX - 1.5f * (squareSize + squareSpacing), baseY), 0.0f, scale, WHITE); // Enemy 1
    DrawTextureEx(enemyTexture, Vector2(baseX - 2.0f * (squareSize + squareSpacing), baseY + 1.25f * (squareSize + squareSpacing)), 0.0f, scale, WHITE); // Enemy 2
    DrawTextureEx(enemyTexture, Vector2(baseX - 2.0f * (squareSize + squareSpacing), baseY - 1.25f * (squareSize + squareSpacing)), 0.0f, scale, WHITE); // Enemy 3
    DrawTextureEx(flagTexture, Vector2(baseX + 1.5f * (squareSize + squareSpacing), baseY), 0.0f, scale, WHITE); // Flag

    EndDrawing();


}

/**
 * Performs PRESS and RELEASE actions that come from gameEngine
 *
 * @param action Action that gets sent from gameEngine user input
 */
void SceneStart::sDoAction(const Action& action) {
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
                // Start the game
                gameEngine->changeScene("MENU", std::make_shared<SceneMenu>(gameEngine,0)); // You can adjust the level path or make it configurable
            }
            else if (selectedMenuItem == 1) {
                // Quit the game
                gameEngine->quit();
            }
        }
        if (action.getName() == "QUIT") {
            gameEngine->quit();
        }
    }
}

/**
 * Updates the scene (rendering only for the Start Scene)
 */
void SceneStart::update() {
 

    // Update the animation positions
    titleOffsetX += animationSpeed;
    if (titleOffsetX > GetScreenWidth() / 8) titleOffsetX = GetScreenWidth() / 8; // Stop moving after it reaches its final position

    descriptionOffsetX -= animationSpeed;
    if (descriptionOffsetX < (GetScreenWidth() / 8 + 60)) descriptionOffsetX = GetScreenWidth() / 8 + 60; // Stop moving after it reaches its final position

    // Here you can add code to update player and enemy positions if needed
    sMusic();
    sRender();
}