# Imgserial Tool

## Commandline tool to send GIMP raw data files over USB or Serial

### Windows
On a Command Prompt type `imgserial file.data port`.

Example: `imgserial parrot.data /dev/ttyS0` if the serial port is COM1.
* COM2 --> /dev/ttyS1
* COM3 --> /dev/ttyS2
* etc

It is important to have a 10µF Capacitor between the RST pin of the Arduino and Ground. Otherwise the Arduino reboots every time the USB transfer starts.

### Linux
In a shell type `./imgserial.linux file.data port`.
Port is the same as used by the Arduino IDE, for example /dev/USB0.
A capacitor is not needed here.
***
## Compilation
The example comes with precompiled binaries, but the source file can be compiled with `g++ imgserial.cpp -o imgserial.<extension> <-static>`

On Windows, Msys64 with Gnu C compilers (GCC) must be installed. The executable needs the cygwin1.dll or a Cygwin environment to run. If there is no Cygwin environment on the target machine, the Windows libraries needs to be statically linked into the executable by the *-static* option. For example `g++ imgserial.cpp -o imgserial.exe -static`.

***
## Create data files from images
I recommend Gimp to convert the images. Before you can proceed, import the *tms9918.gpl* color palette file into Gimp.

1. Crop and scale the image until it has a size of 256x192 pixels
2. Open Jannone's MSX Screen Image Converter https://msx.jannone.org/conv/
3. Upload the file (Must be JPG, PNG or GIF; up to 400 kb). Select `"MSX 1 Colors"` and try the options below until you find the one that looks best <br>
![screenshot](/html/msxconv1.jpg)

4. Download the preview image <br>
![screenshot](/html/msxconv2.jpg)

5. Open Preview in Gimp. Assign the *tms9918* color palette: ```Image->Mode->Indexed...``` Select *tms9918.gpl* as custom palette in the dialog and click ```Convert```. <br>
![screenshot](/html/gimp1.JPG)

6. Export image as raw image data file ```(File->Export)```. <br>
![screenshot](/html/export.JPG)

The .data file must have a size of 48KB