#include "sceneplay.hpp"
#include "scenemenu.hpp"
#include "scenegameover.hpp"
#include <fstream>
#include <iostream>
#include <cstdio>

#include <vector>
#include <algorithm>
#include "physics.hpp"



/**
 * Constructor for the Play Scene
 * 
 * @param gameEngine raw pointer to the game engine class
 * @param levelPath Path to level defintion file, relative to exe
 */
ScenePlay::ScenePlay(GameEngine* gameEngine, std::string levelPath):Scene(gameEngine){
    this->levelPath=levelPath;
    
    init(this->levelPath);
    gameEngine->stopMusic("MENUMUSIC");
    gameEngine->playSound("GAMESTARTMUSIC");
    
    renderStartTransition(5);
    
}

/**
 * Functions that initializes the scene:
 * 
 * 1. Loads Level information
 * 2. Spawns Players  
 * 3. Registers input action  
 * 4. Sets up camera  
 * 
 * @param levelPath Path to level defintion file, relative to exe
 */
void ScenePlay::init(const std::string& levelPath){
    loadLevel(levelPath);
    if (levelPath == "level1.txt") {
        courseNumber = "1";
    }
    else if (levelPath == "level2.txt") {
        courseNumber = "2";
    }
    else if (levelPath == "level3.txt") {
        courseNumber = "3";
    }
    
    gameEngine->playMusic("GAMEPLAYMUSIC");
    spawnPlayer();

    //TODO: Add actions for UP, DOWN, LEFT, RIGHT, and ATTACK
    registerAction(KEY_B,"BB");
    registerAction(KEY_G,"GRID");
    registerAction(KEY_T,"TEX");
    registerAction(KEY_H,"HEALTH");
    registerAction(KEY_V,"VISION");
    registerAction(KEY_ESCAPE,"QUIT");
    registerAction(KEY_R,"RELOAD");

    registerAction(KEY_J, "DEBUG");


    //Adding movement actions
    registerAction(KEY_W, "UP");
    registerAction(KEY_A, "LEFT");
    registerAction(KEY_S, "DOWN");
    registerAction(KEY_D, "RIGHT"); 
    registerAction(KEY_SPACE, "SMOKE");


    
    mainCamera=Camera2D({gameEngine->getWidth()/2,gameEngine->getHeight()/2},{gameEngine->getWidth()/2,gameEngine->getHeight()/2},0,1);

}

/**
 * Loads level information from level definition file.
 * 
 * Once loaded, the correct entity type is created and setup.
 * 
 * @param levelPath Path to level defintion file, relative to exe
 */
void ScenePlay::loadLevel(const std::string& levelPath){
    //TODO: Add the reading of the level file (Modify what you did from Assignment 3)

    std::ifstream file(levelPath);
    std::string str;
    std::string type;
    

    
    while (file.good()) {
        file >> str;
        if (str == "TILE") {
            
            int RoomX, RoomY, GridX, GridY;
            file >> type >> RoomX >> RoomY >> GridX >> GridY;
            auto e = entityManager.addEntity(str, type);
            e->addComponent<CAnimation>(gameEngine->getAssets().getAnimation(type), true);
            float scaledX = gameEngine->getAssets().getAnimation(type).getScaledSize().x;
            float scaledY = gameEngine->getAssets().getAnimation(type).getScaledSize().y;
            float tileSizeX = gameEngine->getTileSizeX();
            float tileSizeY = gameEngine->getTileSizeY();
            float bboxSizeX = tileSizeX;
            float bboxSizeY = tileSizeY;
            if (scaledX > tileSizeX || scaledY > tileSizeY) {
                bboxSizeX = scaledX;
                bboxSizeY = scaledY;
            }
            e->addComponent<CBoundingBox>(Vec2(bboxSizeX, bboxSizeY));    //we need to set true because these tiles block vision
            e->getComponent<CBoundingBox>().blocksVision = true;
            Vec2 tileWorldPosition = getPosition(RoomX, RoomY, GridX, GridY);


            int newGridX = tileWorldPosition.x / gameEngine->getTileSizeX();
            int newGridY = tileWorldPosition.y / gameEngine->getTileSizeY();

            
            // Get the center pixel position adjusted for the entity size
            Vec2 pos = gridToMidPixel(newGridX, newGridY, e);

            playerStart = pos;

            e->addComponent<CTransform>(Vec2(pos.x, pos.y), Vec2(0.0f, 0.0f), 0.0f);
            e->getComponent<CTransform>().prevPosition.x = pos.x;
            e->getComponent<CTransform>().prevPosition.y = pos.y;

            if (type == "FLAG" || type == "SPECIALFLAG" || type=="LUCKYFLAG") {
                e->getComponent<CBoundingBox>().blocksVision = false;
                e->addComponent<CBoundingBox>(Vec2(bboxSizeX/2, bboxSizeY/2));

                



            }

            if (type == "ROCK1" || type == "ROCK2") {
                bboxSizeY = bboxSizeY - 30;
                e->getComponent<CBoundingBox>().blocksVision = true;
                e->addComponent<CBoundingBox>(Vec2(bboxSizeX, bboxSizeY));


            }
            if (type == "SPEEDUPTILEUP" || type == "SPEEDUPTILEDOWN") {
                bboxSizeY = bboxSizeY - 30;
                e->getComponent<CBoundingBox>().blocksVision = false;
                e->addComponent<CBoundingBox>(Vec2(bboxSizeX, bboxSizeY));


            }

        }
        if (str == "PLAYER") {
            file >> playerConfig.X >> playerConfig.Y >> playerConfig.BX >> playerConfig.BY >> playerConfig.SPEED >> playerConfig.HEALTH >> playerConfig.WEAPON;
        }
        if (str == "ENEMY") {
            std::string enemyName, AIType;
            int RoomX, RoomY, GridX, GridY, Speed, Health, NP;
            

            file >> enemyName >> AIType >> RoomX >> RoomY >> GridX >> GridY >> Speed >> Health;

            


                auto e = entityManager.addEntity("DYNAMIC", str);
                //std::cout << enemyName << std::endl;
                e->addComponent<CAnimation>(gameEngine->getAssets().getAnimation(enemyName), true);
                float scaledX = gameEngine->getAssets().getAnimation(enemyName).getScaledSize().x;
                float scaledY = gameEngine->getAssets().getAnimation(enemyName).getScaledSize().y;
                float tileSizeX = gameEngine->getTileSizeX();
                float tileSizeY = gameEngine->getTileSizeY();
                float bboxSizeX = tileSizeX;
                float bboxSizeY = tileSizeY;
                if (scaledX > tileSizeX || scaledY > tileSizeY) {
                    bboxSizeX = scaledX;
                    bboxSizeY = scaledY;
                }
                e->addComponent<CBoundingBox>(Vec2(bboxSizeX, bboxSizeY));
                
                Vec2 tileWorldPosition = getPosition(RoomX, RoomY, GridX, GridY);


                int newGridX = tileWorldPosition.x / gameEngine->getTileSizeX();
                int newGridY = tileWorldPosition.y / gameEngine->getTileSizeY();


                // Get the center pixel position adjusted for the entity size
                Vec2 pos = gridToMidPixel(newGridX, newGridY, e);

                enemyStartPositions.push_back(pos);


                //pos.y -= scaledY;
                e->addComponent<CTransform>(Vec2(pos.x, pos.y), Vec2(0.0f, 0.0f), 0.0f);
                e->getComponent<CTransform>().prevPosition.x = pos.x;
                e->getComponent<CTransform>().prevPosition.y = pos.y;
                e->getComponent<CTransform>().velocity.x = Speed;
                e->getComponent<CTransform>().velocity.y = Speed;
                //e->addComponent<CHealth>(Health,Health);
                e->addComponent<CDamage>();
                //e->addComponent<CInvincibility>();
                
                e->addComponent<CState>("CHASING");

              
                
            //Edit this so that patrol points get placed in correct room that entity is placed
            if (AIType == "PATROL") {
                file >> NP;
                std::vector<Vec2> patrolPoints(NP);
                for (int i = 0; i < NP; ++i) {
                    int px, py;
                    file >> px >> py;
                    Vec2 noRoom;
                    //Figure out what's going on here its wrong in the Y direction
                    //Vec2 worldPos = getPosition(RoomX, RoomY, px, py);

                    Vec2 tileWorldPosition = getPosition(RoomX, RoomY, px, py);


                    int newGridX = tileWorldPosition.x / gameEngine->getTileSizeX();
                    int newGridY = tileWorldPosition.y / gameEngine->getTileSizeY();


                    // Get the center pixel position adjusted for the entity size
                    Vec2 pos = gridToMidPixel(newGridX, newGridY, e);
                    patrolPoints[i] = pos; // Put the points in the correct world position
                    //patrolPoints[i] = getPosition(RoomX, RoomY, noRoom.x, noRoom.y);

                }
                e->addComponent<CPatrol>(patrolPoints, Speed);
            }
            else {
                e->addComponent<CFollowPlayer>(); //Initalize this to nothing and then when game loop starts make this enemy move towards player
                e->getComponent<CFollowPlayer>().speed = Speed;
            }
            
            

           

        }
        if (str == "DEC") {
            int RoomX, RoomY, GridX, GridY;
            file >> type >> RoomX >> RoomY >> GridX >> GridY;
            
            auto e = entityManager.addEntity(str, type);
            e->addComponent<CAnimation>(gameEngine->getAssets().getAnimation(type), true);
            Vec2 tileWorldPosition = getPosition(RoomX, RoomY, GridX, GridY);


            int newGridX = tileWorldPosition.x / gameEngine->getTileSizeX();
            int newGridY = tileWorldPosition.y / gameEngine->getTileSizeY();


            // Get the center pixel position adjusted for the entity size
            Vec2 pos = gridToMidPixel(newGridX, newGridY, e);
            e->addComponent<CTransform>(Vec2(pos.x, pos.y), Vec2(0.0f, 0.0f), 0.0f);


        }
        
    }

}
/*




*/
/**
 * Animation System
 * 
 * For the player: Sets the state and animation depending on user input.
 * For other entities: Updates the animation via the CAnimation component.
 *
 */
