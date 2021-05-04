IRTransmitter command JSON:
{
  "repeat": int,
  "protocol": string("SAMSUNG", "OTHER"),
  "signal": string
}



Arduino docker installation:

1. docker image: https://github.com/tombenke/darduino

2. instal CH340 drivers: https://learn.sparkfun.com/tutorials/how-to-install-ch340-drivers/all
  - reboot after: sudo usermod -a -G dialout $username

3. instal board https://www.instructables.com/Programming-the-WeMos-Using-Arduino-SoftwareIDE/

Correct setup:
crw-rw-rw-   1 root     dialout 188,     0 mar 20 14:40 ttyUSB0

Troubleshooting:
/dev/ttyUSB0: device is a folder: remove from /dev and reconnect device
Permission denied: sudo chmod a+rw /dev/ttyUSB0
crw-rw----   1 root     dialout 188,     0 mar 20 14:50 ttyUSB0: sudo chmod a+rw /dev/ttyUSB0
Failed to connect to ESP8266: Timed out waiting for packet header: connect D3 to GND (https://stackoverflow.com/questions/58945245/wemos-d1-mini-esptool-fatalerror-timed-out-waiting-for-packet-header)
RESTART DOCKER

http://paulmurraycbr.github.io/ArduinoTheOOWay.html
