# vw-can-bus-gauge-cluster-controller
VW Gauge Cluster Can-Bus Controller for 2003 ~ 2008 Car Models, with support for OutGauge protocol \
https://en.wikipedia.org/wiki/CAN_bus \
https://en.lfsmanual.net/wiki/OutSim_/_OutGauge

demonstration ( Frontend: flat HTML file + XHR requests, Backend: nodejs, builds a udp packet for emulating OutGauge protocol ) : 
https://www.npmjs.com/package/python-struct

https://user-images.githubusercontent.com/5550633/171330847-7dcb1376-1a37-44a3-a299-bd44562d5628.mp4

better quality video https://youtu.be/KV-soxbjtZg

BeamNG.drive via built-in OutGauge protocol ( connects directly to nodemcu8266 via wifi )

https://user-images.githubusercontent.com/5550633/171325488-8fdae158-e063-4dcd-a315-125b10d569fb.mp4

better quality video https://youtu.be/kCUtLtM_2gA

connector - gauge cluster side:

<img src="https://user-images.githubusercontent.com/5550633/171311362-77bd7454-941a-4215-82d2-32f43267ce99.JPG" width="800" >

bottom left: pin 1 \
pin 1: +12v \
pin 19: gnd \
pin 5: handbrake \
pin 12: CAN low \
pin 13: CAN high \
pin 18: fuel low \
pin 19 fuel high \
pin 25: engine oil pressure ( 12v no pressure, 0v oil pressure ok, normally 12v )

can bus is used for most functions; speed, rpm, oil temperature, left/right/warning signals, abs, airbag, low battery, doors, and more

prototype custom connector:

<img src="https://user-images.githubusercontent.com/5550633/171311368-a8306e7b-f1d2-46ca-8e2e-f535ff82c6f0.JPG" width="800" >

<img src="https://user-images.githubusercontent.com/5550633/171311382-5b0a6b8f-f8f4-429f-a112-0eeb138587e7.JPG" width="800" >

pin 18+19 are usually connected to a potentiometer tied to a float in the fuel tank to indicate the level of the fuel, those two capacitors smooth out a PWM signal to emulate the behaviour of that fuel level potentiometer.

nodemcu 8266 + Can Bus Module

https://en.wikipedia.org/wiki/NodeMCU

<img src="https://user-images.githubusercontent.com/5550633/171311393-8b55fded-039c-499a-bb3b-b4760f32f44a.JPG" width="800" >

https://cdn-reichelt.de/documents/datenblatt/A300/SBC-CAN01-MANUAL-20201021.pdf

<img src="https://user-images.githubusercontent.com/5550633/171311401-f3eb388d-0a0d-4f60-8c0c-83ddd5d5d1b6.JPG" width="800" >

## Can Bus Messages

0x288 2nd byte: oil temperature mapping as follows : 50-90 degrees => 130 - 170, 90-130 degrees => 207 - 240 \
0x635 2nd byte: backlight level 0-100 \
0x470 1st byte: battery warning, 2nd byte: doors status, 3rd byte: light bulb error \
0xDA0 comes from ABS module, 2nd and 3rd bytes indicates car speed, 6th and 7th bytes indicate distance traveled ( WIP ) \
0x1A0 comes form ABS module, indicates ABS status \
0x050 Airbag status \
0x280 3rd and 4th bytes, comes from ECU module, indicates RPM and engine status \
0x3D0 steering wheel immobilizer status \
0x531 1st byte 0xf for blue high beam indicator, 3rd byte: left 0b001, right 0b110, warning signals 0b111

## high level data flow 

![image](https://user-images.githubusercontent.com/5550633/171321026-8ef68d97-1e15-4c83-bf5b-1832bc7ab31f.png)

# schematics 

wip
