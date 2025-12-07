#include "app.h"
#include "SDL3/SDL_events.h"
#include "entity.h"
#include "errors.h"
#include "log.h"
#include "render.h"
#include "scene.h"
#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

void cam_process_key_event(SDL_KeyboardEvent kb_event, app_hlpr_t *app, uint32_t sdl_kb_event_type) {
	if (sdl_kb_event_type != SDL_EVENT_KEY_DOWN)
		return;

	cam_t *cam = &app->cam;
	switch (kb_event.key) {
	case SDLK_UP:
		cam->y -= 1;
		cam->x -= 1;
		break;
	case SDLK_DOWN:
		cam->y += 1;
		cam->x += 1;
		break;
	case SDLK_LEFT:
		cam->x -= 1;
		cam->y += 1;
		break;
	case SDLK_RIGHT:
		cam->x += 1;
		cam->y -= 1;
		break;
	default:
		break;
	}

	// if you want to move camera out of the map, change here
	int max_x = app->grid.tile_num_x - 1;
	int max_y = app->grid.tile_num_y - 1;

	if (cam->x < 0)
		cam->x = 0;
	else if (cam->x > max_x)
		cam->x = max_x;
	if (cam->y < 0)
		cam->y = 0;
	else if (cam->y > max_y)
		cam->y = max_y;
}

void process_input(app_hlpr_t *app) {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_EVENT_QUIT) {
			app->is_running = false;
		}
		if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
			app->is_running = false;
		}
		if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {
			if (event.key.key <= SDLK_UP && event.key.key >= SDLK_RIGHT) {
				app->key_event = event.key;
				cam_process_key_event(app->key_event, app, event.type);
			}
		}
	}
}