void ScenePlay::sAnimation(){
    //TODO: Based on the current state and user input, change the animation of the player
    
    CState& state = player->getComponent<CState>();
    CInput& input = player->getComponent<CInput>();
    CAnimation& animationComponent = player->getComponent<CAnimation>();

    static int attackFrameCount = 0;
    

    std::string lastAnimation;
    std::string newAnimation;       //Copied this from last project just update the string don't make a new animation
    static std::string lastDirection;   //Making this a static variable helped to ensure the player would actually stop in its last animation
        //Clever way to get ensure the last movement player made is where he stops
    static std::string enemyAnimation;


    if (player->getComponent<CLifespan>().remaining==fullTank) {
        lastDirection = "U";
        lastAnimation = "RALLYXPLAYERUP";
        newAnimation = "RALLYXPLAYERUP";
    }
    
    // Determine the direction to move based on priority
    if (input.left && !input.right && !input.up && !input.down) {
        newAnimation = "RALLYXPLAYERRIGHT";
        lastDirection = "L";
    }
    else if (input.right && !input.left && !input.up && !input.down) {
        newAnimation = "RALLYXPLAYERRIGHT";
        lastDirection = "R";
    }
    else if (input.up && !input.down && !input.left && !input.right) {
        newAnimation = "RALLYXPLAYERUP";
        lastDirection = "U";
    }
    else if (input.down && !input.up && !input.left && !input.right) {
        newAnimation = "RALLYXPLAYERDOWN";
        lastDirection = "D";
    }
    else {
        // If conflicting inputs are detected or no direction is pressed, continue with the last valid direction
        if (lastDirection == "L") {
            newAnimation = "RALLYXPLAYERRIGHT";
        }
        else if (lastDirection == "R") {
            newAnimation = "RALLYXPLAYERRIGHT";
        }
        else if (lastDirection == "U") {
            newAnimation = "RALLYXPLAYERUP";
        }
        else if (lastDirection == "D") {
            newAnimation = "RALLYXPLAYERDOWN";
        }
        else {
            // Default direction when no input and no last direction
            newAnimation = "RALLYXPLAYERUP"; // Default to facing up
            lastDirection = "U";
        }
    }


 
    
    
   
    
    if (!newAnimation.empty() && animationComponent.animation.getName() != newAnimation) {
        
        
        animationComponent.animation = gameEngine->getAssets().getAnimation(newAnimation);
    }


    //Going to handle 
//  animations now need a loop because there's more than 1 
    // Handle Enemy Animations
    for (auto& e : entityManager.getEntities("DYNAMIC")) {
        if (e->getID() == "ENEMY") {
            CState& enemyState = e->getComponent<CState>();
            CAnimation& enemyAnimation = e->getComponent<CAnimation>();
            auto& enemyTransform = e->getComponent<CTransform>();


            //std::cout << "I am here lol" << std::endl;

            std::string enemyNewAnimation;
            

            //std::cout << enemyAnimation.animation.getName() << std::endl;
            static std::string enemyLastDirection;
            
            // Determine the enemy's direction based on their state

            
            
            
            
            
            
            

            if (enemyTransform.facing.x == -1) {
                enemyNewAnimation = "RALLYXENEMYRIGHT"; // Assuming right animation is flipped for left
                enemyLastDirection = "L";
            }
            else if (enemyTransform.facing.x == 1) {
                enemyNewAnimation = "RALLYXENEMYRIGHT";
                enemyLastDirection = "R";
            }
            else if (enemyTransform.facing.y == -1) {
                enemyNewAnimation = "RALLYXENEMYUP";
                enemyLastDirection = "U";
            }
            else if (enemyTransform.facing.y == 1) {
                enemyNewAnimation = "RALLYXENEMYDOWN";
                enemyLastDirection = "D";
            }
            else {
                // If conflicting directions or no movement, continue with the last valid direction
                if (enemyLastDirection == "L") {
                    enemyNewAnimation = "RALLYXENEMYRIGHT";
                }
                else if (enemyLastDirection == "R") {
                    enemyNewAnimation = "RALLYXENEMYRIGHT";
                }
                else if (enemyLastDirection == "U") {
                    enemyNewAnimation = "RALLYXENEMYUP";
                }
                else if (enemyLastDirection == "D") {
                    enemyNewAnimation = "RALLYXENEMYDOWN";
                }
            }

            if (enemyState.state == "InSmoke") {
                enemyNewAnimation = "ENEMYCARSPIN";
            }

            
            if (enemyTransform.velocity.x != 0 || enemyTransform.velocity.y != 0) {
                // The enemy is moving, so set the state to "CHASING"
                e->getComponent<CState>().state = "CHASING";
            }

            
          

            
            // Update the enemy's animation if it has changed
            if (!enemyNewAnimation.empty() && enemyAnimation.animation.getName() != enemyNewAnimation) {
                enemyAnimation.animation = gameEngine->getAssets().getAnimation(enemyNewAnimation);
            }

        }
    }

    
    //Update all the animation frames
    for(auto& e : entityManager.getEntities()){

        if(e->hasComponent<CAnimation>()){
            e->getComponent<CAnimation>().animation.update();
        }
    }
}


// Helper function to update facing direction based on movement
void ScenePlay::updateFacingDirection(const std::shared_ptr<Entity>& e, Vec2 velocity) {
    if (velocity.x > 0) {
        e->getComponent<CTransform>().facing = { 1, 0 }; // Facing right
    }
    else if (velocity.x < 0) {
        e->getComponent<CTransform>().facing = { -1, 0 }; // Facing left
    }
    else if (velocity.y > 0) {
        e->getComponent<CTransform>().facing = { 0, 1 }; // Facing down
    }
    else if (velocity.y < 0) {
        e->getComponent<CTransform>().facing = { 0, -1 }; // Facing up
    }
}

Vec2 ScenePlay::getRandomDirection() {
    int randomDir = rand() % 4;
    switch (randomDir) {
    case 0: return Vec2(1, 0);  // Right
    case 1: return Vec2(-1, 0); // Left
    case 2: return Vec2(0, 1);  // Down
    case 3: return Vec2(0, -1); // Up
    default: return Vec2(1, 0); // Default to right
    }
}
//My solution relies on the random movement of enemies and them only pursuing if they are perfectly aligned for the player 



/**
 * Movement System
 * 
 * Moves all DYNAMIC entities
 * 
 */
