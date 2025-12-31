# Word clock

## Printing

Open `print/woordklok.scad` in OpenSCAD. Adjust `txt` and `text_font`. Export the three parts by setting `draw_main`, `draw_light_cover`, `draw_back_cover` to true (the others false).

Slicing/printing notes:
  - back_cover
    - print upside down
  - light cover
    - print upside down
  - main
    - add a color change so the bottom (front) is white, and the rest is a color of choice
    - use paint-on fuzzy skin to apply fuzzy skin to the sides, but not the white part
    - optionally, use support enforcing to add supports to the USB port and/or LDR sensor hole

## BOM
 - APA102 LED strip (preferred), or Ws2812b or similar. 60LEDs/meter, length 100 LEDs.
 - USB C socket
 - ESP8266 or ESP32 board
 - 4.7k resistor
 - LDR type 5506
 - thin, flexbile wires

## Assembly

Tape the led strip to the `light_cover` part in a horizontal zig zag pattern. The data signal starts at the top left corner. Connect the led strips using short (!) wires.

Shown visually:
```
-->>>>>>>>>>-\
             |
/-<<<<<<<<<<-/
|
\->>>>>>>>>>-\
             |
  <<<<<<<<<<-/
```

Wire the LDR as follows:
```
GND               LDR_PIN          3v3
 |                   |              |
 \-- 4.7k resistor --+-- LDR 5506 --/
```

## Pins
Wemos D1 mini:
 - LED data GPIO 13 (D7)
 - LED clock GPIO 14 (D5)
 - LDR GPIO 17 (A0)
