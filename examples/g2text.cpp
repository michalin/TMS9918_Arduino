#include <tms9918.h>

void g2text()
{
    if (vdp_init_g2())
        Serial.println("VDP Error");

    vdp_textcolor(VDP_GRAY, VDP_BLACK);
    vdp_print("BLACK (1)                       ");
    vdp_textcolor(VDP_BLACK, VDP_GRAY);
    vdp_print("GRAY (14)                       ");
    vdp_textcolor(VDP_BLACK, VDP_WHITE);
    vdp_print("WHITE (15)                      ");
    vdp_textcolor(VDP_GRAY, VDP_MAGENTA);
    vdp_print("MAGENTA (13)                    ");
    vdp_textcolor(VDP_GRAY, VDP_DARK_BLUE);
    vdp_print("DARK BLUE (4)                   ");
    vdp_textcolor(VDP_GRAY, VDP_LIGHT_BLUE);
    vdp_print("LIGHT BLUE (5)                  ");
    vdp_textcolor(VDP_BLACK, VDP_CYAN);
    vdp_print("CYAN (7)                        ");
    vdp_textcolor(VDP_GRAY, VDP_DARK_GREEN);
    vdp_print("DARK GREEN (12)                 ");
    vdp_textcolor(VDP_GRAY, VDP_MED_GREEN);
    vdp_print("MEDIUM GREEN (2)                ");
    vdp_textcolor(VDP_BLACK, VDP_LIGHT_GREEN);
    vdp_print("LIGHT GREEN (3)                 ");
    vdp_textcolor(VDP_BLACK, VDP_DARK_YELLOW);
    vdp_print("DARK YELLOW (10)                ");
    vdp_textcolor(VDP_BLACK, VDP_LIGHT_YELLOW);
    vdp_print("LIGHT YELLOW (11)               ");
    vdp_textcolor(VDP_GRAY, VDP_DARK_RED);
    vdp_print("DARK RED (6)                    ");
    vdp_textcolor(VDP_GRAY, VDP_MED_RED);
    vdp_print("MEDIUM RED (8)                  ");
    vdp_textcolor(VDP_BLACK, VDP_LIGHT_RED);
    vdp_print("LIGHT RED (9)                   ");
    vdp_print("\033[0;1m ------------------------------ ");
    vdp_print("!\033[1;0m                              \033[0;1m!");
    vdp_print("!\033[1;0m                              \033[0;1m!");
    vdp_print("!\033[1;0m                              \033[0;1m!");
    vdp_print("!\033[1m         TRANSPARENT (0)      \033[0;1m!");
    vdp_print("!\033[1;0m                              \033[0;1m!");
    vdp_print("!\033[1;0m                              \033[0;1m!");
    vdp_print("!\033[1;0m                              \033[0;1m!");
    vdp_print(" ------------------------------ ");

    uint8_t j;
    while(1)
    {
        vdp_set_bdcolor(j++);
        delay(2000);
    }
}