app_hlpr_t *app_create(void) {
	app_hlpr_t *app = calloc(1, sizeof(struct app_hlpr));
	if (!app) {
		log_error("Failed to allocate app\n");
		return NULL;
	}

	if (!SDL_Init(SDL_INIT_VIDEO)) {
		log_error("SDL_Init Error: %s\n", SDL_GetError());
		goto err_ex;
	}

	app->window = SDL_CreateWindow(WINDOW_NAME, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
	if (!app->window) {
		log_error("sdl_createwindow error: %s\n", SDL_GetError());
		goto err_ex;
	}

	app->is_running = false;
	app->show_win_screen = false;
	app->show_lose_screen = false;
	app->global_time = 0;
	return app;

err_ex:
	free(app);
	return NULL;
}

void destroy_grid(grid_t *grid) {
	if (!grid) {
		// log_debug("Grid pointer is NULL.\n");
		return;
	}

	if (!grid->tiles) {
		// log_debug("Tiles pointer is NULL.\n");
		return;
	}

	for (int i = 0; i < grid->tile_num_x; i++) {
		if (grid->tiles[i]) {
			free(grid->tiles[i]);
			grid->tiles[i] = NULL;
		}
	}

	free(grid->tiles);
	grid->tiles = NULL;

	// log_debug("Destroyed grid.\n");
}

void destroy_layers(layer_entities_t *layers, int layers_num) {
	for (int l = 0; l < layers_num; l++) {
		if (&layers[l] && layers[l].entities) {
			free(layers[l].entities);
		}
	}

	free(layers);

	// log_debug("Destoyed layers.\n");
}

void app_destroy(app_hlpr_t *app) {
	if (!app)
		return;
	destroy_grid(&app->grid);
	destroy_layers(app->lentities, app->layers_num);
	SDL_DestroyWindow(app->window);
	SDL_Quit();
	free(app);
}

void act_entity(app_hlpr_t *app, entity_t *ent) {
	static unsigned long long last_moved_time = 0;
	entity_t player = app->entities[app->player_ent_id];
	if (!ent)
		return;

	if (ent->beh == PLAYER) {
		ent->x = app->cam.x;
		ent->y = app->cam.y;

		// if (player_entity->x < 0) player_entity->x = 0;
		// if (player_entity->y < 0) player_entity->y = 0;
		// printf("a player acts like a player.\n");
		// printf("player entity is on x, y: %d, %d\n", ent->x, ent->y);
	} else if (ent->beh == NPC) {
		int rand = SDL_rand(4);
		switch (rand) {
		case 0:
			break;
		case 1:
			ent->x++;
			ent->y++;
			break;
		case 2:
			ent->x--;
			ent->y++;
			break;
		case 3:
			ent->x++;
			ent->y--;
			break;
		case 4:
			ent->x--;
			ent->y--;
			break;
		}
	} else if (ent->beh == FOLLOW) {
		int dist = 0;
		if (player.x - ent->x > dist) {
			ent->x++;
		} else if (ent->x - player.x > dist) {
			ent->x--;
		}
		if (player.y - ent->y > dist) {
			ent->y++;
		} else if (ent->y - player.y > dist) {
			ent->y--;
		}
		// log_debug("follow entity is on %d, %d", ent->x, ent->y);
	} else if (ent->beh == CUSTOM) {
		if (app->global_time - last_moved_time > 6) {
			if (player.x > ent->x) {
				ent->x++;
			} else if (ent->x > player.x) {
				ent->x--;
			}
			if (player.y > ent->y) {
				ent->y++;
			} else if (ent->y > player.y) {
				ent->y--;
			}
			last_moved_time = app->global_time;
		}
	} else if (ent->beh == STAND) {
		return;
	}

	int max_x = app->grid.tile_num_x - 1;
	int max_y = app->grid.tile_num_y - 1;

	if (ent->x < 0)
		ent->x = 0;
	else if (ent->x > max_x)
		ent->x = max_x;
	if (ent->y < 0)
		ent->y = 0;
	else if (ent->y > max_y)
		ent->y = max_y;
}

inline int get_depth(entity_t *entity) { return entity->x + entity->y + 1; }

void update_state(app_hlpr_t *app, int (check_condition_fun)()) {
	SDL_Window *window = app->window;
	SDL_Surface *screen = SDL_GetWindowSurface(window);

	int num = app->entities_num;
	entity_t *entities = app->entities;

	layer_entities_t *layers = app->lentities;

	for (int i = 0; i < app->layers_num; i++) {
		app->lentities[i].num_entities = 0;
	}

	for (int i = 0; i < num; i++) {
		// change entities placement somehow
		entity_t *entity = &entities[i];

		act_entity(app, entity);

		int depth = get_depth(entity);
		if (depth < 1) {
			depth = 1;
		}

		int max_layers_num = app->grid.tile_num_x + app->grid.tile_num_y + 1;
		if (depth > max_layers_num) {
			depth = max_layers_num;
		}

		entity->depth = depth;

		// this updates location in memory

		int num_entities = app->lentities[depth].num_entities++;
		app->lentities[depth].entities[num_entities] = *entity;

		// log_debug("entity %d x,y: %d, %d, depth: %d", i, entity.x, entity.y, depth);
	}

	// shadows, etc
	// for ()

	int cond = check_condition_fun();
	if (cond == 0) {
		app->show_win_screen = true;
		app->show_lose_screen = false;
	} else if (cond == 1) {
		app->show_win_screen = false;
		app->show_lose_screen = true;
	}
}

void app_run(app_hlpr_t *app, int (*check_condition_fun)()) {
	app->is_running = true;
	while (app->is_running && !app->show_lose_screen && !app->show_win_screen) {
		process_input(app);
		update_state(app, check_condition_fun);
		render_scene(app);
		app->global_time++;
		// SDL_Delay(16);
	}
	if (!app->show_lose_screen && !app->show_win_screen) {
		return;
	}

	app->is_running = true;
	while (app->is_running) {
		process_input(app);
		char *path = app->show_win_screen ? app->win_screen_path : app->lose_screen_path; 
		show_image_by_path(app, path);
	}
}

int init_layers(app_hlpr_t *app) {
	int max_layers_num = app->grid.tile_num_x + app->grid.tile_num_y + 1;

	layer_entities_t *layers = malloc(sizeof(layer_entities_t) * max_layers_num);
	if (!layers) {
		// log_debug("Failed to init layers\n");
		return ERR_NOMEM;
	}

	for (int l = 0; l < max_layers_num; l++) {
		entity_t *entities = malloc(sizeof(entity_t) * MAX_ENTITIES_PER_LAYER);
		layers[l].num_entities = 0;
		layers[l].entities = entities;
		if (!entities) {
			log_error("Failed to allocate memory for entities on layer %d.\n", l);

			for (int j = 0; j < l; j++) {
				free(layers[j].entities);
			}
			free(layers);
			return ERR_NOMEM;
		}
	}

	for (int i = 0; i < app->entities_num; i++) {
		entity_t entity = app->entities[i];
		int depth = get_depth(&entity);
		layer_entities_t *layer = &layers[depth];

		layer->entities[layer->num_entities++] = entity;
		// log_debug("put entity %d to depth %d", i, depth);
	}

	app->lentities = layers;
	app->layers_num = max_layers_num;
	log_debug("initialized %d layers.", max_layers_num);

	return 0;
}

void add_entities(app_hlpr_t *app) {
	app->entities = get_entities();
	app->entities_num = get_entities_num();
	init_layers(app);
}

int setup_player(app_hlpr_t *app) {
	for (int i = 0; i < app->entities_num; i++) {
		entity_t ent = app->entities[i];
		if (ent.beh == PLAYER) {
			app->player_ent_id = i;
			app->cam.x = ent.x;
			app->cam.y = ent.y;
			return 0;
		}
	}

	log_debug("Error: player entity was not set.\n");
	return 1;
}

int app_setup(app_hlpr_t *app) {
	add_entities(app);

	int err = setup_player(app);
	return err;
}

int RE_init_grid(grid_t *grid, int tile_num_x, int tile_num_y, int tile_width, int tile_height, int pad_y) {
	SDL_Surface ***tiles;

	if (!grid) {
		log_debug("Failed to init grid: a grid pointer should not be NULL pointer.\n");
		return ERR_ARGS;
	}

	tiles = malloc(sizeof(SDL_Surface **) * tile_num_x);
	if (!tiles) {
		log_error("Failed to allocate memory for tiles.\n");
		return ERR_NOMEM;
	}

	for (int k = 0; k < tile_num_x; k++) {
		tiles[k] = malloc(sizeof(SDL_Surface *) * tile_num_y);
		if (!tiles[k]) {
			log_error("Failed to allocate memory for tiles[%d].\n", k);
			for (int f = 0; f < k; f++) {
				free(tiles[f]);
			}
			free(tiles);
			return ERR_NOMEM;
		}
		for (int j = 0; j < tile_num_y; j++) {
			tiles[k][j] = NULL;
		}
	}

	grid->tiles = tiles;
	grid->tile_num_x = tile_num_x;
	grid->tile_num_y = tile_num_y;
	grid->tile_width = tile_width;
	grid->tile_height = tile_height;
	grid->pad_y = pad_y;
	log_debug("Initialized grid %d x %d", tile_num_x, tile_num_y);

	return 0;
}
