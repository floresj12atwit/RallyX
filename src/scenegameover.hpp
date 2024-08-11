#include <string>
#include "entity.hpp"
#include "scene.hpp"

/**
 * Contains all information needed to display the game over screen and allow the user to retry the level or return to the main menu.
 *
 * This scene is loaded when the player loses.
 */
class SceneGameOver : public Scene {
    std::string title;                    /* Scene name */
    int score;                           /* Final score of the player */
    std::vector<std::string> menuStrings; /* List of menu strings that are drawn to the screen */
    std::string menuText;                 /* Other menu text */
    int selectedMenuItem = 0;             /* Currently selected menu item */
    std::string currentLevelPath;         /* Path to the current level */

    void init();
    void sRender() override;
    void sDoAction(const Action& action) override;

public:
    SceneGameOver(GameEngine* gameEngine, int finalScore, std::string levelPath);
    void update() override;
};