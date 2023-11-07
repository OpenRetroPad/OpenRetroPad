OpenRetroPad
------------

[GrechTech fork] Minor alterations to map pins to those used by BlueRetro. Changes pins for ESP32 Genesis/MS, Saturn, SNES/NES and PSX. **To compile for BlueRetro units such as RetroRosetta, use "bluetooth-hid gamepad" output (out-bt).**  

Adapt various input devices to various output devices.

Currently supported inputs: SNES/NES, Sega Genesis/Megadrive/Atari, Sega Saturn, Playstation (and PS2) Digital and Dual shock, Nintendo 64, Nintendo Gamecube, Nintendo Wii Nunchuck/Wii Classic/SNES+NES Mini

Currently supported outputs : bluetooth-hid gamepad, usb-hid gamepad, nintendo switch usb gamepad, wireless usb-hid gamepad over radio

OpenRetroPad is written and tested on an Arduino Pro Micro and an ESP32, there is also an in-progress custom PCB being developed at [OpenRetroPadHW](https://github.com/OpenRetroPad/OpenRetroPadHW)

Build using [PlatformIO](https://platformio.org/) using `pio run` or `pio run -e $board-$input-$output` for a specific target/env.

env's are laid out like `$board-$input-$output`
supported values:
  * $board: micro, esp32
  * $input: snes, genesis, saturn, psx, n64, gc, wii, radio, debug
  * $output: radio, usb, usbradio, switchusb, bt, debug
  * please note not all boards are compatible with all inputs/outputs, for example esp32 can only do bt, micro can only do radio or usb

Wiring
------

| GTRR Pins| OR DB-25 Pins | Arduino Pro Micro GPIO | ESP32 GPIO | Radio    | SNES    | PSX    | N64      | Gamecube | Genesis     | Dreamcast | Saturn      | Wii Ext  |
|----------|---------------|------------------------|------------|----------|---------|--------|----------|----------|-------------|-----------|-------------|----------|
|   6B     |  1  TX        |  1                     | 19         | -        | LATCH   | -      | -        | -        | P1-1        | P1-DATA1  | P1-7        | -        |
|   5B     |  2  SDA       |  2                     | 21         | -        | CLOCK   | DATA   | P1-DATA  | P1-DATA  | P1-3        | P1-DATA5  | P1-2        | SDA      |
|   2B     |  3  SCL       |  3                     | 22         | -        | P1-DATA | CMD    | -        | -        | P1-4        | -         | P1-3        | SCL      |
|  N/A     |  4  Analog ** |  4                     | 15         | -        | -       | -      | -        | -        | -           | -         | -           | -        |
|  16B     |  5  Digital   |  5                     | 16         | -        | P3-DATA | CLK    | -        | -        | P1-7        | -         | PX-5        | -        |
|  N/A     |  6  Analog ** |  6                     |  2         | -        | -       | -      | -        | -        | -           | -         | -           | -        |
|  11B     |  7  Digital   |  7                     | 17         | CE       | -       | -      | -        | -        | P2-7*       | -         | -           | -        |
|  N/A     |  8  Analog    |  8                     |  4         | CSN      | -       | -      | -        | -        | -           | -         | -           | -        |
|  18B     |  9  Analog    |  9 > 1k Ω              | 35 > 1k Ω  | -        | 330 Ω   | 100 Ω  | 220 Ω    | 680 Ω    | 470 Ω       | 820 Ω     | 1000 Ω      | 1500 Ω   |
|  17B     | 10  Analog    | 10                     | 32         | -        | P2-DATA | ATT    | -        | -        | -           | -         | P2-6        | -        |
|  N/A     | 11  RX        |  0                     | 18         | -        | P4-DATA | -      | -        | -        | P1-2        | -         | P1-8        | -        |
|  -       | 12  -         | -                      | -          | -        | -       | -      | -        | -        | -           | -         | -           | -        |
|  -       | 13  -         | -                      | -          | -        | -       | -      | -        | -        | -           | -         | -           | -        |
|  N/A     | 14  MISO **   | 14                     | 12         | MISO     | -       | -      | -        | -        | -           | -         | -           | -        |
|  N/A     | 15  SCLK **   | 15                     | 14         | SCLK     | -       | -      | -        | -        | -           | -         | -           | -        |
|  N/A     | 16  MOSI **   | 16                     | 13         | MOSI     | -       | -      | -        | -        | -           | -         | -           | -        |
|  -       | 17  -         | -                      | -          | -        | -       | -      | -        | -        | -           | -         | -           | -        |
|  14B     | 18  Analog    | 18                     | 27         | -        | -       | -      | -        | -        | P2-1        | -         | P2-7        | -        |
|  13B     | 19  Analog    | 19                     | 26         | -        | -       | -      | -        | -        | P2-2        | -         | P2-8        | -        |
|  15B     | 20  Analog    | 20                     | 25         | -        | -       | -      | -        | -        | P2-3        | -         | P2-2        | -        |
|  16B     | 21  Analog    | 21                     | 33         | -        | -       | -      | -        | -        | P2-4        | -         | P2-3        | -        |
|  -       | 22  -         | -                      | -          | -        | -       | -      | -        | -        | -           | -         | -           | SENSE    |
|  20A     | 23  3.3V VCC  | -                      | 3.3V VCC   | 3.3V VCC | -       | -      | 3.3V VCC | 3.3V VCC | -           | -         | -           | 3.3V VCC |
|  19D     | 24  5V VCC    | 5V VCC OUT             | 5V VCC     | 5V VCC   | 5V VCC  | 5V VCC | -        | 5V VCC   | PX-5 5V VCC | 5V VCC    | PX-1 5V VCC | -        |
|  18A     | 25  GND       | GND                    | GND        | GND      | GND     | -      | GND      | GND      | PX-8 GND    | GND       | PX-9 GND    | GND      |
|  17A     | 25  GND       | GND                    | GND        | GND      | GND     | -      | GND      | GND      | PX-8 GND    | GND       | PX-9 GND    | GND      |
|   1B     | 26  RR EXT 1  | -                      | 23         | -        | -       | -      | -        | -        | -           | -         | P1-6        | -        |
|   8B     | 27  RR EXT 2  | -                      | 5          | -        | -       | -      | -        | -        | -           | -         | PX-4        | -        |
|  19B     | 28  RR EXT 3  | -                      | 34         | -        | -       | -      | -        | -        | P1-6        | -         | -           | -        |
|  18B     | 29  RR EXT 4  | -                      | 35         | -        | -       | -      | -        | -        | P1-9        | -         | -           | -        |
|  12B     | 30  RR EXT 5  | -                      | 36         | -        | -       | -      | -        | -        | P2-6        | -         | -           | -        |
|  20B     | 31  RR EXT 6  | -                      | 39         | -        | -       | -      | -        | -        | P2-9        | -         | -           | -        |

\* 2nd player Genesis is incompatible with Radio because it uses the same pins, 1 player Genesis is compatible  
** Not connected on RR

Ω This is optional and only used for dongle detection [Not yet implemented]. On the microcontroller side, put a 1k resistor between DB-25 pin 9 and VCC (3.3v for ESP32, 5V for Micro). On each controller dongle, put a resistor of the given value between DB-25 pin 9 and GND.

All connected pins can also function as Digital pins (RR EXT 3 - 6 input only).

Credits / Links
---------------

Code and/or inspiration was (or will be) taken from these places, in no particular order:

  * [ESP32-BLE-Gamepad](https://github.com/lemmingDev/ESP32-BLE-Gamepad)
  * [DaemonBite-Retro-Controllers-USB](https://github.com/MickGyver/DaemonBite-Retro-Controllers-USB)
  * [Arduino-USB-HID-RetroJoystickAdapter](https://github.com/mcgurk/Arduino-USB-HID-RetroJoystickAdapter)
  * [BlueCubeMod](https://github.com/NathanReeves/BlueCubeMod)
  * [OpenSwitchPad](https://github.com/agustincampeny/OpenSwitchPad)
  * [arduino-n64-controller-library](https://github.com/pothos/arduino-n64-controller-library)
  * [N64toiQue](https://github.com/mnzlmstr/N64toiQue)
  * [dreamcast_usb](https://github.com/raphnet/dreamcast_usb)
  * [SNES-NRF24](https://github.com/baldengineer/SNES-NRF24)
  * https://github.com/NicoHood/Nintendo
  * https://github.com/darthcloud/BlueRetro
  * https://github.com/raphnet/gc_n64_usb-v3
