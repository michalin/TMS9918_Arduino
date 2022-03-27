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

### Compilation
The example comes with precompiled binaries, but the source file can be compiled with `g++ imgserial.cpp -o imgserial.<extension> <-static>`

On Windows, Cygwin with Gnu C++ compilers (GCC) must be installed. The executable needs the cygwin1.dll or a Cygwin environment to run. If there is no Cygwin environment on the target machine, the Windows libraries needs to be statically linked into the executable by the *-static* option. For example `g++ imgserial.cpp -o imgserial.exe -static`.



