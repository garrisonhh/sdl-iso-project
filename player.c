#include "entity.h"
#include "vector.h"
#include "collision.h"

entity_t *createPlayer() {
	entity_t *newPlayer = (entity_t *)malloc(sizeof(entity_t));
	newPlayer->sprite = 0;
<<<<<<< HEAD
	newPlayer->pos = (dvector3){0, 0, 9};
=======
	newPlayer->pos = (dvector3){5, 5, 1};
>>>>>>> 0ddeedebff5cc2b35e210e4113f6ff331dee8759
	newPlayer->move = (dvector3){0, 0, 0};
	newPlayer->bbox = (bbox_t){(dvector3){-.5, -.5, 1}, (dvector3){1, 1, 1}};

	return newPlayer;
}