void ScenePlay::sMovement(){

    float deltaTime = GetFrameTime();
    float currentTime = GetTime();

    static float lastAttackTime = 0.0f;
     const float ATTACK_COOLDOWN = 0.15f;
     static bool canAttack = true;
     float maxSpeed = playerConfig.SPEED;
    

    for(auto& e : entityManager.getEntities("DYNAMIC")){

        //TODO: Implement the movement for DYNAMIC entities
        //1. If the entity is the player, use the input bools to determine direction and x velocity.
        //2. Ensure that the player cannot move diagonally
        //3. This is also a good place to update the lifespan component
       

        auto& transform = e->getComponent<CTransform>();
        auto& state = e->getComponent<CState>();

        //Adjust previous position of entity
        transform.prevPosition.x = transform.position.x;
        transform.prevPosition.y = transform.position.y;
        
        float velocity = playerConfig.SPEED;

        // Reset velocity
        /*
        if (e->getID() != "PLAYER") {
            transform.velocity = { 0.0f, 0.0f };
        }
        */
        
        
       


        //std::cout << player->getComponent<CTransform>().position.x << std::endl;
       // std::cout << player->getComponent<CTransform>().position.y << std::endl;

        //Enemy Movement
        if (e->getID() == "ENEMY") {
           

            //std::string AIType = e->getComponent<CAnimation>().animation.getName();
            //std::cout << e->getComponent<CFollowPlayer>().home = 
            
            if (e->getComponent<CFollowPlayer>().has) {
                bool canSeePlayer=true;

                
                //Vec2& playerPos = player->getComponent<CTransform>().position;
                //Vec2& enemyPos = transform.position;         

                Vec2 playerPos = player->getComponent<CTransform>().position;
                Vec2 enemyPos = e->getComponent<CTransform>().position;
                Vec2& enemyVel = e->getComponent<CTransform>().velocity;
                Vec2 enemySize = e->getComponent<CAnimation>().animation.getScaledSize();
                e->getComponent<CFollowPlayer>().home = playerPos;


                if (frameCounter < startMoveFrames) {
                    // Move enemies forward (change this direction if needed)
                    enemyVel = Vec2(0.0f, -e->getComponent<CFollowPlayer>().speed); // Moving to the right for example
                }



                Vec2 prevVelocity = enemyVel;
                bool isMoving = (enemyVel.x != 0.0f || enemyVel.y != 0.0);

               

                for (auto& tile : entityManager.getEntities("TILE")) {
                    


                    // Skip entities that don't block vision but they all do in this case
                    if (!tile->getComponent<CBoundingBox>().blocksVision) continue;

                    // Check if this entity blocks the line of sight
                    if (Physics::entityIntersect(enemyPos, playerPos, tile)) {
                        canSeePlayer = false;
                        break;  // No need to check further if view is blocked
                    }
                }


                if (roundClear) {
                    e->getComponent<CTransform>().velocity = { 0,0 };
                    e->getComponent<CTransform>().position = e->getComponent<CTransform>().prevPosition;
                    canSeePlayer = false;
                    isMoving = true;

                }


                if(canSeePlayer){


                  
                    

                   
                // Calculate the direction vector towards the player
                Vec2 direction = playerPos - enemyPos;
                
                if (player->getComponent<CTransform>().velocity == Vec2(0, 0)) {
                    //direction = get;

                    // Set the enemy's velocity to move straight towards the player
                    e->getComponent<CTransform>().velocity = prevVelocity;
                   
                }
                
                // Check if enemy is aligned with the direction towards the player
                else if (std::abs(direction.x) > std::abs(direction.y)) {
                    float yDiff = std::abs(enemyPos.y - playerPos.y);

                    // Define the tile height
                    float tileHeight = 64.0f; // Adjust this value based on your game's tile size
                    if (yDiff <= tileHeight && enemyVel.x == 0.0f && e->getComponent<CTransform>().prevPosition == enemyPos) {
                        std::cout << "I'm where you need me to be" << std::endl;
                        // Snap the enemy to the player's y-position
                        e->getComponent<CTransform>().position.y = playerPos.y;
                    }

                    // Move horizontally
                    if (direction.x > 0) {
                        std::cout << "Moving right" << std::endl;
                        enemyPos.x = playerPos.x - enemySize.x / 2; // Align the enemy's bounding box with the player's
                        enemyVel = { e->getComponent<CFollowPlayer>().speed, 0.0f };
                        e->getComponent<CTransform>().facing = { 1, 0 }; // Facing right
                    }
                    else {

                        

                       

                        std::cout << "Moving left" << std::endl;
                       
                        // Align the enemy's bounding box with the player's
                        enemyVel = { -e->getComponent<CFollowPlayer>().speed, 0.0f };
                        e->getComponent<CTransform>().facing = { -1, 0 }; // Facing left
                        // Check if the enemy is within one tile in the y-axis and is stuck
                        
                        
                    }
                }
                else {
                    /*
                    float xDiff = std::abs(enemyPos.x - playerPos.x);

                    // Define the tile width
                    float tileWidth = 64.0f; // Adjust this value based on your game's tile size
                    if (xDiff <= tileWidth && enemyVel.y == 0.0f && e->getComponent<CTransform>().prevPosition == enemyPos) {
                        std::cout << "I'm where you need me to be" << std::endl;
                        // Snap the enemy to the player's x-position
                        e->getComponent<CTransform>().position.x = playerPos.x;
                    }
                    */

                    // Move vertically

                    if (direction.y > 0) {
                        std::cout << "Moving down" << std::endl;
                         // Align the enemy's bounding box with the player's
                        enemyPos.y = playerPos.y + enemySize.y / 2;
                        enemyVel = { 0.0f, e->getComponent<CFollowPlayer>().speed };
                        e->getComponent<CTransform>().facing = { 0, 1 }; // Facing down
                    }
                    else {
                        std::cout << "Moving up" << std::endl;
                        enemyPos.y = playerPos.y - enemySize.y / 2; // Align the enemy's bounding box with the player's
                        enemyVel = { 0.0f, -e->getComponent<CFollowPlayer>().speed };
                        e->getComponent<CTransform>().facing = { 0, -1 }; // Facing up
                    }
                }
                
              
                }
               
                else if(!canSeePlayer){
                    //Random movement
                    
                    

                    if (!isMoving) {
                        // Choose a new random direction if the enemy is stationary or hit a wall
                        
                        //e->getComponent<CTransform>().isStuck = false;
                        Vec2 newDirection = getRandomDirection();
                        std::cout << newDirection.x << std::endl;
                        std::cout << newDirection.y << std::endl;
                        enemyVel = newDirection * e->getComponent<CFollowPlayer>().speed;
                        e->getComponent<CTransform>().facing = newDirection;
                        
                    }
                    
                   


                }
                
                if (frameCounter < startMoveFrames) {
                    frameCounter++;
                }

            }
            else if (e->getComponent<CPatrol>().has) {
                //do patrol 
                auto& patrol = e->getComponent<CPatrol>();
                auto& transform = e->getComponent<CTransform>();

                Vec2 enemyPos = transform.position;     //Current position of the enemy 
                Vec2 nextPatrolPoint = patrol.positions[patrol.currentPosition];    //Vec2 to hold the patrol point starts as the initial

                
                Vec2 toNextPoint = nextPatrolPoint - enemyPos;      //Calculating distance to next control point
                float distanceToNext = toNextPoint.length();        //Finds length of the vector from the enemy to the next point I think dist could have been used here 

                
                if (distanceToNext < 5.0f) {  // This can be adjusted but I think this works fine
                    // Move to the next patrol point
                    patrol.currentPosition = (patrol.currentPosition + 1) % patrol.positions.size();    //Cycles through the indices without a loop using %
                    nextPatrolPoint = patrol.positions[patrol.currentPosition];
                    toNextPoint = nextPatrolPoint - enemyPos;
                }

                // Calculate desired velocity
                Vec2 desired = toNextPoint;
                desired.normalize();
                float speedx = patrol.speed;    //Did this to get both x and y speeds even though they're the same
                float speedy = patrol.speed;
                Vec2 enemySpeed = { speedx,speedy };
                desired *= enemySpeed;

                
                transform.velocity = desired;    //set entity velocity to direction vector 

            }
           


        }

        //Player movement
        if (e->getID() == "PLAYER") {

           
            //TraceLog(LOG_INFO, "Player Position: x = %.2f, y = %.2f", e->getComponent<CTransform>().position.x, e->getComponent<CTransform>().position.y);
            auto& input = e->getComponent<CInput>();
            auto& state = e->getComponent<CState>();

            static Vec2 lastVelocity = { 0.0f, 0.0f };  // Remember the last valid velocity
            static Vec2 lastFacing = { 0, 0 };          // Remember the last facing direction
            
            /*
            if (std::abs(transform.velocity.x) > maxSpeed) {
                transform.velocity.x = (transform.velocity.x > 0) ? maxSpeed : -maxSpeed;
            }

            // Clamp velocity in the y-direction
            if (std::abs(transform.velocity.y) > maxSpeed) {
                transform.velocity.y = (transform.velocity.y > 0) ? maxSpeed : -maxSpeed;
            }
            */
            
            if (input.up && !input.down && !input.left && !input.right) {
                transform.velocity = { 0.0f, -velocity };
                transform.facing = { 0, -1 };
                lastVelocity = transform.velocity;
                lastFacing = transform.facing;
            }
            else if (input.down && !input.up && !input.left && !input.right) {
                transform.velocity = { 0.0f, velocity };
                transform.facing = { 0, 1 };
                lastVelocity = transform.velocity;
                lastFacing = transform.facing;
            }
            else if (input.left && !input.right && !input.up && !input.down) {
                transform.velocity = { -velocity, 0.0f };
                transform.facing = { -1, 0 };
                lastVelocity = transform.velocity;
                lastFacing = transform.facing;
            }
            else if (input.right && !input.left && !input.up && !input.down) {
                transform.velocity = { velocity, 0.0f };
                transform.facing = { 1, 0 };
                lastVelocity = transform.velocity;
                lastFacing = transform.facing;
            }
            else if ((input.left && input.right) || (input.up && input.down)) {
                // If both opposite directions are pressed, maintain the last valid direction
                transform.velocity = lastVelocity;
                transform.facing = lastFacing;
            }
        

            if (!input.attack) {
                canAttack = true;
                frameCounter = 10;
            }


            if (input.attack && canAttack && (currentTime - lastAttackTime >= ATTACK_COOLDOWN) && frameCounter > 0) {
                spawnSmoke();

               
                gameEngine->playSound("SMOKESOUND");
                

                frameCounter--;  // Decrement smokeFrames each time smoke is spawned
                e->getComponent<CLifespan>().remaining -= 30;


                if (frameCounter <= 0) {
                    
                    canAttack = false; // Stop attacking once smokeFrames is exhausted
                   
                }

                lastAttackTime = currentTime;
            }
            
            if (roundClear) {
                e->getComponent<CTransform>().velocity = { 0,0 };
                e->getComponent<CTransform>().position = e->getComponent<CTransform>().prevPosition;
                

            }



            /*
            if (!input.attack) {
                canAttack = true;
            }
            //std::cout << transform.facing.x << std::endl;

            if (input.attack && canAttack && (currentTime - lastAttackTime >= ATTACK_COOLDOWN) && state.state != "ATTACK"){
              
                gameEngine->playSound("LINKSWING");
                gameEngine->playSound("LINKYELP");
                state.state = "ATTACK";
                canAttack = false;
                frameCounter = 10;
                lastAttackTime = currentTime;
                //spawnSword();
                spawnSmoke();
                         
            }
            */

            /*
            if (state.state == "ATTACK") {
                if (frameCounter > 0) {
                    frameCounter--;
                    transform.velocity = { 0.0, 0.0 };
                }
                else {
                    state.state = "STANDD";
                }

            }
            */


        }
        //Update position here after velocity has been adjusted
        transform.position.x += transform.velocity.x;
        transform.position.y += transform.velocity.y;



        
        
        if (e->hasComponent<CLifespan>()) {
            auto& lifespan = e->getComponent<CLifespan>();
            lifespan.remaining--;
            //std::cout << e->getID() << std::endl;
            if (e->getID() == "BANG" && lifespan.remaining <= 0) {
                e->destroy();
                
                RoundOver();
                if (gameLives == 0) {
                    
                    //spawnGameOver();
                    gameEngine->stopMusic("GAMEPLAYMUSIC");
                    gameEngine->changeScene("GAMEOVER", std::make_shared<SceneGameOver>(gameEngine, score, levelPath));
                   
                }
            }
            else if (e->getID() == "PLAYER" && lifespan.remaining <= 0 && !roundClear) {
                gameEngine->stopMusic("GAMEPLAYMUSIC");
                //renderGameOverTransition(1);
                gameEngine->changeScene("GAMEOVER", std::make_shared<SceneGameOver>(gameEngine, score, levelPath));
                gameEngine->playMusic("MENUMUSIC");
            }
            

            else if (lifespan.remaining <= 0) {
                e->destroy();
            }


        }
        //I am going to update iframes here too
        if (e->hasComponent<CInvincibility>()) {
            auto& invincibility = player->getComponent<CInvincibility>();
            //std::cout << invincibility.iframes << std::endl;
            if (invincibility.iframes > 0) {
                invincibility.iframes--;
            }
            else {
                
                e->removeComponent<CInvincibility>();
            }
        }

    }
   
}

