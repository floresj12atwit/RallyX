#include "sceneplay.hpp"
#include "scenemenu.hpp"
#include <fstream>
#include <iostream>
#include <cstdio>
#include <queue>
#include <vector>
#include <unordered_map>
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
    renderStartTransition(1);
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
    spawnPlayer();

    //TODO: Add actions for UP, DOWN, LEFT, RIGHT, and ATTACK
    registerAction(KEY_B,"BB");
    registerAction(KEY_G,"GRID");
    registerAction(KEY_T,"TEX");
    registerAction(KEY_H,"HEALTH");
    registerAction(KEY_V,"VISION");
    registerAction(KEY_ESCAPE,"QUIT");
    registerAction(KEY_R,"RELOAD");

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
                e->addComponent<CHealth>(Health,Health);
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
Animation LINKSTANDD LINKSTANDD 1 1 4
Animation LINKSTANDR LINKSTANDR 1 1 4
Animation LINKSTANDU LINKSTANDU 1 1 4
Animation LINKUSED LINKUSED 1 1 4
Animation LINKUSER LINKUSER 1 1 4
Animation LINKUSEU LINKUSEU 1 1 4
Animation LINKWALKD LINKWALKD 2 5 4
Animation LINKWALKR LINKWALKR 2 5 4
Animation LINKWALKU LINKWALKU 2 5 4



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


    //Going to handle enemy animations now need a loop because there's more than 1 
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

            auto& input = e->getComponent<CInput>();
            auto& state = e->getComponent<CState>();

            static Vec2 lastVelocity = { 0.0f, 0.0f };  // Remember the last valid velocity
            static Vec2 lastFacing = { 0, 0 };          // Remember the last facing direction
            
            if (std::abs(transform.velocity.x) > maxSpeed) {
                transform.velocity.x = (transform.velocity.x > 0) ? maxSpeed : -maxSpeed;
            }

            // Clamp velocity in the y-direction
            if (std::abs(transform.velocity.y) > maxSpeed) {
                transform.velocity.y = (transform.velocity.y > 0) ? maxSpeed : -maxSpeed;
            }
            
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

                if (frameCounter <= 0) {
                   
                    canAttack = false; // Stop attacking once smokeFrames is exhausted
                   
                }

                lastAttackTime = currentTime;
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
                    renderGameOverTransition(1);
                    //spawnGameOver();
                    gameEngine->changeScene("MENU", std::make_shared<SceneMenu>(gameEngine));
                   
                }
            }

            else if (e->getID() == "GAMEOVER" && lifespan.remaining <= 0) {
                e->destroy();
   
              
                    //GameOver();

                    reload = true;
                
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




            
            

            




            Vec2 currentOverlap = Physics::getOverlap(de, e);


            if (currentOverlap.x > 0 && currentOverlap.y > 0) { //collision has occurred 

                Vec2 prevOverlap = Physics::getPreviousOverlap(de, e);

                Vec2 resolution(0, 0);

                std::cout << "I am overlapping" << std::endl;


                //Player-flag collision
                if ((de->getID() == "PLAYER" &&
                    (e->getID() == "FLAG" || e->getID() == "LUCKYFLAG" || e->getID() == "SPECIALFLAG")) ||
                    ((de->getID() == "FLAG" || de->getID() == "LUCKYFLAG" || de->getID() == "SPECIALFLAG") &&
                        e->getID() == "PLAYER")) {

                    auto& player = (de->getID() == "PLAYER") ? de : e;

                    auto& flag = (de->getID() == "FLAG" || de->getID() == "LUCKYFLAG" || de->getID() == "SPECIALFLAG") ? de : e;

                    if (flag->getID() == "FLAG") {
                        gameEngine->playSound("FLAG");
                    }
                    else if (flag->getID() == "SPECIALFLAG") {
                        gameEngine->playSound("SPECIALFLAG");
                    }


                    flag->destroy();        //Flags dissapear just need to implement score collection now 

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

                        playerHit = true;
                        gameLives--;
                        
                        //Respawn player at starting point and respawn enemies too
                        //Round over is called at the same time spawnbang is destroyed so that it lingers on the screen (in lifespan section of movement)



                        if (gameLives == 0) {
                            //gameEngine->playSound("LINKDIE");
                           // GameOver();
                            //reload = true;
                        }
                        

                       
                       
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


//Going to render startgame and end game transitions 
    void ScenePlay::renderStartTransition(float transitionDuration) {
        float elapsedTime = 0.0f;
        while (elapsedTime < transitionDuration) {
            BeginDrawing();
            ClearBackground(BLACK);


            std::string message = "Starting game...";

            const Font& font = gameEngine->getAssets().getFont("orbitron");
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
    void ScenePlay::renderGameOverTransition(float transitionDuration) {
        float elapsedTime = 0.0f;
        while (elapsedTime < transitionDuration) {
            BeginDrawing();
            ClearBackground(BLACK);


            std::string message = "Game Over!";

            const Font& font = gameEngine->getAssets().getFont("orbitron");
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
void ScenePlay::sGUI(){
    rlImGuiBegin();
    ImGui::SetNextWindowSize(ImVec2(400, 350));
        ImGui::Begin("Zelda 0.5",NULL,ImGuiWindowFlags_NoResize);
            ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
            if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
            {
                if (ImGui::BeginTabItem("Controls"))
                {
                    ImGui::SeparatorText("Rendering Controls");
                    ImGui::Checkbox("Textures",&renderTextures);
                    ImGui::Checkbox("Health Bar",&renderHealth);
                    ImGui::Checkbox("Bounding Boxes",&renderBoundingBox);
                    ImGui::Checkbox("Vision Debug",&renderVisionDebug);
                    ImGui::Checkbox("Grid",&renderGridLines);
                    ImGui::SeparatorText("Camera Controls");
                    ImGui::Checkbox("Follow Camera",&followCam);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Entities"))
                {
                    int count=0;
                    if (ImGui::TreeNode("By Tag")){
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
                    if (ImGui::TreeNode("All Entitites")){
                        //TODO Add list of all entities
                        //See your Assignment 2
                        for (const auto& entity : entityManager.getEntities()) {




                            ImGui::PushID(entity->getID().c_str());
                           
                            if (ImGui::Button("D")) {

                                if (entity->getTag() == "PLAYER") {
                                    reload=true;
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
            gameEngine->changeScene("MENU",std::make_shared<SceneMenu>(gameEngine));
        }
        if(action.getName()=="RELOAD"){
            reload=true;
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

    bang->addComponent<CLifespan>(50);
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
void GameOver() {



}

//Randomly spawn flags on game start ensure that tile they are placed on is not already occupied
void spawnFlags() {



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
    player->addComponent<CHealth>(playerConfig.HEALTH,2);
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
 * Spawns a sword at the player's location
 */
void ScenePlay::spawnSword() {
    //TODO: Spawn sword using the players location and facing
    /*
     Ensure that it spawns far enough from the player, has a bounding box, is rotated properly, and has a lifespan of 8-10 frames

     copy bullet code from previous project

    */
    Vec2 playerPos = player->getComponent<CTransform>().position;
    Vec2 direction = player->getComponent<CTransform>().facing;

    std::shared_ptr<Entity> sword;
    sword = entityManager.addEntity("DYNAMIC", "SWORD");
    sword->addComponent<CState>("ATTACKING");   //I don't think the sword needs an animation state but it now has one 
    //player->getComponent<CState>().state = "ATTACK";
    sword->addComponent<CAnimation>(gameEngine->getAssets().getAnimation("SWORD"), true);
    sword->addComponent<CLifespan>(10);
    sword->addComponent<CTransform>();
    sword->addComponent<CDamage>();
    int swordHeight = sword->getComponent<CAnimation>().animation.getScaledSize().x;
    int swordWidth = sword->getComponent<CAnimation>().animation.getScaledSize().y;
    //sword->addComponent<CBoundingBox>(Vec2(swordWidth, swordHeight));

    //float swordDistance = 30.0f;

    // Get player size
    Vec2 playerSize = player->getComponent<CBoundingBox>().size;

    // Calculate the sword's position and rotation
    Vec2 swordPos;
    float rotation;
    float swordDistance = playerSize.x; // Adjus t this value to fine-tune the sword's distance from the player



    //std::cout << direction.x << std::endl;
    //std::cout << direction.y << std::endl;

    if (direction.x > 0) { // Facing right
        std::cout << "Attacking right" << std::endl;
        swordPos.x = playerPos.x + swordDistance + 8;
        swordPos.y = playerPos.y;
        rotation = 90.0f;
    }
    else if (direction.x < 0) { // Facing left
        std::cout << "Attacking Left" << std::endl;
        swordPos.x = playerPos.x - swordDistance - 8;
        swordPos.y = playerPos.y;
        rotation = 270.0f;
    }
    else if (direction.y < 0) { // Facing up
        swordPos.x = playerPos.x;
        swordPos.y = playerPos.y - swordDistance - 12;
        rotation = 0.0f;

    }
    else { // Facing down
        std::cout << "Attacking Down" << std::endl;
        swordPos.x = playerPos.x;
        swordPos.y = playerPos.y + swordDistance + 8;
        rotation = 180.0f;
    }

    // Adjust the position so that the bottom middle of the sword is at the calculated position
    //swordPos.x -= sin(rotation) * swordHeight / 2;
    //swordPos.y += cos(rotation) * swordHeight / 2;

    if (direction.y < 0 || direction.y>0) { // Facing up or down change bbox generation
        sword->addComponent<CBoundingBox>(Vec2(swordHeight, swordWidth));
        sword->addComponent<CTransform>(swordPos, Vec2(0.0f, 0.0f), rotation);

    }
    else {


    sword->addComponent<CBoundingBox>(Vec2(swordWidth, swordHeight));
    sword->addComponent<CTransform>(swordPos, Vec2(0.0f, 0.0f), rotation);
    
   
}

   // sword->addComponent<CTransform>(Vec2(playerPos.x+(swordDistance*direction.x), playerPos.y+(swordDistance*direction.y)), Vec2(0.0f, 0.0f), 0.0f);
    


    //transform.angle = atan2(direction.y, (direction.x) * 180.0 / PI); // Adjust rotation 

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

    if(reload==true){
        reloadScene();
    }
    
}