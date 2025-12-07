#include "render.h"
#include "app.h"
#include "asset.h"
#include "log.h"
#include "utils.h"
#include <stdint.h>

void render_asset(SDL_Surface *screen, int sx, int sy, asset_t *asset) {
	if (!screen) {
		log_debug("Given null screen, I give up.");
		return;
	}
	if (!asset) {
		log_debug("Given null asset, I give up.");
		return;
	}

	int asset_w = asset->width;
	int asset_h = asset->height;

	if (asset_w < 0) {
		log_debug("Given negative width %d, I give up.", asset_w);
		return;
	}
	if (asset_h < 0) {
		log_debug("Given negative height %d, I give up.", asset_h);
		return;
	}

	int bytes_per_pxl = (screen->pitch / screen->w);

	for (int x = 0; x < asset_w; x++) {
		for (int y = 0; y < asset_h; y++) {
			// log_debug ("x, y: %d, %d, x, y: %d, %d", x, y, x, y);

			int screen_x = sx + x;
			int screen_y = sy + y;

			if (screen_x < 0 || screen_x >= WINDOW_WIDTH || screen_y < 0 || screen_y >= WINDOW_HEIGHT) {
				continue;
			}

			SDL_Surface *img = asset->img;

            Uint32 *const screen_pxl = (Uint32*) ((Uint8 *) screen->pixels + screen_y * screen->pitch + screen_x * bytes_per_pxl);
            Uint32 *asset_pxl = (Uint32*) ((Uint8 *) img->pixels + y * img->pitch + x * bytes_per_pxl);

			Uint8 ar, ag, ab, aa;

			const SDL_PixelFormatDetails *aformat = SDL_GetPixelFormatDetails(img->format);
			SDL_GetRGBA(*asset_pxl, aformat, NULL, &ar, &ag, &ab, &aa);

			if (aa == 255) {
				*screen_pxl = *asset_pxl;
			} else if (aa > 0) {
				Uint8 sr, sg, sb;
				const SDL_PixelFormatDetails *sformat = SDL_GetPixelFormatDetails(screen->format);

				SDL_GetRGB(*screen_pxl, sformat, NULL, &sr, &sg, &sb);

				float norm_aa = aa / 255.0f;

				Uint8 r = MIN(255, sr + ar * norm_aa);
				Uint8 g = MIN(255, sg + ag * norm_aa);
				Uint8 b = MIN(255, sb + ab * norm_aa);

				*screen_pxl = SDL_MapRGB(sformat, NULL, r, g, b);
			}
		}
	}
}

void render_shadow(SDL_Surface *screen, int sx, int sy, asset_t *asset, float li_dir_x, float li_dir_y,
                   float sh_scale) {

	int bytes_per_pxl = (screen->pitch / screen->w);
	int asset_w = asset->img->w;
	int asset_h = asset->img->h;

	for (int x = 0; x < asset_w; x++) {
		for (int y = 0; y < asset_h; y++) {
			SDL_Surface *img = asset->img;
			Uint32 *asset_pxl = (Uint32 *)((Uint8 *)img->pixels + y * img->pitch + x * bytes_per_pxl);

			Uint8 ar, ag, ab, aa;
			const SDL_PixelFormatDetails *aformat = SDL_GetPixelFormatDetails(img->format);
			SDL_GetRGBA(*asset_pxl, aformat, NULL, &ar, &ag, &ab, &aa);

			if (aa == 0)
				continue;

			int height_scale = (asset_h - y) * sh_scale;
			int shadow_x = x + sx + (li_dir_x * height_scale);
			int shadow_y = y + sy + (li_dir_y * height_scale);

			if (shadow_x < 0 || shadow_x >= WINDOW_WIDTH || shadow_y < 0 || shadow_y >= WINDOW_HEIGHT) {
				continue;
			}

			Uint32 *shadow_pxl =
			    (Uint32 *)((Uint8 *)screen->pixels + shadow_y * screen->pitch + shadow_x * bytes_per_pxl);
			const SDL_PixelFormatDetails *sformat = SDL_GetPixelFormatDetails(screen->format);

			Uint8 sr, sg, sb;
			SDL_GetRGB(*shadow_pxl, sformat, NULL, &sr, &sg, &sb);

			Uint8 shadow_r = sr * 0.5f;
			Uint8 shadow_g = sg * 0.5f;
			Uint8 shadow_b = sb * 0.5f;

			*shadow_pxl = SDL_MapRGB(sformat, NULL, shadow_r, shadow_g, shadow_b);
		}
	}
}

