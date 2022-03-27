#include <tms9918.h>

bool loaded = false;

void serialEvent()
{
    uint16_t y = 0, n_cols, n_lines;
    uint8_t line[256];
    delay(10);
    n_cols = Serial.read();
    n_lines = Serial.read();
    n_cols == 0 ? n_cols = 256 : n_cols;
    if (n_cols == 256) //Hi-res
    {
        vdp_init_g2();
        vdp_set_bdcolor(VDP_BLACK);
        while (y < 192)
        {
            Serial.readBytes(line, 256);
            uint8_t color1 = 0, color2 = 0;
            for (int x = 0; x < n_cols; x++)
            {
                if(x%8 == 0)
                    color1 = color2 = line[x];
                else if(line[x] != color1)
                    color2 = line[x];

                if(line[x] == color1)    
                    vdp_plot_hires(x, y, color1+1);
                else
                    vdp_plot_hires(x, y, NULL, color2+1);
            }
            y++;
            Serial.write('@');
        }
        //sprites();
    } 
    else if(n_cols == 64)
    {
        vdp_init_multicolor();
        while (y < 64)
        {
            Serial.readBytes(line, 64);
            for (int x = 0; x < 64; x++)
            {
                vdp_plot_color(x, y, 1+line[x]);
            }
            y++;
            Serial.write('@');
        }
    }
    else 
    {
        vdp_print("Unknown format");

    }
}

void g2image()
{
    Serial.begin(115200);
    vdp_init_textmode();
    vdp_print("Start the imgserial tool on PC to \r\nsend a picture");
    //Serial.write('@');

}
