# Thermocouple-meter-PlatformIO
Thermocouple thermometer for Arduino under PlatformIO. Written in the afternoon of Eastern Monday.

I had the need to use a thermocouple to verify a little reflow oven.

So instead of buying one I checked what I have in house:

- Arduino Nano
- Display LCD 16x2 
- 18650 LiPo battery holder/charger with 5V output
- a legacy switch
- Encoder Knob (can be pushed)

I added a Max6675 based thermocpuple bought on banggood.

The software is based on the Arduino framework but using PlatformIO in Visual Studio Code (sorry but the Arduino IDE is really not usable)

Current features (first release):
- User interface controlled with the encoder knob (left/right/push)
- Temperature sampled each second
- Log measured Max and Min temperature
- Set/Reset offset
- Offset is stored on Eeprom and resists a restart


Next version will also allow to store values in memory by pushing the knob during measures

Enjoy
