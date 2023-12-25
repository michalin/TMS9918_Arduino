/* Arduino library for TMS9918A, TMS9928 and TMS9929A Video Display Processors
    Copyright (C) 2022  Doctor Volt

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "tms9918.h"
#include "patterns.h"

#define MODE 11
#define CSW 10
#define CSR 9
#define RESET 8
#define R1_IE 0x20
#define R1_M1 0x10
#define R1_M2 0x08
#define R1_SIZE 0x02
#define R1_MAG 0x01

struct
{
    uint8_t x;
    uint8_t y;
} cursor;

uint16_t sprite_attribute_table;
uint16_t sprite_pattern_table;
uint8_t sprite_size_sel; //0: 8x8 sprites 1: 16x16 sprites
uint16_t name_table;
uint16_t color_table_size = 0;
uint16_t color_table;
uint16_t pattern_table;
uint8_t crsr_max_x = 31; // Overwritten in Text mode
const uint8_t crsr_max_y = 23;
uint8_t vdp_mode;

uint8_t fgcolor;
uint8_t bgcolor;

#define FORCE_INLINE //This makes the code faster, but increases memory usage
#ifdef FORCE_INLINE 
inline void writeByteToVRAM(unsigned char value) __attribute__((always_inline));
inline uint8_t readByteFromVRAM() __attribute__((always_inline));
inline uint8_t readPort() __attribute__((always_inline));
inline void writePort(unsigned char value) __attribute__((always_inline));
inline void setDBReadMode() __attribute__((always_inline));
inline void setDBWriteMode() __attribute__((always_inline));
#endif

//Core IO functions. Make adaptions to other platforms here -->
void setDBReadMode()
{
#ifdef ARDUINO_ARCH_AVR
    DDRD = DDRD & B00001111; // Set Pin 4..7 as inputs. High nibble of databus. D7 MSB
    DDRC = DDRC & B11110000; // Set Analog pin 0..3 as inputs A0: LSB
#endif
}

uint8_t readPort()
{
#ifdef ARDUINO_ARCH_AVR
    return (PIND & 0xF0) | (PINC & 0x0F);
#endif
}

void setDBWriteMode()
{
#ifdef ARDUINO_ARCH_AVR
    DDRD = DDRD | B11110000; // Set Pin 4..7 as outputs. High nibble of databus.
    DDRC = DDRC | B00001111; // Set Analog pin 0..3 as outputs
#endif
}

void writePort(unsigned char value)
{
#ifdef ARDUINO_ARCH_AVR
    PORTD = (PIND & 0x0F) | (value & 0xF0);
    PORTC = (PINC & 0xF0) | (value & 0x0F);
#endif
}
//<-- Core IO functions. Make adaptions to other platforms here

void reset()
{
    // Serial.println("Resetting");
    digitalWrite(RESET, HIGH);
    delayMicroseconds(100);
    digitalWrite(RESET, LOW);
    delayMicroseconds(5);
    digitalWrite(RESET, HIGH);
}

// Writes a byte to databus for register access
void writeByte(unsigned char value)
{
    setDBWriteMode();
    writePort(value);
    digitalWrite(MODE, HIGH);
    digitalWrite(CSW, LOW);
    delayMicroseconds(1);
    digitalWrite(CSW, HIGH);
    setDBReadMode();
}

// Reads a byte from databus for register access
uint8_t read_status_reg()
{
    setDBReadMode();
    digitalWrite(MODE, HIGH);
    digitalWrite(CSR, LOW);
    delayMicroseconds(1);
    uint8_t memByte = readPort();
    digitalWrite(CSR, HIGH);
    return memByte;
}

// Writes a byte to databus for vram access
void writeByteToVRAM(unsigned char value)
{
    digitalWrite(MODE, LOW);
    digitalWrite(CSW, LOW);
    setDBWriteMode();
    writePort(value);
    //delayMicroseconds(1);
    digitalWrite(CSW, HIGH);
    setDBReadMode();
    //delayMicroseconds(10);
}

// Reads a byte from databus for vram access
unsigned char readByteFromVRAM()
{
    unsigned char memByte = 0;
    digitalWrite(MODE, LOW);
    digitalWrite(CSR, LOW);
    //delayMicroseconds(1);
    memByte = readPort();
    digitalWrite(CSR, HIGH);
    //delayMicroseconds(10);
    return memByte;
}

void setRegister(unsigned char registerIndex, unsigned char value)
{
    writeByte(value);
    writeByte(0x80 | registerIndex);
}

void setWriteAddress(unsigned int address)
{
    writeByte(address & 0xff);
    writeByte(0x40 | (address >> 8) & 0x3f);
}

void setReadAddress(unsigned int address)
{
    writeByte(address & 0xff);
    writeByte((address >> 8) & 0x3f);
}

int vdp_init(uint8_t mode, uint8_t color, bool big_sprites, bool magnify)
{
    vdp_mode = mode;
    sprite_size_sel = big_sprites;
    pinMode(MODE, OUTPUT);
    pinMode(RESET, OUTPUT);
    pinMode(CSW, OUTPUT);
    pinMode(CSR, OUTPUT);

    digitalWrite(RESET, HIGH);
    digitalWrite(MODE, HIGH);
    digitalWrite(CSW, HIGH);
    digitalWrite(CSR, HIGH);
    reset();
#ifdef RAMTEST
    // Test RAM
    setWriteAddress(0x0);
    for (int i = 0; i < 0x3fff; i++)
    {
        writeByteToVRAM(i);
    }
    setReadAddress(0x00);
    for (int i = 0; i < 0x3fff; i++)
    {
        if (readByteFromVRAM() != (uint8_t)i)
            return VDP_ERROR;
    }
#endif
    // Clear Ram
    setWriteAddress(0x0);
    for (int i = 0; i < 0x3FFF; i++)
        writeByteToVRAM(0);

    switch (mode)
    {
    case VDP_MODE_G1:
        setRegister(0, 0x00);
        setRegister(1, 0xC0 | (big_sprites << 1) | magnify); // Ram size 16k, activate video output
        setRegister(2, 0x05); // Name table at 0x1400
        setRegister(3, 0x80); // Color, start at 0x2000
        setRegister(4, 0x01); // Pattern generator start at 0x800
        setRegister(5, 0x20); // Sprite attriutes start at 0x1000
        setRegister(6, 0x00); // Sprite pattern table at 0x000
        sprite_pattern_table = 0;
        pattern_table = 0x800;
        sprite_attribute_table = 0x1000;
        name_table = 0x1400;
        color_table = 0x2000;
        color_table_size = 32;
        // Initialize pattern table with ASCII patterns
        setWriteAddress(pattern_table + 0x100);
        for (uint16_t i = 0; i < 768; i++)
            // writeByteToVRAM(patterns[i]);
            writeByteToVRAM(pgm_read_byte(ASCII + i));
        break;

    case VDP_MODE_G2:
        setRegister(0, 0x02);
        setRegister(1, 0xC0 | (big_sprites << 1) | magnify); // Ram size 16k, Disable Int, 16x16 Sprites, mag off, activate video output
        setRegister(2, 0x0E); // Name table at 0x3800
        setRegister(3, 0xFF); // Color, start at 0x2000
        setRegister(4, 0x03); // Pattern generator start at 0x0
        setRegister(5, 0x76); // Sprite attriutes start at 0x3800
        setRegister(6, 0x03); // Sprite pattern table at 0x1800
        pattern_table = 0x00;
        sprite_pattern_table = 0x1800;
        color_table = 0x2000;
        name_table = 0x3800;
        sprite_attribute_table = 0x3B00;
        color_table_size = 0x1800;
        setWriteAddress(name_table);
        for (uint16_t i = 0; i < 768; i++)
            writeByteToVRAM(i);
        break;

    case VDP_MODE_TEXT:
        setRegister(0, 0x00);
        setRegister(1, 0xD2); // Ram size 16k, Disable Int
        setRegister(2, 0x02); // Name table at 0x800
        setRegister(4, 0x00); // Pattern table start at 0x0
        pattern_table = 0x00;
        name_table = 0x800;
        crsr_max_x = 39;
        setWriteAddress(pattern_table + 0x100);
        for (uint16_t i = 0; i < 768; i++)
            // writeByteToVRAM(patterns[i]);
            writeByteToVRAM(pgm_read_byte(ASCII + i));
        vdp_textcolor(VDP_WHITE, VDP_BLACK);
        break;

    case VDP_MODE_MULTICOLOR:
        setRegister(0, 0x00);
        setRegister(1, 0xC8 | (big_sprites << 1) | magnify); // Ram size 16k, Multicolor
        setRegister(2, 0x05); // Name table at 0x1400
        // setRegister(3, 0xFF); // Color table not available
        setRegister(4, 0x01); // Pattern table start at 0x800
        setRegister(5, 0x76); // Sprite Attribute table at 0x1000
        setRegister(6, 0x03); // Sprites Pattern Table at 0x0
        pattern_table = 0x800;
        name_table = 0x1400;
        setWriteAddress(name_table); // Init name table
        for (uint8_t j = 0; j < 24; j++)
            for (uint16_t i = 0; i < 32; i++)
                writeByteToVRAM(i + 32 * (j / 4));
      
        break;
    default:
        return VDP_ERROR; // Unsupported mode
    }
    //vdp_set_bdcolor(VDP_WHITE);
    //vdp_textcolor(VDP_BLACK);
    setRegister(7, color);

    /*setWriteAddress(sprite_attribute_table);
    for(uint16_t i = 0; i<128; i++)
        writeByteToVRAM(208);*/
    return VDP_OK;
}

