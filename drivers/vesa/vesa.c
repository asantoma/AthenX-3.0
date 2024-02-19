#include "vesa.h"
#include "bios32.h"
#include "io_ports.h"
#include "isr.h"
#include "string.h"
#include "types.h"
#include "vmm.h"
#include "pagepmm.h"
#include "printf.h"
#define PRINT_MODES 1
// vbe information
void test_memory_mapping(uint32_t start_address, uint32_t size);
VBE20_INFOBLOCK g_vbe_infoblock;
VBE20_MODEINFOBLOCK g_vbe_modeinfoblock;
// selected mode
int g_selected_mode = -1;
// selected mode width & height
uint32 g_width = 0, g_height = 0;
// buffer pointer pointing to video memory
uint32 *g_vbe_buffer = NULL;

// get vbe info
int get_vbe_info() {
    REGISTERS16 in = {0}, out = {0};
    // set specific value 0x4F00 in ax to get vbe info into bios memory area
    in.ax = 0x4F00;
    // set address pointer to BIOS_CONVENTIONAL_MEMORY where vbe info struct will be stored
    in.di = BIOS_CONVENTIONAL_MEMORY;
    kprints("Calling BIOS interrupt\n");
    int86(0x10, &in, &out);  // call video interrupt 0x10
    kprints("called video interrupt\n");
    // copy vbe info data to our global variable g_vbe_infoblock
    memcpy(&g_vbe_infoblock, (void *)BIOS_CONVENTIONAL_MEMORY, sizeof(VBE20_INFOBLOCK));
    return (out.ax == 0x4F);
}

void get_vbe_mode_info(uint16 mode, VBE20_MODEINFOBLOCK *mode_info) {
    REGISTERS16 in = {0}, out = {0};
    // set specific value 0x4F00 in ax to get vbe mode info into some other bios memory area
    in.ax = 0x4F01;
    in.cx = mode; // set mode info to get
    in.di = BIOS_CONVENTIONAL_MEMORY + 1024;  // address pointer, different than used in get_vbe_info()
    int86(0x10, &in, &out);  // call video interrupt 0x10
    // copy vbe mode info data to parameter mode_info
    memcpy(mode_info, (void *)BIOS_CONVENTIONAL_MEMORY + 1024, sizeof(VBE20_MODEINFOBLOCK));
}

void vbe_set_mode(uint32 mode) {
    REGISTERS16 in = {0}, out = {0};
    // set any given mode, mode is to find by resolution X & Y
    in.ax = 0x4F02;
    in.bx = mode;
    int86(0x10, &in, &out);  // call video interrupt 0x10 to set mode
}

// find the vbe mode by width & height & bits per pixel
uint32 vbe_find_mode(uint32 width, uint32 height, uint32 bpp) {
    // iterate through video modes list
    uint16 *mode_list = (uint16 *)g_vbe_infoblock.VideoModePtr;
    uint16 mode = *mode_list++;
    while (mode != 0xffff) {
        // get each mode info
        get_vbe_mode_info(mode, &g_vbe_modeinfoblock);
        if (g_vbe_modeinfoblock.XResolution == width && g_vbe_modeinfoblock.YResolution == height && g_vbe_modeinfoblock.BitsPerPixel == bpp) {
            return mode;
        }
        mode = *mode_list++;
    }
    return -1;
}

// print availabel modes to console
void vbe_print_available_modes() {
    VBE20_MODEINFOBLOCK modeinfoblock;

    // iterate through video modes list
    uint16 *mode_list = (uint16 *)g_vbe_infoblock.VideoModePtr;
    uint16 mode = *mode_list++;
    while (mode != 0xffff) {
        get_vbe_mode_info(mode, &modeinfoblock);
        kprints("Mode: %d, X: %d, Y: %d\n", mode, modeinfoblock.XResolution, modeinfoblock.YResolution);
        mode = *mode_list++;
    }
}

// set rgb values in 32 bit number
uint32 vbe_rgb(uint8 red, uint8 green, uint8 blue) {
    uint32 color = red;
    color <<= 16;
    color |= (green << 8);
    color |= blue;
    return color;
}

