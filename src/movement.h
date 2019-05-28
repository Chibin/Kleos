#ifndef __MOVEMENT_H__
#define __MOVEMENT_H__

struct Movement
{
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
};

enum MovementType
{
    X_MOVEMENT,
    Y_MOVEMENT,
    XY_MOVEMENT
};

enum MovementPattern
{
    WAYPOINT,
    UNI_DIRECTIONAL,
    BI_DIRECTIONAL,
};

void MoveUp(Movement *movement);
void MoveDown(Movement *movement);
void MoveLeft(Movement *movement);
void MoveRight(Movement *movement);
#endif
