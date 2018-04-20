# 433MHz-MQTT-Bridge
Dedicated Processor to receive 433MHz signals from a switch and to translate to MQTT commands

I have lots of intelligent nodes/IOTs (ESP8266s) around my living room and of course I can switch lights on and off through Domoticz or Alexa.
My wife though loves common wall switches. A good solution are these 433Mhz wall mount switches. The program running on a Wemos D1 mini 
translates the code sent by the switch in a MQTT toggle command. Instead of having a 433MHz receiver on all my nodes this program 
runs on a dedicated Wemos (Could also be added to an existing node). No change needed to all my existing nodes.

### Hardware requirements:
* 1 x Wemos D1 mini
* 1 x 1000uF capacitor between GND and 5V (I use this with all my Wemos project, it helps to stabilize operation)
* 1 x 433MHz receiver 
* 1 x Power supply 5V 0.75A
* appropriate number of 433MHz switches https://www.aliexpress.com/item/86-Wall-Panel-Remote-Transmitter-1-2-3-Button-Sticky-RF-Remote-Smart-Home-Room-Living/32839079801.html?spm=a2g0s.9042311.0.0.Xv4t8Q
* a project box
