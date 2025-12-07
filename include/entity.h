#ifndef ENTITY_H
#define ENTITY_H

#include "animation.h"
#include "asset.h"

#define MAX_ENTITIES_NUM 1000
#define MAX_ENTITIES_PER_LAYER 100

/*
    A number specifying entity's behaviour.
*/
enum e_behaviour {
	DELETED = -1,
	PLAYER = 0,
	NPC = 1,
	FOLLOW = 2,
	STAND = 3,
	CUSTOM = 4,
	ESCAPING = 5,
	// these should be more precise, to be added
};

/*
    A structure representing an entity.
*/
typedef struct entity {
	int x;
	int y;
	int depth;
	int asset_id;
	enum e_behaviour beh;
	animation_t animations[MAX_ANIMATIONS_NUM];
	int animations_num;
	int curr_animation;
	asset_t *asset;
} entity_t;

/*
    A structure representing all entities in the layer i.
*/
typedef struct layer_entities {
	entity_t *entities;
	int num_entities;
} layer_entities_t;

/*
    Add entity on given grid coordinate with given behaviour.

	@param x x of pile to add to.
	@param y y of pile to add to.
	@param beh behaviour of the entity.

	@returns id of created entity on success, -1 otherwise.
*/
int RE_add_entity(int x, int y, enum e_behaviour beh);

/*
    Delete entity by id.

    @param id id of entity to be deleted.
	Note that player entity cannot be deleted.
	If entity was already deleted

	@returns 0 if entity was deleted, -1 otherwise.
*/
int RE_delete_entity(int id);


/*
    Move entity by id.

    @param id id of entity to be deleted.
	Note that player entity cannot be moved.
	@param x x of pile to move to.
	@param y y of pile to move to.

	@returns 0 if entity was moved, -1 otherwise.
*/
int RE_move_entity(int id, int x, int y);

entity_t *get_entities(void);
int get_entities_num(void);
void reset_entities(void);

#endif // ENTITY_H
