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

	int id1 = 0;
	int id2 = 1;
	int id3 = 2;
	int id4 = 3;

	if (entities[id4].x == entities[id1].x  &&  entities[id4].y == entities[id1].y) {
		counter++;
	}
	if (entities[id1].x == entities[id2].x && entities[id1].y == entities[id2].y) {
		counter++;
		RE_delete_entity(id2);
		// return 1; // lose condition
	}
	if (entities[id1].x == entities[id3].x && entities[id1].y == entities[id3].y) {
		counter++;
	}
	if (counter == 5) {
		return 0; // win condition
	}

	return -1; // any other value is ignored
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

	app->win_screen_path = "/home/spisladqo/Downloads/spongebob.png";
	app->lose_screen_path = "/home/spisladqo/Downloads/spongebob_youlost.png";

	app_run(app, example_cond_fun);

	app_destroy(app);

	return EXIT_SUCCESS;
}
