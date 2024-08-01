#pragma once
#include <string>
#include <imgui/imgui.h>
#include <imgui/rlImGui.h>
#include <imgui/imgui_stdlib.h>
#include "entity.hpp"
#include "scene.hpp"

/**
 * Contains the player data from the level definition file. Filled when a scene loads a level.
 */
struct PlayerConfig{float X,Y,BX,BY,SPEED,HEALTH; std::string WEAPON;};
struct FollowAIConfig { std::string name, AI; int RoomX, RoomY, GX, GY, SPEED, HEALTH; };
struct PatrolAIConfig { std::string name, AI; int RoomX, RoomY, GX, GY, SPEED, HEALTH, NP; };


/**
 * Scene that contains all logic and information for playing a loaded level
 */
class ScenePlay : public Scene{
    private:
        PlayerConfig playerConfig;      /* Information about the properties of the player for this level */
        FollowAIConfig followAI;
        PatrolAIConfig patrolAI;


        std::string levelPath;          /* Relative path to the level definition file from exe */
        std::shared_ptr<Entity> player; /* Shared pointer reference to the player entity */
        bool reload=false;              /* If the Scene should be reloaded at the end of the current frame */
        bool renderHealth=true;         /* If the entity health should be rendered */
        bool followCam=false;           /* If the locked follow camera should be active, false=room cam */
        Vec2 room = {0,0};              /* Current (x,y) room. (0,0) is the starting room */

        void init(const std::string& levelPath);
        void sAnimation();
        void sMovement();
        void sCollision();
        void sRender();
        void sGUI();
        void sDoAction(const Action& action);
        void sCamera();
        void spawnPlayer();
        void spawnSword();
        void loadLevel(const std::string& levelPath);
        void reloadScene();
        Vec2 getPosition(int rx, int ry, int tx, int ty);
        void renderHealthBar();
        void renderAIDebug();
    public:
        ScenePlay(GameEngine* gameEngine,std::string levelPath);
        ScenePlay()=default;
        void update();
};