void vdp_colorize(uint8_t fg, uint8_t bg)
{
    if (vdp_mode != VDP_MODE_G2)
        return;
    uint16_t name_offset = cursor.y * (crsr_max_x + 1) + cursor.x; // Position in name table
    uint16_t color_offset = name_offset << 3;                      // Offset of pattern in pattern table
    setWriteAddress(color_table + color_offset);
    for (uint8_t i = 0; i < 8; i++)
        writeByteToVRAM((fg << 4) + bg);
}

void vdp_plot_hires(uint8_t x, uint8_t y, uint8_t color1, uint8_t color2)
{
    uint16_t offset = 8 * (x / 8) + y % 8 + 256 * (y / 8);
    setReadAddress(pattern_table + offset);
    uint8_t pixel = readByteFromVRAM();
    setReadAddress(color_table + offset);
    uint8_t color = readByteFromVRAM();
    if(color1 != NULL)
    {
        pixel |= 0x80 >> (x % 8); //Set a "1"
        color = (color & 0x0F) | (color1 << 4); 
    }
    else
    {
        pixel &= ~(0x80 >> (x % 8)); //Set bit as "0"
        color = (color & 0xF0) | (color2 & 0x0F);
    }
    setWriteAddress(pattern_table + offset);
    writeByteToVRAM(pixel);
    setWriteAddress(color_table + offset);
    writeByteToVRAM(color);
}

