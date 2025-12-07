#include "app.h"
#include "asset.h"
#include "cfg_loader.h"
#include "include.h"
#include "scene.h"
#include <stdlib.h>

int counter = 0;

int example_cond_fun(void) {
	struct entity* entities = get_entities();
	int entities_num = get_entities_num();

	int player = 0;
	int enemy = 1;
	int food[5] = {2, 3, 4, 5, 6};

	// if (counter != 5 && entities[enemy].x == entities[player].x && entities[enemy].y == entities[player].y) {
	// 	return 1;
	// }
	for (int id = 0; id < 5; id++) {
		if (entities[food[id]].beh != DELETED && entities[player].x == entities[food[id]].x && entities[player].y == entities[food[id]].y) {
			counter++;
			RE_delete_entity(food[id]);
		}
	}
	if (counter == 5 && entities[enemy].x == entities[player].x && entities[enemy].y == entities[player].y) {
		return 0;
	}

	return -1;
}

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;

	map_layout_cfg_t config;

	app_hlpr_t *app = app_create();
	if (!app) {
		return EXIT_FAILURE;
	}

	printf("Parsing config file 'demo.cfg'...\n");
	parse_config("demo/demo.cfg", &config);

	printf("Initializing scene from config...\n");
	load_cfg(&app->grid, &config);

	int err = app_setup(app);
	if (err) {
		app_destroy(app);
		return err;
	}

	app->win_screen_path = "demo/assets/victory.png";
	app->lose_screen_path = "demo/assets/victory.png";

	app_run(app, example_cond_fun);

	app_destroy(app);

	return EXIT_SUCCESS;
}
