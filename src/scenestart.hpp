#include <string>
#include <vector>
#include "scene.hpp"

class SceneStart : public Scene {
    std::string title;                    /* Scene title */
    std::string description;              /* Scene description or instructions */
    std::vector<std::string> menuStrings; /* List of menu options */
    std::string menuText;                 /* Additional menu text (e.g., controls) */
    int selectedMenuItem = 0;             /* Currently selected menu item */
    float animationSpeed = 15.0f; // Speed of the movement for animations

    float squareSize;
    float squareSpacing;
    float baseX;
    float baseY;

    float titleOffsetX =  -GetScreenWidth();            // For animating the title moving in from the left
    float descriptionOffsetX = 2*GetScreenWidth();      // For animating the description moving in from the right

    // Textures for player, enemies, and flag
    Texture2D playerTexture;
    Texture2D enemyTexture;
    Texture2D flagTexture;

    void init();
    void sRender() override;
    void sDoAction(const Action& action) override;
public:
    SceneStart(GameEngine* gameEngine);
    void update() override;
};