void vdp_plot_color(uint8_t x, uint8_t y, uint8_t color)
{
    if (vdp_mode == VDP_MODE_MULTICOLOR)
    {
        uint16_t addr = pattern_table + 8 * (x / 2) + y % 8 + 256 * (y / 8);
        setReadAddress(addr);
        uint8_t dot = readByteFromVRAM();
        setWriteAddress(addr);
        if (x & 1) // Odd columns
            writeByteToVRAM((dot & 0xF0) + (color & 0x0f));
        else
            writeByteToVRAM((dot & 0x0F) + (color << 4));
    }
    else if (vdp_mode == VDP_MODE_G2)
    {
        // Draw bitmap
        uint16_t offset = 8 * (x / 2) + y % 8 + 256 * (y / 8);
        setReadAddress(color_table + offset);
        uint8_t color_ = readByteFromVRAM();
        if((x & 1) == 0) //Even 
        {
            color_ &= 0x0F; 
            color_ |= (color << 4);
        }
        else
        {
            color_ &= 0xF0;
            color_ |= color & 0x0F;
        }
        setWriteAddress(pattern_table + offset);
        writeByteToVRAM(0xF0);
        setWriteAddress(color_table + offset);
        writeByteToVRAM(color_);
        // Colorize
    }
}

void vdp_set_sprite_pattern(uint8_t number, const uint8_t *sprite)
{

    if(sprite_size_sel)
    {
        setWriteAddress(sprite_pattern_table + 32*number);
        for (uint8_t i = 0; i<32; i++)
        {
            writeByteToVRAM(sprite[i]);
        }
    }
    else
    {
        setWriteAddress(sprite_pattern_table + 8*number);
        for (uint8_t i = 0; i<8; i++)
        {
            writeByteToVRAM(sprite[i]);
        }
    }

}

void vdp_sprite_color(uint16_t addr, uint8_t color)
{
    setReadAddress(addr + 3);
    uint8_t ecclr = readByteFromVRAM() & 0x80 | (color & 0x0F);
    setWriteAddress(addr + 3);
    writeByteToVRAM(ecclr);
}

Sprite_attributes vdp_sprite_get_attributes(uint16_t addr)
{
    Sprite_attributes attrs;
    setReadAddress(addr);
    attrs.y = readByteFromVRAM();
    attrs.x = readByteFromVRAM();
    attrs.name_ptr = readByteFromVRAM();
    attrs.ecclr = readByteFromVRAM();
    return attrs;
}

void vdp_sprite_get_position(uint16_t addr, uint16_t &xpos, uint8_t &ypos)
{
    setReadAddress(addr);
    ypos = readByteFromVRAM();
    uint8_t x = readByteFromVRAM();
    readByteFromVRAM();
    uint8_t eccr = readByteFromVRAM();
    xpos = eccr & 0x80 ? x : x+32;
}

uint16_t vdp_sprite_init(uint8_t name, uint8_t priority, uint8_t color)
{
    uint16_t addr = sprite_attribute_table + 4*priority;
    setWriteAddress(addr);
    writeByteToVRAM(0); 
    writeByteToVRAM(0);
    if(sprite_size_sel)
        writeByteToVRAM(4*name);
    else
        writeByteToVRAM(name);
    writeByteToVRAM(0x80 | (color & 0xF));
    return addr;
}

