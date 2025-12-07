#include "asset.h"
#include "cfg_parser.h"
#include "entity.h"
#include "include.h"
#include "log.h"
#include "scene.h"

asset_cfg_t *find_asset_by_shortcut(map_layout_cfg_t *config, const char shortcut) {
	for (int i = 0; i < MAX_ASSETS; i++) {
		if (config->assets[i].shortcut == shortcut)
			return &config->assets[i];
	}
	return NULL;
}

entity_cfg_t *get_entity_with_type(map_layout_cfg_t *config, const char type_shortcut) {
	for (int i = 0; i < config->entity_count; i++) {
		if (config->entities[i].shortcut == type_shortcut) {
			return &config->entities[i];
		}
	}
	return NULL;
}
void assign_layers(grid_t *grid, map_layout_cfg_t *config) {
	int parsed_entity_count = 0;
	asset_cfg_t *asset;
	entity_cfg_t *curr_entity_cfg;

	for (int y = 0; y < config->grid_height; y++) {
		for (int x = 0; x < config->grid_width; x++) {
			char shortcut = config->layout[0][y][x];
			if (shortcut == '\0')
				continue;

			asset = find_asset_by_shortcut(config, shortcut);

			if (asset) {
				RE_assign_asset_static(grid, asset->engine_asset_key, 0, x, y);
			}
		}
	}

	for (int i = 1; i < config->layer_count; i++) {
		for (int y = 0; y < config->grid_height; y++) {
			for (int x = 0; x < config->grid_width; x++) {
				char shortcut = config->layout[i][y][x];

				if (shortcut != '-' && shortcut != '\0') {
					curr_entity_cfg = get_entity_with_type(config, shortcut);

					if (curr_entity_cfg) {
						int id = RE_add_entity(x, y, curr_entity_cfg->type);
						entity_t* ents = get_entities();
						ents[id].asset_id = curr_entity_cfg-> asset_id;
						parsed_entity_count++;
					}
				}
			}
		}
	}

	if (config->entity_count != parsed_entity_count) {
		log_error("Warning: Number of entities in config (%d) does not match entities found in layout (%d)\n",
		          config->entity_count, parsed_entity_count);
	}
}

int load_cfg(grid_t *grid, map_layout_cfg_t *config) {
	int i;
	asset_cfg_t *curr_asset_cfg;

	RE_init_grid(grid, config->grid_width, config->grid_height, config->tile_width, config->tile_height, config->pad_y);

	for (i = 0; i < config->asset_count; i++) {
		curr_asset_cfg = &config->assets[i];
		curr_asset_cfg->engine_asset_key =
		    RE_load_asset(curr_asset_cfg->filename, curr_asset_cfg->pos_x, curr_asset_cfg->pos_y, curr_asset_cfg->dim_w,
		                  curr_asset_cfg->dim_h);
	}

	assign_layers(grid, config);
	//	assign_animations(); TODO: in parser as well
}