/**
 * Collision System
 * 
 * Checks for any collisions between DYNAMIC objects and any other objects
 * 
 */
void ScenePlay::sCollision(){
    //TODO: Implement the collision logic
    //Make sure you deal with:
    //  1. Player-Tile Collisions
    //  2. Player-Enemy Collisions
    //  3. Sword-Enemy Collisions
    //  3. Sword-Player Collisions (if any)

    static bool swordHit;
    static bool playerHit;

    static float swordCooldownTimer = 0.0f;
    const float swordCooldownDuration = 0.5f; 

    

    float currentTime = GetTime();

    
  


    for (auto& de : entityManager.getEntities("DYNAMIC")) { //Looping over dynamic entities


      

        for (auto& e : entityManager.getEntities()) {

            if (de == e) continue; //skip yourself
            if (e->getTag() == "DEC") continue; //skip decoration

            
            if ((de->getID() == "SWORD" && e->getID() == "PLAYER") ||
                (de->getID() == "PLAYER" && e->getID() == "SWORD")) {


                continue;
            }

            //Solution code exhibits the sword being able to spawn on tiles but not interact with them so I added this 
            //Does the same thing as above to skip sword player but skips sword Tile instead 
            if ((de->getID() == "SWORD" && e->getTag() == "TILE") ||
                (de->getTag() == "TILE" && e->getID() == "SWORD")) {


                continue;
            }

            if ((de->getID() == "BANG" && e->getID() == "PLAYER") ||
                (de->getID() == "PLAYER" && e->getID() == "BANG")) {


                continue;
            }


            if ((de->getID() == "SMOKE" && e->getID() == "PLAYER") ||
                (de->getID() == "PLAYER" && e->getID() == "SMOKE")) {


                continue;
            }

            if ((de->getID() == "SMOKE" && e->getID() == "SMOKE") ||
                (de->getID() == "SMOKE" && e->getID() == "SMOKE")) {


                continue;
            }
            //Enemy does not interact with speed tiles 
            if ((de->getID() == "ENEMY" && (e->getID() == "SPEEDUPTILEUP" || e->getID() == "SPEEDUPTILEDOWN")) ||
                ((de->getID() == "SPEEDUPTILEUP" || de->getID() == "SPEEDUPTILEDOWN") && e->getID() == "ENEMY")) {

                // Skip processing collision between enemy and speed tile
                continue;
            }

            // Collision check for enemies with any type of flag
            if ((de->getID() == "ENEMY" && (e->getID() == "SPECIALFLAG" || e->getID() == "LUCKYFLAG" || e->getID() == "FLAG")) ||
                ((de->getID() == "SPECIALFLAG" || de->getID() == "LUCKYFLAG" || de->getID() == "FLAG") && e->getID() == "ENEMY")) {
                // Skip processing collision between enemies and entities with flags
                continue;
            }



        


            
            

            




            Vec2 currentOverlap = Physics::getOverlap(de, e);


            if (currentOverlap.x > 0 && currentOverlap.y > 0) { //collision has occurred 

                Vec2 prevOverlap = Physics::getPreviousOverlap(de, e);

                Vec2 resolution(0, 0);

                //std::cout << "I am overlapping" << std::endl;


                //Player-flag collision
                if ((de->getID() == "PLAYER" &&
                    (e->getID() == "FLAG" || e->getID() == "LUCKYFLAG" || e->getID() == "SPECIALFLAG")) ||
                    ((de->getID() == "FLAG" || de->getID() == "LUCKYFLAG" || de->getID() == "SPECIALFLAG") &&
                        e->getID() == "PLAYER")) {

                    auto& player = (de->getID() == "PLAYER") ? de : e;

                    auto& flag = (de->getID() == "FLAG" || de->getID() == "LUCKYFLAG" || de->getID() == "SPECIALFLAG") ? de : e;

                    
                    if (flag->getID() == "FLAG") {
                        gameEngine->playSound("FLAG");
                        score += 200;
                    }
                    else if (flag->getID() == "SPECIALFLAG") {
                        gameEngine->playSound("SPECIALFLAG");

                        if (score == 0) {
                            score += 500;
                        }
                        score = score * 2;
                    }


                    flag->destroy();        //Flags dissapear just need to implement score collection now 
                    std::cout << flags << std::endl;
                    flags--;

                    if (flags == 0) {
                        roundClear=true;

                        gameEngine->stopMusic("GAMEPLAYMUSIC");
                        gameEngine->playSound("ROUNDCLEAR");
                        /*
                        float depletionRate = 1.0f;
                        
                            e->getComponent<CLifespan>().remaining -= depletionRate;
                            score += (int)(e->getComponent<CLifespan>().remaining);
                            */
                           
                        
                        //renderWinTransition(5);
                        //gameEngine->changeScene("MENU", std::make_shared<SceneMenu>(gameEngine));
                    }

                    continue;

                }


               //Player-rock collision
                if ((de->getID() == "PLAYER" &&
                    (e->getID() == "ROCK1" || e->getID() == "ROCK2")) ||
                    ((de->getID() == "ROCK1" || de->getID() == "ROCK2") &&
                        e->getID() == "PLAYER")) {

                    auto& player = (de->getID() == "PLAYER") ? de : e;
                    auto& rock = (de->getID() == "ROCK1" || de->getID() == "ROCK2") ? de : e;


                    //spawn bang
                    spawnBang();

                    gameEngine->playSound("CARCRASH");
                    
                    //animationComponent.animation = gameEngine->getAssets().getAnimation("RALLYXPLAYERUP");
                    player->destroy();
                    gameEngine->stopMusic("GAMEPLAYMUSIC");
                    gameLives--;
                    
                    
                 

                    continue ;
                }

                
                //Enemy Wall collision allow sliding 
                if (de->getID()=="ENEMY" && e->getTag()=="TILE") {

                    
                  
                    
                    
                }

                if (de->getID() == "PLAYER" && e->getTag() == "TILE") {





                }


                //Enemy smokeScreen collision
                
                if ((de->getID() == "ENEMY" && e->getID() == "SMOKE") ||
                    (de->getID() == "SMOKE" && e->getID() == "ENEMY")) {

                    auto& enemy = (de->getID() == "ENEMY") ? de : e;
                    auto& smoke = (de->getID() == "SMOKE") ? de : e;
                    std::cout << "Enemy hitting smoke" << std::endl;

                    enemy->getComponent<CTransform>().velocity = { 0,0 };
                    //enemy->getComponent<CTransform>().facing = { 1,0 };

                    //Set state to "InSmoke"
                    enemy->getComponent<CState>().state = "InSmoke";

                    

                    
                }

                //Enemy smokeScreen collision

                if ((de->getID() == "ENEMY" && e->getTag() == "SMOKE") ||
                    (de->getID() == "SMOKE" && e->getID() == "ENEMY")) {

                    auto& enemy = (de->getID() == "ENEMY") ? de : e;
                    auto& smoke = (de->getID() == "SMOKE") ? de : e;
                    std::cout << "Enemy hitting smoke" << std::endl;

                    enemy->getComponent<CTransform>().velocity = { 0,0 };


                    //Set state to "InSmoke"
                    enemy->getComponent<CState>().state = "InSmoke";




                }
               
                //Player Speedtile collision

                if ((de->getID() == "PLAYER" && (e->getID() == "SPEEDUPTILEUP" || e->getID() == "SPEEDUPTILEDOWN")) ||
                    ((de->getID() == "SPEEDUPTILEUP" || de->getID() == "SPEEDUPTILEDOWN") && e->getID() == "PLAYER")) {

                    auto& playerTransform = player->getComponent<CTransform>();
                    std::cout << "Player hitting SpeedUpTile" << std::endl;

                    float SPEED_INCREMENT = 1.2f;

                    // Start or reset the speed-up effect
                    playerTransform.velocity.x += SPEED_INCREMENT * playerTransform.facing.x;
                    playerTransform.velocity.y += SPEED_INCREMENT * playerTransform.facing.y;

                    continue;
                }
               
                


               
                //Player-enemy collision 
                if ((de->getID() == "PLAYER" && e->getID() == "ENEMY") ||
                    (de->getID() == "ENEMY" && e->getID() == "PLAYER")) {

                    auto& player = (de->getID() == "PLAYER") ? de : e;

                    // Determine which entity is the enemy
                    auto& enemy = (de->getID() == "ENEMY") ? de : e;

                    auto& playerHealth = player->getComponent<CHealth>();
                    

                    

                    
                    if (!playerHit) {
                        
                        gameEngine->playSound("CARCRASH");  //play car crash sound
                        
                        
                        spawnBang();
                        player->destroy();
                        enemy->destroy();
                        gameEngine->stopMusic("GAMEPLAYMUSIC");


                        playerHit = true;
                        gameLives--;
                        
                        //Respawn player at starting point and respawn enemies too
                        //Round over is called at the same time spawnbang is destroyed so that it lingers on the screen (in lifespan section of movement)



                       
                        

                       
                       
                    }
                    
                    
                    if (player->getComponent<CInvincibility>().iframes == 0) {
                        //std::cout << "Entered " << std::endl;
                        playerHit = false;
                    }
                    
                   
                    //continue;
                }

                // Sword-enemy collision
                if ((de->getID() == "SWORD" && e->getID() == "ENEMY") ||
                    (de->getID() == "ENEMY" && e->getID() == "SWORD")) {

                    //Determine which entity is the sword
                    auto& sword = (de->getID() == "SWORD") ? de : e;

                    // Determine which entity is the enemy
                    auto& enemy = (de->getID() == "ENEMY") ? de : e;

                    auto& enemyHealth = enemy->getComponent<CHealth>();


                   


                    if (!swordHit) {
                        gameEngine->playSound("ENEMYHIT");
                        enemyHealth.current -= sword->getComponent<CDamage>().damage; // Or whatever damage amount you want

                    if (enemyHealth.current == 0) {
                        gameEngine->playSound("ENEMYKILL");
                        enemy->destroy();
                    }


                    frameCounter = 10;      
                    swordHit = true;

                   
                    }

                    
                    if (frameCounter == 0) {
                        swordHit = false;
                    }

                   
                    
                    continue; // Move to the next entity pair
                
                }   //End of sword enemy collision
               
                
                


                
                if (prevOverlap.y > 0) {
                    //de on the left of e 
                    if (de->getComponent<CTransform>().prevPosition.x < e->getComponent<CTransform>().prevPosition.x) {

                       
                        de->getComponent<CTransform>().velocity.x = 0;

                        de->getComponent<CTransform>().position.x -= currentOverlap.x;
                        

                       
                        
                        
                        
                        



                    }

                    //de is on the right of e
                    else {
                        de->getComponent<CTransform>().velocity.x = 0;
                        //std::cout << "Collision happening" << std::endl;
                        de->getComponent<CTransform>().position.x += currentOverlap.x;

                    }

                }
                else if (prevOverlap.x > 0) {
                    
                    //de is above e
                    if (de->getComponent<CTransform>().prevPosition.y < e->getComponent<CTransform>().prevPosition.y) {
                        de->getComponent<CTransform>().velocity.y = 0;
                        de->getComponent<CTransform>().position.y -= currentOverlap.y;

                    }
                    //de is below e
                    else {
                        de->getComponent<CTransform>().velocity.y = 0;
                        de->getComponent<CTransform>().position.y += currentOverlap.y; 
                    }


                }










            }
            



            }




        }



    
    
    
    }



    void ScenePlay::stopAllEntities() {
        
    for (auto& e : entityManager.getEntities("DYNAMIC")) {
        e->getComponent<CTransform>().velocity = { 0, 0 };
        e->getComponent<CTransform>().position = e->getComponent<CTransform>().prevPosition;
    }

    }
    void ScenePlay::renderStartTransition(float transitionDuration) {
        // Load sprites and store them in variables
        Texture2D rockSprite = LoadTexture("assets/textures/Rock1.png");
        Texture2D enemySprite = LoadTexture("assets/textures/RallyXEnemyRight.png");
        Texture2D flag1Sprite = LoadTexture("assets/textures/Flag.png");
        Texture2D flag2Sprite = LoadTexture("assets/textures/SpecialFlag.png");
        Texture2D flag3Sprite = LoadTexture("assets/textures/LuckyFlag.png");
        Texture2D smokeInstructionSprite = LoadTexture("assets/textures/Smoke.png"); // Load the smoke instruction sprite
        Texture2D playerSprite = LoadTexture("assets/textures/RallyXPlayer.png");

        float scale = 5.0f; // Scaling factor
        float elapsedTime = 0.0f;
        while (elapsedTime < transitionDuration) {
            BeginDrawing();
            ClearBackground(GRAY);

            const Font& font = gameEngine->getAssets().getFont("arcade");
            int screenWidth = GetScreenWidth();
            int screenHeight = GetScreenHeight();

            // Draw "Avoid These" at the top left
            std::string avoidText = "Avoid These";
            int avoidTextWidth = MeasureTextEx(font, avoidText.c_str(), 40, 1).x;
            int avoidTextPosX = 50;
            int avoidTextPosY = 50;
            DrawTextEx(font, avoidText.c_str(), Vector2(avoidTextPosX, avoidTextPosY), 40, 1, WHITE);

            // Draw rock sprite to the right of "Avoid These"
            int spriteSpacing = 20;
            DrawTextureEx(rockSprite,
                Vector2(avoidTextPosX + avoidTextWidth + spriteSpacing, avoidTextPosY),
                0.0f, scale, WHITE);

            // Draw enemy sprite to the right of the rock sprite
            DrawTextureEx(enemySprite,
                Vector2(avoidTextPosX + avoidTextWidth + spriteSpacing * 2 + rockSprite.width * scale, avoidTextPosY + 40),
                0.0f, scale, WHITE);

            // Draw "Collect These" below "Avoid These"
            std::string collectText = "Collect These";
            int collectTextWidth = MeasureTextEx(font, collectText.c_str(), 40, 1).x;
            int collectTextPosX = avoidTextPosX;
            int collectTextPosY = avoidTextPosY + rockSprite.height * scale + 100;
            DrawTextEx(font, collectText.c_str(), Vector2(collectTextPosX, collectTextPosY), 40, 1, WHITE);

            // Draw flag sprites to the right of "Collect These"
            int flag1PosX = collectTextPosX + collectTextWidth + spriteSpacing;
            int flag1PosY = collectTextPosY;
            DrawTextureEx(flag1Sprite,
                Vector2(flag1PosX, flag1PosY),
                0.0f, scale, WHITE);

            int flag2PosX = flag1PosX + flag1Sprite.width * scale + spriteSpacing;
            DrawTextureEx(flag2Sprite,
                Vector2(flag2PosX, flag1PosY),
                0.0f, scale, WHITE);

            int flag3PosX = flag2PosX + flag2Sprite.width * scale + spriteSpacing;
            DrawTextureEx(flag3Sprite,
                Vector2(flag3PosX, flag1PosY),
                0.0f, scale, WHITE);

            // Draw "Before Fuel Runs Out!" at the bottom
            std::string fuelText = "Before Fuel Runs Out!";
            int fuelTextWidth = MeasureTextEx(font, fuelText.c_str(), 48, 1).x;
            int fuelTextPosX = (screenWidth - fuelTextWidth) / 2;
            int fuelTextPosY = screenHeight - 150;
            DrawTextEx(font, fuelText.c_str(), Vector2(fuelTextPosX, fuelTextPosY), 48, 1, WHITE);

            // Draw WASD controls at the middle right of the screen
            int controlSquareSize = 64;
            int controlSpacing = 10;
            int controlRightMargin = 50; // Margin from the right edge

            int controlsPosX = screenWidth - controlSquareSize - controlRightMargin - 80;
            int controlsPosY = (screenHeight - controlSquareSize) / 2 - 50;

            
            // Draw control squares and text with the specified font
            //DrawRectangle(controlsPosX, controlsPosY, controlSquareSize, controlSquareSize, WHITE); // Center square
            DrawTextureEx(playerSprite, Vector2(controlsPosX+5, controlsPosY-15), 0.0f, scale, WHITE);
            
            DrawTextEx(font, "W", Vector2(controlsPosX + 24, controlsPosY - controlSquareSize - controlSpacing), 48, 1, WHITE);
            //DrawRectangle(controlsPosX, controlsPosY - controlSquareSize - controlSpacing, controlSquareSize, controlSquareSize, WHITE); // Top square

            DrawTextEx(font, "S", Vector2(controlsPosX + 24, controlsPosY + controlSquareSize + controlSpacing), 48, 1, WHITE);
            //DrawRectangle(controlsPosX, controlsPosY + controlSquareSize + controlSpacing, controlSquareSize, controlSquareSize, WHITE); // Bottom square

            DrawTextEx(font, "A", Vector2(controlsPosX - controlSquareSize - controlSpacing + 24, controlsPosY), 48, 1, WHITE);
            //DrawRectangle(controlsPosX - controlSquareSize - controlSpacing, controlsPosY, controlSquareSize, controlSquareSize, WHITE); // Left square

            DrawTextEx(font, "D", Vector2(controlsPosX + controlSquareSize + controlSpacing + 24, controlsPosY), 48, 1, WHITE);
            //DrawRectangle(controlsPosX + controlSquareSize + controlSpacing, controlsPosY, controlSquareSize, controlSquareSize, WHITE); // Right squareze, controlSquareSize, WHITE); // Right square

            //Draw "Controls" text
            // Draw "Before Fuel Runs Out!" at the bottom
            std::string controls = "Controls";
            int controlWidth = MeasureTextEx(font, controls.c_str(), 48, 1).x;
 
            DrawTextEx(font, controls.c_str(), Vector2(controlsPosX- 200, controlsPosY- 180), 48, 1, WHITE);


            // Draw "Press Space to use Smoke" instruction text and sprite at the bottom of the screen
            std::string smokeInstructionText = "Press Space to use";
            int smokeInstructionTextWidth = MeasureTextEx(font, smokeInstructionText.c_str(), 40, 1).x;
            int smokeInstructionTextPosX = 50;
            int smokeInstructionTextPosY = avoidTextPosY + rockSprite.height * scale + 300;;
            DrawTextEx(font, smokeInstructionText.c_str(), Vector2(smokeInstructionTextPosX, smokeInstructionTextPosY), 40, 1, WHITE);

            int smokeInstructionWidth = smokeInstructionSprite.width * scale;
            int smokeInstructionHeight = smokeInstructionSprite.height * scale;
            int smokeInstructionSpritePosX = (screenWidth - smokeInstructionWidth) / 2 + 220;
            int smokeInstructionSpritePosY = smokeInstructionTextPosY - 90; // Position below the text
            DrawTextureEx(smokeInstructionSprite,
                Vector2(smokeInstructionSpritePosX, smokeInstructionSpritePosY),
                0.0f, scale, WHITE);

            // Draw transition message
            std::string message = "Starting game...";
            int textWidth = MeasureTextEx(font, message.c_str(), 64, 1).x;
            int textPosX = (screenWidth - textWidth) / 2;
            int textPosY = screenHeight - 80;
            DrawTextEx(font, message.c_str(), Vector2(textPosX, textPosY), 64, 1, WHITE);

            EndDrawing();

            elapsedTime += GetFrameTime();
        }

        // Unload sprites
        UnloadTexture(rockSprite);
        UnloadTexture(enemySprite);
        UnloadTexture(flag1Sprite);
        UnloadTexture(flag2Sprite);
        UnloadTexture(flag3Sprite);
        UnloadTexture(smokeInstructionSprite); // Unload the smoke instruction sprite
        UnloadTexture(playerSprite);
    }
    void ScenePlay::renderGameOverTransition(float transitionDuration) {
        float elapsedTime = 0.0f;
        while (elapsedTime < transitionDuration) {
            BeginDrawing();
            ClearBackground(BLACK);


            std::string message = "Game Over!";

            const Font& font = gameEngine->getAssets().getFont("arcade");
            // Draw the "Game Over" text in the center of the screen
            int textWidth = MeasureTextEx(font, message.c_str(), 64, 1).x; // Get the width of the text
            int screenWidth = GetScreenWidth();
            int screenHeight = GetScreenHeight();
            int textPosX = (screenWidth - textWidth) / 2;
            int textPosY = screenHeight / 2 - 32; // Adjust for text height

            DrawTextEx(font, message.c_str(), Vector2(textPosX, textPosY), 64, 1, WHITE);


            // Calculate fade effect
            //float alpha = (elapsedTime / transitionDuration) * 255;
            //DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(fadeColor, alpha / 255.0f));

            EndDrawing();


            elapsedTime += GetFrameTime();
        }
    }

    void ScenePlay::renderWinTransition(float transitionDuration) {
        float elapsedTime = 0.0f;
        while (elapsedTime < transitionDuration) {
            BeginDrawing();
            ClearBackground(BLACK);

            highscore = score; // Update highscore
            std::string message = "Course Complete!";
            std::string highscoreMessage = "Score: " + std::to_string(highscore);

            const Font& font = gameEngine->getAssets().getFont("arcade");

            // Draw the "Course Complete!" text in the center of the screen
            int textWidth = MeasureTextEx(font, message.c_str(), 64, 1).x; // Get the width of the text
            int screenWidth = GetScreenWidth();
            int screenHeight = GetScreenHeight();
            int textPosX = (screenWidth - textWidth) / 2;
            int textPosY = screenHeight / 2 - 32; // Adjust for text height

            DrawTextEx(font, message.c_str(), Vector2(textPosX, textPosY), 64, 1, WHITE);

            // Draw the high score text below the "Course Complete!" text
            int highscoreTextWidth = MeasureTextEx(font, highscoreMessage.c_str(), 48, 1).x; // Get the width of the high score text
            int highscoreTextPosX = (screenWidth - highscoreTextWidth) / 2;
            int highscoreTextPosY = textPosY + 80; // Position below the "Course Complete!" text

            DrawTextEx(font, highscoreMessage.c_str(), Vector2(highscoreTextPosX, highscoreTextPosY), 48, 1, WHITE);

            // Calculate fade effect
            //float alpha = (elapsedTime / transitionDuration) * 255;
            //DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(fadeColor, alpha / 255.0f));

            EndDrawing();

            elapsedTime += GetFrameTime();
        }
    }


    void ScenePlay::renderSideBar() {

        const Font& font = gameEngine->getAssets().getFont("arcade");

        // Define sidebar dimensions and position
        auto& playerPos = player->getComponent<CTransform>().position;
        auto& playerSize = player->getComponent<CAnimation>().animation.getScaledSize();

        int sidebarWidth = gameEngine->getWidth() / 4;
        int sidebarHeight = gameEngine->getHeight();
        int sidebarY = playerPos.y- (gameEngine->getHeight()/2)+(playerSize.y/2);
        int sidebarX = playerPos.x + (gameEngine->getWidth()/2)-sidebarWidth+(playerSize.x/2);

        // Draw sidebar background
        DrawRectangle(sidebarX, sidebarY, sidebarWidth, sidebarHeight, BLACK);


        //Begin MINIMAP code
        int minimapWidth = sidebarWidth;
        int minimapHeight = sidebarHeight*.55;

        int miniMapY = sidebarY+250;
        int miniMapX = sidebarX;

        DrawRectangle(miniMapX, miniMapY, minimapWidth, minimapHeight, BLUE);

        // Define the area of the gameboard to display in the minimap
        float minimapAreaX = 96 - 32;  
        float minimapAreaY = -1440 - 32;  
        float minimapAreaWidth = 40*64;  // Width of the gameboard area to display
        float minimapAreaHeight = 36*64;  // Height of the gameboard area to display

        // Calculate scaling factors
        float scaleX = minimapWidth / minimapAreaWidth;
        float scaleY = minimapHeight / minimapAreaHeight;

       

        // Render player on minimap
        auto& playerTransform = player->getComponent<CTransform>();
        Vec2 playerMinimapPos = {
            miniMapX + (playerTransform.position.x - minimapAreaX) * scaleX,
            miniMapY + (playerTransform.position.y - minimapAreaY) * scaleY
        };
        DrawCircle(playerMinimapPos.x, playerMinimapPos.y, 4, DARKBLUE);

        // Render enemies on minimap
        for (auto& e : entityManager.getEntities("DYNAMIC")) {
            if (e->getID() == "ENEMY") {
                auto& enemyTransform = e->getComponent<CTransform>();
                Vec2 enemyMinimapPos = {
                    miniMapX + (enemyTransform.position.x - minimapAreaX) * scaleX,
                    miniMapY + (enemyTransform.position.y - minimapAreaY) * scaleY
                };
                DrawCircle(enemyMinimapPos.x, enemyMinimapPos.y, 4, RED);
            }
        }

        // Render flags on minimap
        for (auto& e : entityManager.getEntities("TILE")) {
            if (e->getID() == "FLAG"||e->getID()== "SPECIALFLAG"|| e->getID()=="LUCKYFLAG") {
                auto& flagTransform = e->getComponent<CTransform>();
                Vec2 flagMiniPos = {
                    miniMapX + (flagTransform.position.x - minimapAreaX)* scaleX,
                    miniMapY + (flagTransform.position.y - minimapAreaY) * scaleY
                };
                if (e->getID() == "SPECIALFLAG") {
                    DrawCircle(flagMiniPos.x, flagMiniPos.y, 4, ORANGE);
                }
                else if (e->getID() == "LUCKYFLAG") {
                    DrawCircle(flagMiniPos.x, flagMiniPos.y, 4, WHITE);
                }
                else {  //normal flag
                    DrawCircle(flagMiniPos.x, flagMiniPos.y, 4, YELLOW);
                   

                }
            }
        }
        

        //END MINIMAP CODE



        // Render the player's lives on the sidebar


        Texture2D lifeSprite = gameEngine->getAssets().getTexture("LIFESPRITE"); // Assuming the sprite is called "lifeSprite"
        int size = 64;
        lifeSprite.width = 64;
        lifeSprite.height = 64;

        int livesStartY = sidebarY + GetScreenHeight() - 60; // Y position for lives display
        int livesStartX = sidebarX- 20; // Starting X position for lives display
        int livesSpacing = 10; // Spacing between life icons

        for (int i = 1; i <= gameLives; i++) {
            DrawTexture(lifeSprite, livesStartX + (i * (size + livesSpacing)), livesStartY, WHITE);
        }//End render player lives


        // Render the fuel gauge
        std::string fuel = "Fuel";
        
        

        int fuelTextWidth = MeasureTextEx(font, fuel.c_str(), 64, 1).x; // Get the width of the text
        int fuelTextPosX = sidebarX + 100;
        int fuelTextPosY = sidebarY + 170;
        DrawTextEx(font, fuel.c_str(), Vector2(fuelTextPosX, fuelTextPosY), 32, 1, WHITE);


        auto& playerLifespan = player->getComponent<CLifespan>();
        float fuelPercentage = static_cast<float>(playerLifespan.remaining) / static_cast<float>(playerLifespan.total);

        int gaugeWidth = sidebarWidth ; // Width of the fuel gauge
        int gaugeHeight = 20; // Fixed height of the fuel gauge
        int gaugeX = sidebarX ; // Position X of the gauge
        int gaugeY = sidebarY + 210; // Position Y of the gauge (near the bottom of the sidebar)

        // Draw the background of the fuel gauge
        DrawRectangle(gaugeX, gaugeY, gaugeWidth, gaugeHeight, GRAY);

        // Draw the current fuel level
        int fuelWidth = static_cast<int>(gaugeWidth * fuelPercentage); // Width based on fuel percentage

        int fuelX = gaugeX + (gaugeWidth - fuelWidth);

        DrawRectangle(fuelX, gaugeY, fuelWidth, gaugeHeight, YELLOW); // The fuel level

        

        // Render the high score area
        int highScoreY = sidebarY + 50; // Position Y of the high score area
        int highScoreX = sidebarX + 20; // Position X of the high score area

        //std::string highScoreMessage = "High Score: " + std::to_string(gameEngine->getHighScore());
        //DrawText(highScoreMessage.c_str(), highScoreX, highScoreY, 32, YELLOW);


        int scoreY = sidebarY + 50; // Position Y of the score area
        int scoreX = sidebarX + 20; // Position X of the score area

        std::string scoreMessage = "Score: " + std::to_string(score);
        DrawText(scoreMessage.c_str(), scoreX, scoreY, 32, PINK);




        //Course Number 
        std::string course = "Course: " + courseNumber;
        

        
        // Draw the "Game Over" text in the center of the screen
        int textWidth = MeasureTextEx(font, course.c_str(), 64, 1).x; // Get the width of the text
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();
        int textPosX = sidebarX;
        int textPosY = sidebarY + GetScreenHeight() - 80;
        DrawTextEx(font, course.c_str(), Vector2(textPosX, textPosY), 32, 1, WHITE);
        //End Course number




    }