// put the pixel on the given x,y point
void vbe_putpixel(int x, int y, int color) {
    uint32 i = y * g_width + x;
    *(g_vbe_buffer + i) = color;
}

int vesa_init(uint32 width, uint32 height, uint32 bpp) {
    bios32_init();
    kprints("initializing vesa vbe 2.0\n");
    if (!get_vbe_info()) {
        kprints("No VESA VBE 2.0 detected\n");
        return -1;
    }
    kprints("Checked All modes");
    // set this to 1 to print all available modes to console
    #define PRINT_MODES 0
    #if PRINT_MODES
        kprints("Press UP and DOWN arrow keys to scroll\n");
        kprints("Modes:\n");
        vbe_print_available_modes();
        return 1;
    #else
        g_selected_mode = vbe_find_mode(width, height, bpp);
        if (g_selected_mode == -1) {
            kprints("failed to find mode for %d-%d\n", width, height);
            return -1;
        }
        kprints("\nselected mode: %d \n", g_selected_mode);
        // set selection resolution to width & height
        g_width = g_vbe_modeinfoblock.XResolution;
        g_height = g_vbe_modeinfoblock.YResolution;
        // set selected mode video physical address point to buffer for pixel plotting
        g_vbe_buffer = (uint32 *)g_vbe_modeinfoblock.PhysBasePtr;
        // set the mode to start graphics window
        vbe_set_mode(g_selected_mode);
    #endif
    set_print_mode(1);
    return 0;
}
#define VESA_VIRTUAL_BASE 0xB8000000

#define VESA_VIRTUAL_BASE 0xB8000000

int map_vesa() {
    uint32_t vesa_size = g_vbe_modeinfoblock.YResolution*g_vbe_modeinfoblock.XResolution*g_vbe_modeinfoblock.BitsPerPixel;
    if (g_vbe_buffer == NULL) {
        // If the VBE buffer is not initialized, return an error
        return -1;
    }

    uint32_t vesa_virtual_address = VESA_VIRTUAL_BASE;
    uint32_t vesa_physical_address = (uint32_t)g_vbe_buffer;
    uint32_t vesa_size_in_pages = (vesa_size + PAGE_SIZE - 1) / PAGE_SIZE; // Calculate the number of pages needed

    for (uint32_t i = 0; i < vesa_size_in_pages; i++) {
        map(vesa_virtual_address, vesa_physical_address, PAGE_PRESENT | PAGE_WRITE);
        vesa_virtual_address += PAGE_SIZE;
        vesa_physical_address += PAGE_SIZE;
    }

    // Update the global VBE buffer pointer
    g_vbe_buffer = (uint32_t *)VESA_VIRTUAL_BASE;

    return 0;
}
void test_memory_mapping(uint32_t start_address, uint32_t size) {
    // Pattern to write to memory (just an example)
    uint32_t pattern = VBE_RGB(255,0,0);
    
    // Write the pattern to memory
    for (uint32_t i = 0; i < size / sizeof(uint32_t); i++) {
        *((uint32_t *)(start_address + i * sizeof(uint32_t))) = pattern;
    }
    
    write_to_com1_string("Memory written successfully!\n");
}
int leave_vesa_mode() {
    REGISTERS regs;
    regs.eax = 0x0003; // Function 00h - Set Video Mode
    // regs.h.al = 0x03; // VGA mode number 03h (80x25 text mode)
    int86(0x10, &regs, &regs); // Call BIOS interrupt
    return 0;
}

int vesa_scroll(int lines)
{
    scroll_screen(g_vbe_buffer,g_width,g_height,lines);
}
void scroll_screen(uint32_t* framebuffer, int width, int height, int num_lines) {
    // Calculate the number of pixels to shift up
    int pixels_to_shift = num_lines * width;

    // Calculate the number of pixels to copy
    int pixels_to_copy = width * (height - num_lines);

    // Copy pixels upwards
    memmove(framebuffer, framebuffer + pixels_to_shift, pixels_to_copy * sizeof(uint32_t));

    // Clear the bottom lines
    memset(framebuffer + pixels_to_copy, 0, pixels_to_shift * sizeof(uint32_t));
}