#include "vesa.h"
#include "stdint.h"
#include "stddef.h"
size_t terminal_postion_x;
size_t terminal_postion_y;
size_t terminal_font_width;
size_t terminal_font_height;
extern unsigned char arr_8x16_font[];


int init_terminal()
{
    terminal_postion_x = 0;
    terminal_postion_y = 0;
    terminal_font_width = 8;
    terminal_font_height = 16;
}

int draw_vbe_char(char c)
{
    if(c != '\n')
    {
        draw_char_8x16(terminal_postion_x, terminal_postion_y, c);
        terminal_postion_x+=terminal_font_width;
    }
    else
    {
        terminal_postion_x = 0;
        terminal_postion_y+=terminal_font_height;
    }
    

}
void draw_char_8x16(int x, int y, char character) {
    // Assuming each character is represented by 8 bytes in the font array
    int charWidth = 8;
    int charHeight = 16;

    // Calculate the starting index in the font array for the given character
    int startIndex = (int)character * charHeight;

    // Loop through each row of the character
    for (int row = 0; row < charHeight; row++) {
        // Get the corresponding byte from the font array
        unsigned char fontByte = arr_8x16_font[startIndex + row];

        // Loop through each pixel in the row
        for (int col = 0; col < charWidth; col++) {
            // Check if the current pixel is set in the font byte
            if ((fontByte >> (7 - col)) & 1) {
                // Set the pixel at the corresponding position
                vbe_putpixel(x + col, y + row, vbe_rgb(255, 255, 255)); // Assuming white color for the character
            }
        }
    }
}