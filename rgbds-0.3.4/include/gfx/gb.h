/*
 * Copyright © 2013 stag019 <stag019@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef RGBDS_GFX_GB_H
#define RGBDS_GFX_GB_H

#include <stdint.h>
#include "gfx/main.h"

void png_to_gb(const struct PNGImage png, struct GBImage *gb);
void output_file(const struct Options opts, const struct GBImage gb);
int get_tile_index(uint8_t *tile, uint8_t **tiles, int num_tiles,
		   int tile_size);
void create_tilemap(const struct Options opts, struct GBImage *gb,
		    struct Tilemap *tilemap);
void output_tilemap_file(const struct Options opts,
			 const struct Tilemap tilemap);
void output_palette_file(const struct Options opts, const struct PNGImage png);

#endif
