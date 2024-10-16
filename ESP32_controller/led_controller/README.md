Upload each ESP32 - but modify the led_controller.ino with an new IP address per device (use sequential static IP addresses, eg. 120-140 for the last octet).

Upload sketch. Some of the boards are using: AI Thinker ESP32-CAM (Install both Espressif ESP32 boards and Arduino ESP32 boards in the IDE)

Visit the IP address in the browser: http://192.168.134.LAST_OCTET

For each ESP32 Camera:
- Update the last octed
- Change Brigtness: 2, Exposure: 1200, Gain: 30
- Select the right core/board. Some ESP32 Cam devices are using the ESP32 Wrover Module

Troubleshooting:
- Make sure your System laptop is on the same local network as the ESP32 devices
- Check the ESP32 .ino notes on which library to use for NeoPixel, otherwise device crashes when using >75 pixes