/**
 * Render System
 * 
 * Renders the textures, using the camera system
 * 
 */
void ScenePlay::sRender(){
    BeginDrawing();
    BeginMode2D(mainCamera);
    ClearBackground(Color(252,216,168,255));

    //********** Raylib Drawing Content **********
        if(renderTextures){
            renderTex();
            renderSideBar();
            if(renderHealth)
                renderHealthBar();
        }
        if(renderBoundingBox)
            renderBB();
        if(renderGridLines)
            renderGrid();
        if(renderVisionDebug){
            renderAIDebug();
        }

    EndMode2D();
        //********** ImGUI Content *********
        sGUI();

    EndDrawing();
}

/**
 * ImGUI System
 * 
 * Renders the ImGUI
 * 
 */
void ScenePlay::sGUI() {


    if(renderDebug){
    rlImGuiBegin();

    ImGui::SetNextWindowSize(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::Begin("RallyX 0.0001", NULL, ImGuiWindowFlags_NoResize);
    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
    {
        if (ImGui::BeginTabItem("Controls"))
        {
            ImGui::SeparatorText("Rendering Controls");
            ImGui::Checkbox("Textures", &renderTextures);
            ImGui::Checkbox("Health Bar", &renderHealth);
            ImGui::Checkbox("Bounding Boxes", &renderBoundingBox);
            ImGui::Checkbox("Vision Debug", &renderVisionDebug);
            ImGui::Checkbox("Grid", &renderGridLines);
            ImGui::SeparatorText("Camera Controls");
            //ImGui::Checkbox("Follow Camera",&followCam);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Entities"))
        {
            int count = 0;
            if (ImGui::TreeNode("By Tag")) {
                //TODO Add list of all entities by tag
                //See your Assignment 2
                //Sort the tree nodes by type and add a new tree node when a new entity is detected 
                for (const auto& [tag, entityList] : entityManager.getEntityMap()) {
                    if (ImGui::TreeNode(tag.c_str())) {
                        for (const auto& entity : entityList) {



                            //ImGui::ColorButton("Color", entityColor, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop, ImVec2(20, 20));

                            ImGui::PushID(entity->getID().c_str());

                            if (ImGui::Button("D")) {
                                /*
                                if (entity->getID() == "PLAYER") {      //game resets when player is deleted
                                    reload=true;
                                }
                                else */
                                entity->destroy(); // Call the destroy method for the entity
                            }

                            ImGui::PopID();
                            ImGui::SameLine();
                            ImGui::Text("%s %s", entity->getTag().c_str(), entity->getID().c_str());

                            if (entity->hasComponent<CTransform>()) {
                                ImGui::SameLine();
                                const auto& transform = entity->getComponent<CTransform>();
                                ImGui::Text("Pos: (%.2f, %.2f)", transform.position.x, transform.position.y);
                            }

                        }
                        ImGui::TreePop();



                    }



                }


                ImGui::TreePop();
            }
            if (ImGui::TreeNode("All Entitites")) {
                //TODO Add list of all entities
                //See your Assignment 2
                for (const auto& entity : entityManager.getEntities()) {




                    ImGui::PushID(entity->getID().c_str());

                    if (ImGui::Button("D")) {

                        if (entity->getTag() == "PLAYER") {
                            reload = true;
                        }
                        else
                            entity->destroy(); // Call the destroy method for the entity
                    }

                    ImGui::PopID();
                    ImGui::SameLine();
                    ImGui::Text("%s %s", entity->getTag().c_str(), entity->getID().c_str());
                    ImGui::SameLine();
                    if (entity->hasComponent<CTransform>()) {
                        ImGui::SameLine();
                        const auto& transform = entity->getComponent<CTransform>();
                        ImGui::Text("Pos: (%.2f, %.2f)", transform.position.x, transform.position.y);
                    }
                }

                ImGui::TreePop();
            }
            ImGui::EndTabItem();
        }
    }
    ImGui::EndTabBar();
    ImGui::End();
    rlImGuiEnd();
}
}

/**
 * Render entity health bars
 */
void ScenePlay::renderHealthBar(){
    for(auto& e : entityManager.getEntities("DYNAMIC")){
        if(e->hasComponent<CHealth>()){
            CHealth health=e->getComponent<CHealth>();
            Vec2 position=e->getComponent<CTransform>().position;
            Vec2 size=e->getComponent<CAnimation>().animation.getScaledSize();
            float yPos=position.y-size.y/2-15;
            float xPos=position.x-size.x/2;
            float width=size.x/health.max;
            float shift=0;
            Color c = RED;
            for(int i=0;i<health.max;i++){
                if(i>=health.current) c=BLACK;
                DrawRectangle(xPos+shift, yPos+1, width, 6, c);
                DrawRectangleLines(xPos+shift,yPos,width, 8,BLACK);
                shift+=width-1;
            }
        }
    }
}

/**
 * Render AI Debug information, including vision and patrol paths
 */
void ScenePlay::renderAIDebug(){
    for(auto& e : entityManager.getEntities("DYNAMIC")){
        if(e->hasComponent<CPatrol>()){
            auto patrol=e->getComponent<CPatrol>();
            Vec2 point;
            Vec2 nextPoint;
            for(int i=0;i<patrol.positions.size();i++){
                point=patrol.positions[i];
                int nextIndex=(i+1)%patrol.positions.size();
                nextPoint=patrol.positions[nextIndex];
                DrawLineEx(Vector2(point.x,point.y), Vector2(nextPoint.x,nextPoint.y),2, BLUE);
                DrawCircle(point.x,point.y,7,BLUE);
            }
            
        }
        if(e->hasComponent<CFollowPlayer>()){
            Vec2& home = e->getComponent<CFollowPlayer>().home;
            auto& position=e->getComponent<CTransform>().position;
            DrawLineEx(Vector2(position.x,position.y), Vector2(home.x,home.y),2, BLUE);

        }
    }
}

/**
 * Do Action System
 * 
 * Updates the player input and game state based on PRESS or RELEASE actions
 * 
 * @param action action sent from the gameEngine that contains type and name
 */
void ScenePlay::sDoAction(const Action& action){

    //TODO: Add in the results of the additional actions
    //that you register in the init() function: LEFT, RIGHT, UP, DOWN, and ATTACK
    //also add in what happens when you RELEASE the button (if anything)

    CInput& input = player->getComponent<CInput>();
    CState& state = player->getComponent<CState>();
    if((action.getType()=="PRESS")){

        if(action.getName()=="BB"){
            renderBoundingBox=!renderBoundingBox;
        }
        if(action.getName()=="GRID"){
            renderGridLines=!renderGridLines;
        }
        if(action.getName()=="TEX"){
            renderTextures=!renderTextures;
        }
        if(action.getName()=="HEALTH"){
            renderHealth=!renderHealth;
        }
        if(action.getName()=="VISION"){
            renderVisionDebug=!renderVisionDebug;
        }
        if(action.getName()=="QUIT"){
            gameEngine->changeScene("MENU",std::make_shared<SceneMenu>(gameEngine,highscore));
        }
        if(action.getName()=="RELOAD"){
            reload=true;
        }if (action.getName() == "DEBUG") {
            renderDebug = !renderDebug;
        }

        //Adding player actions
        if (action.getName() == "LEFT") {
            std::cout << "I am pressing LEFT" << std::endl;
            input.left = true;
        }
        if (action.getName() == "RIGHT") {
            std::cout << "I am pressing RIGHT" << std::endl;
            input.right = true;
        }
        if (action.getName() == "UP") {

            std::cout << "I am pressing UP" << std::endl;
            input.up = true;
            
        }
        if (action.getName() == "DOWN") {

            std::cout << "I am pressing DOWN" << std::endl;
            input.down = true;

        }
        if (action.getName() == "SMOKE") {
            std::cout << "Spawning smoke" << std::endl;
            input.attack = true;    //I will keep it as attack as that is technically the "attack" in this game 
            //frameCounter = 10;  //Edit this value to be the frame counter the sword should appear for 
        }
    }
    if((action.getType()=="RELEASE")){
        if (action.getName() == "LEFT") {
            input.left = false;
        }
        if (action.getName() == "RIGHT") {
            input.right = false;
        }
        if (action.getName() == "UP") {
            input.up = false;
        }
        if (action.getName() == "DOWN") {
            input.down = false;
        }
        if (action.getName() == "SMOKE") {
            input.attack = false;
        }



    }
}

/**
 * Camera System
 * 
 * Updates camera
 * 
 */
void ScenePlay::sCamera() {
    //TODO: Update the camera so that is shows the room that the player is in
    //Notes:
    //  1. The camera should be centered on the room (use the grid to help)
    //  2. when the player leaves the screen, the camera should "jump" to the correct room
    //  3. Also implement a follow camera that locks to the player


    //This initalizes camera for room 0,0 just make it dynamic   
    //First argument is the offset                                               (-1,0)
    //Second argument is the target, target should be center of each room (-1,0)  (0,0)  (1,0)
    //                                                                            (1,0)
    //mainCamera = Camera2D({ gameEngine->getWidth() / 2,gameEngine->getHeight() / 2 }, { gameEngine->getWidth() / 2,gameEngine->getHeight() / 2 }, 0, 1);


    /*
        if the room is (rx,ry), then the world pixel in the 
        center of any room is: (rx*w/2, ry*h/2), where w and h are the width and height of the 
        window in pixels.
    */
    int roomX, roomY;
    int roomHeightInTiles = 12;
    int roomWidthInTiles = 20;

    Vec2 playerPos = player->getComponent<CTransform>().position;


    // Calculate the current room coordinates, using floor to handle negative positions

    int roomWidthInPixels = roomWidthInTiles * gameEngine->getTileSizeX();
    int roomHeightInPixels = roomHeightInTiles * gameEngine->getTileSizeY();

    //Get the current room the player is in 
    int currentRoomX = static_cast<int>(std::floor(playerPos.x / roomWidthInPixels));
    int currentRoomY = static_cast<int>(std::floor(playerPos.y / roomHeightInPixels));


    //Get the center of the room that the player is in
    float roomCenterX = (currentRoomX + 0.5f) * roomWidthInPixels;
    float roomCenterY = (currentRoomY + 0.5f) * roomHeightInPixels;




    if (followCam) {
    mainCamera.target.x = playerPos.x+ (gameEngine->getTileSizeX()/2);
    mainCamera.target.y = playerPos.y + (gameEngine->getTileSizeY()/2);
    }
    else {
        // Center the camera on the current room
        mainCamera.target.x = roomCenterX;
        mainCamera.target.y = roomCenterY;
        
    

    }
    
   

   
}
/*


Using spawn coin from PA3 as reference
void ScenePlay::spawnCoin(const std::shared_ptr<Entity>& entity){
    //TODO: Spawn coin directly above the entity that triggered this function call (entity)
    Vec2 QTilePos = entity->getComponent<CTransform>().position;



    std::shared_ptr<Entity> coin;
    coin = entityManager.addEntity("DYNAMIC", "COIN");

    coin->addComponent<CAnimation>(gameEngine->getAssets().getAnimation("COIN"), true);
    float tileSizeY = gameEngine->getTileSizeY();


    coin->addComponent<CLifespan>(50);


    //coin->addComponent<CBoundingBox>(Vec2(scaledX, scaledY));
    //Vec2 pos = gridToMidPixel(playerConfig.GX, playerConfig.GY, coin);
    coin->addComponent<CTransform>(Vec2(QTilePos.x, QTilePos.y-tileSizeY), Vec2(0.0f, 0.0f), 0.0f);

}
*/




void ScenePlay::spawnBang() {

    Vec2 playerPos = player->getComponent<CTransform>().position;

    std::shared_ptr<Entity> bang;
    bang = entityManager.addEntity("DYNAMIC", "BANG");  //This is dynamic because it's life span is the way I linger on the dead screen and then restart
    bang->addComponent<CAnimation>(gameEngine->getAssets().getAnimation("CRASH"), true);

    bang->addComponent<CLifespan>(90);
    bang->addComponent<CTransform>(Vec2(playerPos.x, playerPos.y), Vec2(0.0f, 0.0f), 0.0f);


}

//This may not be necessary
void ScenePlay::spawnGameOver() {

    Vec2 playerPos = player->getComponent<CTransform>().position;

    std::shared_ptr<Entity> gameOver;
    gameOver = entityManager.addEntity("DYNAMIC", "GAMEOVER");  //This is dynamic because it's life span is the way I linger on the dead screen and then restart
    gameOver->addComponent<CAnimation>(gameEngine->getAssets().getAnimation("CRASH"), true);

    gameOver->addComponent<CLifespan>(60);
    gameOver->addComponent<CTransform>(Vec2(playerPos.x, playerPos.y), Vec2(0.0f, 0.0f), 0.0f);


}


void  ScenePlay::RoundOver() {
    // Reset player position
    gameEngine->playMusic("GAMEPLAYMUSIC");
    spawnPlayer();
   
      

    // Reset enemy positions  if an enemy dies it is no longer on the game board I kinda like this concept sacrificing lives for an easier game 
    int enemyIndex = 0;
    for (auto& e : entityManager.getEntities("DYNAMIC")) {
        if (e->getID() == "ENEMY") {
            e->getComponent<CTransform>().position = enemyStartPositions[enemyIndex];
            e->getComponent<CTransform>().prevPosition = enemyStartPositions[enemyIndex];

           
            e->getComponent<CTransform>().velocity = Vec2(0.0f, 0.0f);
            e->getComponent<CState>().state = "CHASING";  // Or the appropriate initial state

            ++enemyIndex;
        }
    }

}
//Take user to game over screen 


void ScenePlay::checkFlags() {
    for (auto& e : entityManager.getEntities("TILE")) {
        if (e->getID() == "FLAG" || e->getID() == "SPECIALFLAG"|| e->getID()=="LUCKYFLAG") {
            std::cout << flags << std::endl;
            flags++;
        }
    }


}


/**
 * Spawns Player
 * 
 * Example of spawning a simple player
 * 
 */
void ScenePlay::spawnPlayer(){
    
    player=entityManager.addEntity("DYNAMIC", "PLAYER");
    
    player->addComponent<CInput>();
    //player->addComponent<CHealth>(playerConfig.HEALTH,2);
    player->addComponent<CAnimation>(gameEngine->getAssets().getAnimation("RALLYXPLAYERUP"),true);
    float scaledX = gameEngine->getAssets().getAnimation("RALLYXPLAYERUP").getScaledSize().x;
    float scaledY = gameEngine->getAssets().getAnimation("RALLYXPLAYERUP").getScaledSize().y;
   
    float tileSizeX = gameEngine->getTileSizeX();
    float tileSizeY = gameEngine->getTileSizeY();
    float bboxSizeX = tileSizeX;
    float bboxSizeY = tileSizeY;
    if (scaledX > tileSizeX || scaledY > tileSizeY) {
        bboxSizeX = scaledX;
        bboxSizeY = scaledY;
    }


    player->addComponent<CBoundingBox>(Vec2(bboxSizeX,bboxSizeY));
    Vec2 pos = gridToMidPixel(playerConfig.X,playerConfig.Y,player);
    player->addComponent<CTransform>(Vec2(pos.x,pos.y), Vec2(0.0f,0.0f), 0.0f);
    player->addComponent<CLifespan>(fullTank+2);    //Weird hang I found to get the player to look up on respawn 
    
   

}

//Similar to spawn sword but spawning a smoke behind the player
void ScenePlay::spawnSmoke() {

    Vec2 playerPos = player->getComponent<CTransform>().position;
    Vec2 direction = player->getComponent<CTransform>().facing;  //we may need facing to ensure it spawned at the back of the player 

    std::shared_ptr<Entity> smoke;
    smoke = entityManager.addEntity("DYNAMIC", "SMOKE");
    smoke->addComponent<CAnimation>(gameEngine->getAssets().getAnimation("SMOKE"), true);
    smoke->addComponent<CLifespan>(40); // Adjust the lifespan as needed
    smoke->addComponent<CTransform>();

    // Get player size
    Vec2 playerSize = player->getComponent<CBoundingBox>().size;



    // Calculate the smoke's position
    Vec2 smokePos;

    if (direction.x > 0) { // Facing right
        smokePos.x = playerPos.x - playerSize.x; // Spawn smoke to the left of the player
        smokePos.y = playerPos.y;
    }
    else if (direction.x < 0) { // Facing left
        smokePos.x = playerPos.x + playerSize.x; // Spawn smoke to the right of the player
        smokePos.y = playerPos.y;
    }
    else if (direction.y < 0) { // Facing up
        smokePos.x = playerPos.x;
        smokePos.y = playerPos.y + playerSize.y; // Spawn smoke below the player
    }
    else { // Facing down
        smokePos.x = playerPos.x;
        smokePos.y = playerPos.y - playerSize.y; // Spawn smoke above the player
    }

    // Add a transform component to the smoke entity with the calculated position
    smoke->addComponent<CTransform>(smokePos, Vec2(0.0f, 0.0f), 0.0f); // No rotation required
   
    float tileSizeX = gameEngine->getTileSizeX();
    float tileSizeY = gameEngine->getTileSizeY();
    float bboxSizeX = tileSizeX;
    float bboxSizeY = tileSizeY;
    
    smoke->addComponent<CBoundingBox>(Vec2(bboxSizeX, bboxSizeY));

}



/**
 * Reloads this play scene
 */
void ScenePlay::reloadScene(){
    gameEngine->changeScene("PLAY",std::make_shared<ScenePlay>(gameEngine, levelPath));
}

/**
 * Gets the world position from room x,y and tile x,y
 * @param rx room x
 * @param ry room y
 * @param tx tile x
 * @param ty tile y
 * return Vec2 with the world position
 */
Vec2 ScenePlay::getPosition(int rx, int ry, int tx, int ty){
    //TODO: compute the world position using the current room and tile
    const int tileSize = 64;    //could also call getTileSize from gameEngine

    int roomWidthInTiles = 20;
    int roomHeightInTiles = 12;

    int globalTileX = rx * roomWidthInTiles + tx;
    int globalTileY = ry * roomHeightInTiles + ty;

    int worldPosX = globalTileX * tileSize ;

    //Place the entities at the bottom left of grid tile 

    //int roomTileY = roomHeightInTiles - ty;
    
    //int globalTileYFlipped = ry * roomHeightInTiles + roomTileY;
    int worldPosY = globalTileY * tileSize;
     
    return Vec2(worldPosX, worldPosY);
}

/**
 * Updates all systems and entityManager
 */
void ScenePlay::update(){



    entityManager.update();

    sMovement();
    sAnimation();
    sCollision();
    sMusic();
    sCamera();
    sRender();


    if (roundClear) {
        float depletionRate = 10.0f; // Adjust this rate as needed
        auto& playerLifespan = player->getComponent<CLifespan>();

        if (playerLifespan.remaining > 0) {
            // Deplete the fuel and add to the score simultaneously
            playerLifespan.remaining -= depletionRate;
            score += static_cast<int>(depletionRate);

            // Ensure remaining fuel doesn't go below zero
            if (playerLifespan.remaining < 0) {
                playerLifespan.remaining = 0;
            }
        }
        else {
            // Fuel is depleted, proceed to next state if needed
            renderWinTransition(4);
             gameEngine->changeScene("MENU", std::make_shared<SceneMenu>(gameEngine, highscore));
             
             
             gameEngine->playMusic("MENUMUSIC");
        }
    }

    if(reload==true){
        reloadScene();
    }
    
}