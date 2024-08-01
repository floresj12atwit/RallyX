#include "physics.hpp"
#include <cmath>

/**
 * Compute overlap this frame
 * 
 * @param a  shared pointer to entity a
 * @param b  shared pointer to entity b
 * @return Vec2  Contains the x and y overlap of entities a and b
 */
Vec2 Physics::getOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b){
    //TODO: Implement the overlap math between two entities as we discussed in class (and Assignment 3)

    //Intercept is only when x1 < x2+w2 x2 && < x2 + w1 && y1 < y2 + h2 && y2 < y2 + h1
    //This needs position values and width and height
    Vec2 posA = a->getComponent<CTransform>().position;
    Vec2 posB = b->getComponent<CTransform>().position;
    Vec2 size_a = a->getComponent<CBoundingBox>().size;  //x and y will be width and height of these enitities
    Vec2 size_b = b->getComponent<CBoundingBox>().size;



    float ox = 0, oy = 0;      //Zero means no overlap so it's an easy way to initalize it 
    //The specific bounding box overlap is only gonna be calculated if we're certain the boxes are intercepting in the first place




    Vec2 delta = { fabs(posA.x - posB.x), fabs(posA.y - posB.y) };


    ox = (size_a.x / 2) + (size_b.x / 2) - delta.x;
    oy = (size_a.y / 2) + (size_b.y / 2) - delta.y;

    //Vec2 overlap = { ox, oy };    another Vec2 for overlap isn't needed since its just passed into a vec 2 at output







      //half size is the mid point of the shape






//

    return Vec2(ox, oy);
}

/**
 * Compute overlap from previous frame
 * 
 * @param a  shared pointer to entity a
 * @param b  shared pointer to entity b
 * @return Vec2  Contains the x and y overlap of entities a and b
 */
Vec2 Physics::getPreviousOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b){
    //TODO:: Same as getOverlap, but uses the previous positions of Entity a and b
    Vec2 posA = a->getComponent<CTransform>().prevPosition;
    Vec2 posB = b->getComponent<CTransform>().prevPosition;
    Vec2 size_a = a->getComponent<CBoundingBox>().size;  //x and y will be width and height of these enitities

    Vec2 size_b = b->getComponent<CBoundingBox>().size;


    float ox = 0, oy = 0;      //Zero means no overlap so it's an easy way to initalize it 


    Vec2 delta = { fabs(posA.x - posB.x), fabs(posA.y - posB.y) };

    ox = (size_a.x / 2) + (size_b.x / 2) - delta.x;
    oy = (size_a.y / 2) + (size_b.y / 2) - delta.y;


    return Vec2(ox, oy);
}

/**
 * Compute if and where there is an overlap between line segment ab and cd
 * 
 * @param a  shared pointer to Vec2 a
 * @param b  shared pointer to Vec2 b
 * @param c  shared pointer to Vec2 c
 * @param d  shared pointer to Vec2 d
 * @return Intersect Struct
 */
Intersect Physics::lineIntersect(const Vec2& a, const Vec2& b, const Vec2& c, const Vec2& d) {
    //TODO: Implement based on lectures slides
    Vec2 u = b - a;
    Vec2 v = d - c;

    float uxv = u.cross(v);
    Vec2 cma = c - a;
    float t = (cma.cross(v)) / uxv;
    float s = (cma.cross(u)) / uxv;

    if (t >= 0 && t <= 1 && s >= 0 && s <= 1) {

        float x = a.x + t * u.x;
        float y = c.y + t * u.y;
        return { true, Vec2(x, y) };
    }
    else {

     return { false,Vec2{0.0f,0.0f} };
     }
}

/**
 * Computes if the line segment ab intersects with the bounding box of an entity
 * 
 * @param a  shared pointer to Vec2 a
 * @param b  shared pointer to Vec2 b
 * @param e  entity to check intersection
 * @return boolean that stores if the line segment ab intersects the bounding box of e
 */
bool Physics::entityIntersect(const Vec2& a, const Vec2& b, std::shared_ptr<Entity> e){
    //TODO: Implement based on lectures slides
    //1. Get each line segment from the entity bounding box (there should be 4 points/lines)
    //2. Call lineIntersect for ab and each line segment from the entity bounding box (4 calls total)
    //3. If any of the lineIntersect calls result in an intersection, this function should return true

    auto& bbox = e->getComponent<CBoundingBox>();               //Getting bounding box of entity
    Vec2 entityPos = e->getComponent<CTransform>().position;    //Getting position of entity (This is the CENTER of the entity)
    Vec2 halfSize = bbox.size * 0.5f;                           //Getting half of the bounding box's size 
    
    //Intersect found ;

    //Getting the four corners of the bounding box
    //Since position is the center of the box we have to do some math 
    Vec2 topLeft = entityPos - halfSize;
    Vec2 topRight = entityPos + Vec2(halfSize.x, -halfSize.y);
    Vec2 bottomLeft = entityPos + Vec2(-halfSize.x, halfSize.y);
    Vec2 bottomRight = entityPos + halfSize;

    // Check intersection with each side of the bounding box
    if (lineIntersect(a, b, topLeft, topRight).doesIntersect) return true;    // Top side
    if (lineIntersect(a, b, topRight, bottomRight).doesIntersect) return true;    // Right side
    if (lineIntersect(a, b, bottomLeft, bottomRight).doesIntersect) return true;    // Bottom side
    if (lineIntersect(a, b, topLeft, bottomLeft).doesIntersect) return true;    // Left side



    return false;
}