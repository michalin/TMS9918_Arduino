#include <tms9918.h>


void g1text()
{
    vdp_init_textmode(VDP_BLACK, VDP_WHITE);
    //vdp_init_textmode();

    //vdp_textcolor(VDP_DARK_BLUE, VDP_WHITE);
    vdp_print("Character set in graphic mode 1:\r\n\r\n");
    vdp_print(" !\"#$%&'()*+,-./\r\r");
    vdp_print("0123456789:;<=>?@\r\n");
    vdp_print("ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_'\r\n");
    vdp_print("abcdefghijklmnopqrstuvwxyz{>}+~\r\n");
return;
    while (1)
    {
        for (uint8_t i = 1; i <= 15; i++)
            for (uint8_t j = 1; j <= 15; j++)
            {
                vdp_textcolor(j, i);
                delay(1000);
            }
    }
}
