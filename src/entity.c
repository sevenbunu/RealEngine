#include "entity.h"
#include "asset.h"
#include "errors.h"
#include "log.h"
#include <SDL3/SDL.h>

static entity_t entities[MAX_ENTITIES_NUM];

static int curr_entities_num;

int RE_add_entity(int x, int y, enum e_behaviour beh) {
	entity_t ent = {.x = x, .y = y, .depth = 1, .beh = beh};

	if (curr_entities_num >= MAX_ENTITIES_NUM) {
		log_error("Failed to add entity: limit %d is reached.\n", MAX_ENTITIES_NUM);
		return -1;
	}
	if (beh == PLAYER) {
		for (int i = 0; i < curr_entities_num; i++) {
			if (entities[i].beh == 0) {
				log_error("Cannot add more than one player.\n");
				return -1;
			}
		}
	}

	ent.asset_id = 3;
	int id = curr_entities_num++;
	entities[id] = ent;

	char *ent_name;
	if (beh == PLAYER)
		ent_name = "player";
	else
		ent_name = "unknown";
	// log_debug("Added entity with behaviour %s on (%d, %d) with id %d\n", ent_name, x, y, id);

	return id;
}

int RE_delete_entity(int id) {
	if (id < 0 || id > MAX_ENTITIES_NUM) {
		log_error("Failed to delete entity: id should be in range [0, %d].\n", MAX_ENTITIES_NUM);
		return -1;
	}

	if (id >= curr_entities_num) {
		log_error("Failed to delete entity: no entity with id %d exists.\n", id);
		return -1;
	}

	if (entities[id].beh == DELETED) {
		log_warn("Entity with id %d was already deleted.\n", id);
	} else if (entities[id].beh == PLAYER) {
		log_error("Failed to delete entity: cannot delete player entity.\n");
		return -1;
	}

	entities[id].beh = DELETED;

	return 0;
}

int RE_move_entity(int id, int x, int y) {
	if (id < 0 || id > MAX_ENTITIES_NUM) {
		log_error("Failed to move entity: id should be in range [0, %d].\n", MAX_ENTITIES_NUM);
		return -1;
	}

	if (id >= curr_entities_num) {
		log_error("Failed to move entity: no entity with id %d exists.\n", id);
		return -1;
	}

	if (entities[id].beh == DELETED) {
		log_error("Failed to move entity: entity with id %d was deleted.\n", id);
		return -1;
	}

	if (id == 0) {
		log_error("Failed to move entity: cannot move player entity.\n", id);
		return -1;
	}

	entities[id].x = x;
	entities[id].y = y;

	return 0;
}

entity_t *get_entities(void) { return (entity_t *)&entities; }

int get_entities_num(void) { return curr_entities_num; }

void reset_entities(void) {
	for (int i = 0; i < curr_entities_num; i++) {
		entities[i].x = 0;
		entities[i].y = 0;
		entities[i].depth = 0;
		entities[i].beh = 0;
		entities[i].asset_id = 0;
	}
	curr_entities_num = 0;
}