status_pixel
============

This little arduino program allows you to use 
an Arduino with Neopixels as a status display.

Just connect the Arduino to your computer and
send coloring commands via serial communication.

Example of a serial sequence:
L00,0F,00,00
L01,00,0F,00
L02,00,00,0F
S
Expected Result: First LED shows red, second green, third blue