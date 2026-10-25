#define WIDTH  320
#define HEIGHT 240

/* 32-bit ARGB pixel buffer: 320 * 240 * 4 bytes = 307,200 bytes */
static unsigned int pixel_buffer[WIDTH * HEIGHT];

/* Declare the host import exposed by the Chloride runtime */
__attribute__((import_module("env"), import_name("draw_frame")))
extern void draw_frame(const unsigned int *buffer_ptr, unsigned int size);

/* Helper to generate raw ARGB pixels (0xAARRGGBB) */
static inline unsigned int make_color(unsigned char r, unsigned char g, unsigned char b) {
	return (0xFF000000) | ((unsigned int)r << 16) | ((unsigned int)g << 8) | (unsigned int)b;
}

/* Entry point exported to the Chloride runtime */
__attribute__((visibility("default")))
void render(void) {
	int x, y;

	/* Draw a simple gradient / test pattern directly into the framebuffer */
	for (y = 0; y < HEIGHT; y++) {
		for (x = 0; x < WIDTH; x++) {
			unsigned char r = (unsigned char)((x * 255) / WIDTH);
			unsigned char g = (unsigned char)((y * 255) / HEIGHT);
			unsigned char b = 128;

			pixel_buffer[y * WIDTH + x] = make_color(r, g, b);
		}
	}

	/* Flush framebuffer to the host runtime via imports */
	draw_frame(pixel_buffer, sizeof(pixel_buffer));
}