// TODO: store background statically
void render_background(app_hlpr_t *app) {
	SDL_Window *window = app->window;
	SDL_Surface *screen = SDL_GetWindowSurface(window);

	int TILE_WIDTH = app->grid.tile_width;
	int TILE_HEIGHT = app->grid.tile_height;

	int cam_x = app->cam.x;
	int cam_y = app->cam.y;

	// light blue for sky
	Uint32 color = SDL_MapSurfaceRGB(screen, 144, 213, 255);
	SDL_FillSurfaceRect(screen, NULL, color);

	if (!app->grid.tiles) {
		log_debug("NO TILES!\n");
		return;
	}

	int tile_num_x = app->grid.tile_num_x;
	int tile_num_y = app->grid.tile_num_y;

	int screen_center_x = screen->w / 2;
	int screen_center_y = screen->h / 2;

	int bytes_per_pxl = (screen->pitch / screen->w);
	int pad = app->grid.pad_y;

	for (int x = 0; x < tile_num_x; ++x) {
		for (int y = 0; y < tile_num_y; ++y) {
			SDL_Surface *image = app->grid.tiles[x][y];
			if (!image) {
				continue;
			}

			int grid_x = x - cam_x;
			int grid_y = y - cam_y;
			int sx = (grid_x - grid_y) * (TILE_WIDTH / 2) + WINDOW_WIDTH / 2 - TILE_WIDTH / 2;
			int sy = (grid_x + grid_y) * (TILE_HEIGHT / 2 - pad) + WINDOW_HEIGHT / 2 - TILE_HEIGHT / 2;

			for (int tile_x = 0; tile_x < TILE_WIDTH; tile_x++) {
				for (int tile_y = 0; tile_y < TILE_HEIGHT; tile_y++) {
					// log_debug ("x, y: %d, %d, tile_x, tile_y: %d, %d", x, y, tile_x, tile_y);

					int screen_x = sx + tile_x;
					int screen_y = sy + tile_y;

					if (screen_x < 0 || screen_x >= WINDOW_WIDTH || screen_y < 0 || screen_y >= WINDOW_HEIGHT) {
						continue;
					}

					Uint32 *const screen_pxl =
					    (Uint32 *)((Uint8 *)screen->pixels + screen_y * screen->pitch + screen_x * bytes_per_pxl);
					Uint32 *new_pixel =
					    (Uint32 *)((Uint8 *)image->pixels + tile_y * image->pitch + tile_x * bytes_per_pxl);

					Uint8 r, g, b, a;

					const SDL_PixelFormatDetails *format = SDL_GetPixelFormatDetails(image->format);
					SDL_GetRGBA(*new_pixel, format, NULL, &r, &g, &b, &a);

					if (a > 0) {
						*screen_pxl = *new_pixel;
					}
				}
			}
		}
	}
}

void render_entities(app_hlpr_t *app) {
	SDL_Window *window = app->window;
	SDL_Surface *screen = SDL_GetWindowSurface(window);

	int num = app->entities_num;
	entity_t *entities = app->entities;

	int layers_num = app->layers_num;
	layer_entities_t *layers = app->lentities;

	int TILE_WIDTH = app->grid.tile_width;
	int TILE_HEIGHT = app->grid.tile_height;

	int pad = app->grid.pad_y;

    for (int l = 0; l < layers_num; l++) {
        for (int i = 0; i < layers[l].num_entities; i++) {

			// entity_t ent = entities[i];

			entity_t ent = layers[l].entities[i];

			if (ent.beh == DELETED) continue;

			asset_t *asset = RE_get_asset(ent.asset_id - 1);
			if (!asset) {
				continue;
			}
			int asset_h = asset->height;
			int asset_w = asset->width;
			SDL_Surface *image = asset->img;
			if (!image) {
				continue;
			}

			int cam_x = app->cam.x;
			int cam_y = app->cam.y;
			int x = ent.x;
			int y = ent.y;
			int grid_x = x - cam_x - 1;
			int grid_y = y - cam_y - 1;

			int sx = (grid_x - grid_y) * (TILE_WIDTH / 2) + WINDOW_WIDTH / 2 - asset_w / 2;
			int sy = (grid_x + grid_y) * (TILE_HEIGHT / 2 - pad) + WINDOW_HEIGHT / 2 - asset_h / 2;

			int TILE_WIDTH = app->grid.tile_width;
			int TILE_HEIGHT = app->grid.tile_height;

			float li_dir_x = 1.0;
			float li_dir_y = 0.5;
			float sh_scale = 1;

			// int shadow_sx = sx - TILE_WIDTH * li_dir_x;
			// int shadow_sy = sy - TILE_HEIGHT * li_dir_y * 2;
			int shadow_sx = sx;
			int shadow_sy = sy;

			if (sx < 0 || sx > WINDOW_WIDTH || sy < 0 || sy > WINDOW_HEIGHT) {
				continue;
			}
            render_shadow(screen, shadow_sx, shadow_sy, asset,
                                li_dir_x, li_dir_y, sh_scale);
            render_asset(screen, sx, sy, asset);
        }
    }
}

void render_scene(app_hlpr_t *app) {
	render_background(app);
	render_entities(app);
	SDL_UpdateWindowSurface(app->window);
}

void show_image(app_hlpr_t *app, SDL_Surface *img) {
	SDL_Window *window = app->window;
	SDL_Surface *screen = SDL_GetWindowSurface(window);
	SDL_Rect rect_dest = {0, 0, img->w, img->h};
	SDL_BlitSurface(img, NULL, screen, &rect_dest);
	SDL_UpdateWindowSurface(window);
}

void show_image_by_path(app_hlpr_t *app, const char *path) {
	SDL_Surface *img = SDL_LoadPNG(path);
	show_image(app, img);
}