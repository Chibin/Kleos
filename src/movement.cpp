#include "movement.h"

inline void MoveUp(Movement *movement)
{
    /* TODO: Replace this later with speed (time based)*/
    movement->acceleration = glm::vec3(0, 9.81, 0);
    movement->velocity = glm::vec3(0, 15, 0);
}

inline void MoveDown(Movement *movement)
{
    movement->velocity += glm::vec3(0, -0.01, 0);
}

inline void MoveLeft(Movement *movement)
{
    movement->velocity.x += -5.0f;
}

inline void MoveRight(Movement *movement)
{
    movement->velocity.x += 5.0f;
}


