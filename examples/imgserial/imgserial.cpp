/* Tool to send for Gimp raw data files an Arduino running the g2image example
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
#include <stdio.h>
#include <stdint.h>
#include <cstring>
#include <string>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/stat.h>

using namespace std;

int main(int argc, const char *argv[])
{
    termios tty;
    tty.c_iflag = 0;
    tty.c_oflag = 0;
    tty.c_cflag = CS8 | CREAD | CLOCAL; // 8 Bit, No parity, one stop bit, no flow control, disable signal lines, Read enabled
    tty.c_lflag = 0;
    tty.c_cc[VMIN] = 5; // Block until at least one byte received
    tty.c_cc[VTIME] = 0;
    cfsetspeed(&tty, B115200);

    if (argc != 3)
    {
        printf("This program sends GIMP raw data files to an Arduino running the \"g2image\" example over serial(USB) interface\r\n");
        printf("\r\nUsage: imgserial filename.data port \r\n");
#ifdef __CYGWIN__
        printf("Example: imgserial parrot.data /dev/ttyS0 where /dev/ttyS0 is COM1, /dev/ttyS1 COM2 etc. \r\n");
#else
        printf("Example: imgserial parrot.data /dev/ttyUSB0 \r\n");
#endif
        return -1;
    }
#ifdef __CYGWIN__
        printf("IMPORTANT: Put a 10µF cap between the RST pin of your Arduino and Ground to prevent a reboot.\r\n");
#endif


    string fname_in = argv[1];
    if (fname_in.rfind(".data", string::npos) == -1)
    {
        printf("Invalid filename. Must be a *.data file\r\n");
        return -1;
    }

    // FILE *infile = fopen(fname_in.c_str(), "r");
    int infile = open(fname_in.c_str(), O_RDONLY);
    if (errno)
    {
        printf("Error opening file: %s\r\n", strerror(errno));
        return -1;
    }

    // Opening serial port
    // int port = open("/dev/ttyUSB0", O_RDWR);
    int port = open(argv[2], O_RDWR);
    if (errno)
    {
        printf("Error opening serial Port: %s\n", strerror(errno));
        return -1;
    }

    // Setting port attributes
    if (tcsetattr(port, TCSANOW, &tty) != 0)
    {
        printf("Error setting serial port: %s\n", strerror(errno));
        return -1;
    }

    // Determine filesize
    struct stat buf;
    fstat(infile, &buf);
    size_t filesize = buf.st_size;

    uint16_t n_cols, n_lines;
    switch (filesize)
    {
    case 64 * 48:
        printf("\r\n64x48 Multicolor Mode\r\n");
        n_cols = 64;
        n_lines = 48;
        break;
    case 256 * 192:
        printf("\r\n256x192 Graphic Mode 2 high resolution\r\n");
        n_cols = 256;
        n_lines = 192;
        break;
    default:
        printf("\r\nInvalid file format. Size: %d \r\n", (int)filesize);
        return -1;
    }
    uint8_t ack;
    // read(port, &ack, 1); //Block until Arduino is ready
    printf("Transfer started: %s to %s...\r\n", argv[1], argv[2]);
    write(port, (uint8_t *)&n_cols, 1); // Send resolution
    write(port, (uint8_t *)&n_lines, 1);
    uint8_t *inbuf = (uint8_t *)calloc(1, filesize);
    if (read(infile, inbuf, filesize) < 1)
    {
        printf("Error reading file: %s\r\n", strerror(errno));
        return -1;
    }

    for (int i = 0; i < n_lines; i++)
    {
        write(port, (const void *)(inbuf + n_cols * i), n_cols);
        read(port, &ack, 1); // Block until Arduino acknowledged that he processed the packet
    }

    close(port);
    close(infile);
    printf("\r\nFile sent\r\n");

    return 0;
}
