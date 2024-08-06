#pragma once

#include <string>
#include <vector>
#include <include/raylib.h>
#include "vec2.hpp"
#include "animation.hpp"

/**
 * Base class for all Component objects
 */
class Component{
    public:
    bool has=false; /* Boolean if the object has this component */
};

/**
 * Transform Component
 * 
 * Defines the position, velocity, scale, and angle of an entity
 * 
 * All entities should have a transform
 */
class CTransform : public Component{
    public:
        Vec2 position;        /* Current position (x,y) of the entity */
        Vec2 prevPosition;    /* Previous frame position (x,y) of the entity */
        Vec2 velocity;        /* Current velocity (vx,vy) of the entity */
        Vec2 scale;           /* Current scale (sx,xy) of the entity */
        Vec2 facing = {0,-1};  /* Current (x,y) facing, either 1 or -1 for x and y */  
        float angle;          /* Current rotation angle of the object (deg) */
        bool isStuck;

        CTransform();
        CTransform(const Vec2& position, const Vec2& velocity, float angle);
};

/**
 * Lifespan Component
 * 
 * Added to entities that have a limited lifespan, e.g. bullets
 */
class CLifespan : public Component{
    public:
        int remaining=0;  /* Remaining frames that the entity is alive */
        int total=0;      /* Total number of frame the entity can live */
        CLifespan(int total);
};

/**
 * Input Component
 * 
 * Contains booleans for the possible inputs that the player can perform
 */
class CInput : public Component{
    public:
        bool left=false;   /* Is the player pressing the left button */
        bool right=false;  /* Is the player pressing the right button */
        bool up=false;     /* Is the player pressing the up button */
        bool down=false;   /* Is the player pressing the down button */
        bool attack=false; /* Is the player pressing the attack button */


        CInput();
};

/**
 * Health Component
 * 
 * Defines the current and maximum health of the entity
 */
class CHealth : public Component{
    public:
        int max;      /* Maximum entity health */
        int current;  /* Current entity health */

        CHealth();
        CHealth(int m, int c);
};

/**
 * Bounding Box Component
 * 
 * Defines the hight and width of the bounding box used in the physics system
 * and if this bounding box blocks vision
 */
class CBoundingBox : public Component{
    public:
        Vec2 size = {0.0f, 0.0f};    /* Width and height of the entity bounding box */
        bool blocksVision=true;      /* Does this bounding box block AI vision */
        CBoundingBox();
        CBoundingBox(const Vec2& s);
};

/**
 * Animation Component
 * 
 * Defines the animation properties
 */
class CAnimation : public Component {
    public:
        Animation animation; /*Actual animation object for this component */
        bool repeat;         /*true/false if this animation repeats after the last animation frame */

        CAnimation() = default;
        CAnimation(const Animation &animation, bool r) : animation(animation), repeat(r) {}
};

/**
 * State Component
 * 
 * Defines the state of the object, including if it is on the ground and the direction, left/right, that it is facing
 */
class CState : public Component {
    public:
        std::string state;     /*Current state name */
        bool isGrounded=false; /*Is the entity touching the top of another entity */
        
        

        CState();
        CState(std::string state);
};

/**
 * Damage Component
 * 
 * Defines the amount of damage this entity does when it collides with another entity
 */
class CDamage : public Component{
    public:
        int damage=1;  /*Amount of damage this entity does when in contact with another entity that contains a health component */
        CDamage();
        CDamage(int damage);
};

/**
 * Invincibility Component
 * 
 * When attached to an entity, this entity does not take damage
 */
class CInvincibility : public Component{
    public:
        int iframes=0;   /*Total invincibility frames */
        int remaining=0; /*Remaining invincibility frames */
        CInvincibility();
        CInvincibility(int frames);
};

/**
 * Follow Player AI Component
 * 
 * Defines the follow AI
 */
class CFollowPlayer : public Component{
    public:
        Vec2 home={0,0};  /*Point that the AI moves toward */
        float speed=0;    /*Speed that the AI moves at */
        CFollowPlayer();
        CFollowPlayer(Vec2 h, float s);
};

/**
 * Patrol Component
 * 
 * Defines the patrol movement AI
 */
class CPatrol : public Component{
    public:
        std::vector<Vec2> positions;  /*Array of Vec2 patrol positions */
        int currentPosition=0;        /*Current index in the positions array that the AI is moving toward */
        float speed=0;                /*Speed that the AI moves at */
        CPatrol();
        CPatrol(std::vector<Vec2>& pos, float s);
};

