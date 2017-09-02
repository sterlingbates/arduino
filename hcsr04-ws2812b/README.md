# About
This code works with an HC-SR04 sonic distance measuring component and a WS2812B individually-addressable LED strip.

The general idea is to measure the range to an object, determine where that object sits relative to the full range of the distance finder, and to light a correspondingly relative number of LEDs on the strip. As the object moves closer or further away, more or fewer LEDs are lit.

This code also smooths the typical jitter that distance finders exhibit by averaging reported distances inside a small buffer.
