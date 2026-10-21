// Engine symbols that libsd's loaders reference but the ocelot tool never reaches:
// it only reads and writes AF/BK files, no PNGs and no live palette.
#include <stdbool.h>

bool read_paletted_png(void) {
    return false;
}
bool read_paletted_png_from_memory(void) {
    return false;
}
bool write_paletted_png(void) {
    return false;
}
bool write_rgb_png(void) {
    return false;
}
void get_resource_filename(void) {
}
void vga_state_set_base_palette_from_range(void) {
}
void vga_state_set_base_palette_index(void) {
}
