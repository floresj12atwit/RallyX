#include <string>
#include "entity.hpp"
#include "scene.hpp"

/**
 * Contains all information needed to display the main menu and allow the user to choose a level to play.
 * 
 * This is the first scene loaded by the gameEngine
 */
class SceneMenu : public Scene{
    std::string title;                    /* Scene name */
    std::string nameText;

    std::vector<std::string> menuStrings; /* List of menu strings that are drawn to the screen */
    std::vector<std::string> levelPaths;  /* List of relative paths to level definition files */
    std::string menuText;                 /* Other menu text */
    int selectedMenuItem=0;               /* Currently selected menu item */

    float titlePosX = -300; // Starting off-screen to the left
    float namePosX = GetScreenWidth(); // Starting off-screen to the right
    float moveSpeed = 5.0f; // Speed of the movement



    void init();
    void sRender() override;
    void sDoAction(const Action& action) override;
public:
    SceneMenu(GameEngine* gameEngine);
    void update() override;
};