uint8_t vdp_sprite_set_position(uint16_t addr, uint16_t x, uint8_t y)
{
    uint8_t ec, xpos;
    if (x < 144)
    {
        ec = 1;
        xpos = x;
    }
    else
    { 
        ec = 0;
        xpos = x-32;
    }
    setReadAddress(addr + 3);
    uint8_t color = readByteFromVRAM() & 0x0f;

    setWriteAddress(addr);
    writeByteToVRAM(y);
    writeByteToVRAM(xpos);
    setWriteAddress(addr + 3);
    writeByteToVRAM((ec << 7) | color);
    return read_status_reg();
}

void vdp_print(String text)
{
    for (uint16_t i = 0; text[i]; i++)
    {
        switch (text[i])
        {
        case '\n':
            vdp_set_cursor(cursor.x, ++cursor.y);
            break;
        case '\r':
            vdp_set_cursor(0, cursor.y);
            break;
        case '\033':
        {
            // Serial.println("Color Change");
            String c = text.substring(i + 2, text.indexOf("m", i));
            uint8_t fc = c.toInt();
            uint8_t bc = 0;
            if (c.indexOf(";") > 0)
                bc = c.substring(c.indexOf(";") + 1).toInt();
            vdp_textcolor(fc, bc);
            text.remove(i, c.length() + 3);
            i--;
        }
        break;
        default:
            vdp_write(text[i]);
            vdp_colorize(fgcolor, bgcolor);
            vdp_set_cursor(VDP_CSR_RIGHT);
        }
    }
}

void vdp_set_bdcolor(uint8_t color)
{
    setRegister(7, color);
}

void vdp_set_pattern_color(uint16_t index, uint8_t fg, uint8_t bg)
{
    if (vdp_mode == VDP_MODE_G1)
    {
        index &= 31;
    }
    setWriteAddress(color_table + index);
    writeByteToVRAM((fg << 4) + bg);
}

void vdp_set_cursor(uint8_t col, uint8_t row)
{
    if (col == 255) //<0
    {
        col = crsr_max_x;
        row--;
    }
    else if (col > crsr_max_x)
    {
        col = 0;
        row++;
    }

    if (row == 255)
    {
        row = crsr_max_y;
    }
    else if (row > crsr_max_y)
    {
        row = 0;
    }

    cursor.x = col;
    cursor.y = row;
}

void vdp_set_cursor(uint8_t direction)
{
    switch (direction)
    {
    case VDP_CSR_UP:
        vdp_set_cursor(cursor.x, cursor.y - 1);
        break;
    case VDP_CSR_DOWN:
        vdp_set_cursor(cursor.x, cursor.y + 1);
        break;
    case VDP_CSR_LEFT:
        vdp_set_cursor(cursor.x - 1, cursor.y);
        break;
    case VDP_CSR_RIGHT:
        vdp_set_cursor(cursor.x + 1, cursor.y);
        break;
    }
}

void vdp_textcolor(uint8_t fg, uint8_t bg)
{
    fgcolor = fg;
    bgcolor = bg;
    if (vdp_mode == VDP_MODE_TEXT)
        setRegister(7, (fg << 4) + bg);
}

void vdp_write(uint8_t chr)
{
    uint16_t name_offset = cursor.y * (crsr_max_x + 1) + cursor.x; // Position in name table
    uint16_t pattern_offset = name_offset << 3;                    // Offset of pattern in pattern table
    if (vdp_mode == VDP_MODE_G2)
    {
        setWriteAddress(pattern_table + pattern_offset);
        for (uint8_t i = 0; i < 8; i++)
        {
            // writeByteToVRAM(patterns[((chr - 32) << 3) + i]);
            writeByteToVRAM(pgm_read_byte(ASCII + (((chr - 32) << 3) + i)));
        }
    }
    else // G1 and text mode
    {
        setWriteAddress(name_table + name_offset);
        writeByteToVRAM(chr);
    }
}

//Wrapper functions
int vdp_init_textmode(uint8_t fgcolor, uint8_t bgcolor)
{
    return vdp_init(VDP_MODE_TEXT, (fgcolor<<4) | (bgcolor & 0x0f), 0, 0);
}

int vdp_init_g1(uint8_t fgcolor, uint8_t bgcolor)
{
    return vdp_init(VDP_MODE_G1, (fgcolor<<4) | (bgcolor & 0x0f), 0, 0);
}

int vdp_init_g2(bool big_sprites, bool scale_sprites)
{
    return vdp_init(VDP_MODE_G2, 0x0, big_sprites, scale_sprites);
}

int vdp_init_multicolor()
{
    return vdp_init(VDP_MODE_MULTICOLOR, 0, 0, 0);
}
