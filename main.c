#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <time.h>

#define OUTPUT_FILE_PATH "output.ppm"

#define WIDTH 1080
#define HEIGHT 720

#define COLOR_RED 0xFF0000FF
#define COLOR_BLUE 0xFFFF0000
#define COLOR_GREEN 0xFF00FF00

#define BACKGROUND_COLOR 0xFF000000

#define SEEDS_COUNT 10
#define SEED_MARKER_COLOR 0xFFFFFFFF
#define SEED_MARKER_RADIUS 5

typedef uint32_t color32;
typedef struct {
	int16_t x, y;
} Point32;

static color32 image[HEIGHT][WIDTH];
static Point32 seeds[SEEDS_COUNT];
static color32 colors[SEEDS_COUNT];

void fill_image (color32 color)
{
	for (size_t y = 0; y < HEIGHT; ++y) {
		for (size_t x = 0; x < WIDTH; ++x) {
			image[y][x] = color;
		}
	}
}

void generate_random_seeds (void)
{
	srand(time(0));
	for (size_t i = 0; i < SEEDS_COUNT; ++i) {
		seeds[i].x = rand() % WIDTH;
		seeds[i].y = rand() % HEIGHT;
	}
}

color32 point32_to_color (Point32 p)
{
	assert(p.x > 0 && p.x < UINT16_MAX);
	assert(p.y > 0 && p.y < UINT16_MAX);
	return (0xFFFF0000 & (p.x << 16 * 1)) |
			(0x0000FFFF & (p.y << 16 * 0));
}

void generate_color_from_seeds (void)
{
	for (size_t i = 0; i < SEEDS_COUNT; ++i) {
		colors[i] = point32_to_color(seeds[i]);
	}
}

int sqr_dist(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	int dx = x2 - x1;
	int dy = y2 - y1;

	return dx * dx + dy * dy;
}

void fill_circle (uint16_t cx, uint16_t cy, uint16_t radius, color32 color)
{
	size_t x0 = cx > radius ? cx - radius : 0;
	size_t y0 = cy > radius ? cy - radius : 0;
	size_t x1 = cx + radius > WIDTH ? cx + radius : WIDTH;
	size_t y1 = cy + radius > HEIGHT? cy + radius : HEIGHT;

	int radius_sq = radius * radius;

	for (size_t x = x0; x <= x1; ++x) {
		for (size_t y = y0; y <= y1; ++y) {
			if(sqr_dist(x, y, cx, cy) < radius_sq) {
				image[y][x] = color;
			}
		}
	}
}

void render_seeds (void)
{
	for (size_t i = 0; i < SEEDS_COUNT; ++i) {
		fill_circle(
				seeds[i].x,
				seeds[i].y,
				SEED_MARKER_RADIUS,
				SEED_MARKER_COLOR
				);
	}
}

void render_voronoi (void)
{
	for (size_t y = 0; y < HEIGHT; ++y) {
		for (size_t x = 0; x < WIDTH; ++x) {
			size_t i = 0;
			for (size_t j = 1; j < SEEDS_COUNT; ++j)
				if (sqr_dist(x, y, seeds[j].x, seeds[j].y) < sqr_dist(x, y, seeds[i].x, seeds[i].y))
					i = j;
			image[y][x] = colors[i];
		}
	}
	render_seeds();
}

void save_image_as_ppm (const char *file_path)
{
	FILE *f = fopen(file_path, "wb");
	if(f == NULL) {
		fprintf(
			stderr,
			"Error: Cannot write into file %s: %s\n",
			file_path,
			strerror(errno));
		exit(1);
	}

	fprintf(f, "P6\n%d %d 255\n", WIDTH, HEIGHT);
	for (size_t y = 0; y < HEIGHT; ++y) {
		for (size_t x = 0; x < WIDTH; ++x) {
			// pixel format of uint32_t is AABBGGRR
			color32 color = image[y][x];
			uint8_t bytes[3] = {
				(color & 0x0000FF) >> 8 * 0,
				(color & 0x00FF00) >> 8 * 1,
				(color & 0xFF0000) >> 8 * 2,
			};

			fwrite(bytes, sizeof bytes, 1, f);
			assert(!ferror(f));
		}
	}
	int ret = fclose(f);
	assert(!ret);
}

int main(void)
{
	fill_image(BACKGROUND_COLOR);
	generate_random_seeds();
	generate_color_from_seeds();
	render_voronoi();
	save_image_as_ppm(OUTPUT_FILE_PATH);
	return 0;
}
