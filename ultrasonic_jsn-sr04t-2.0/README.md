# Ultrasonic JSN-SR04T-2.0

## Monitor
In the jsn-sr04t-2.0_serial_monitor folder is a arduino-program, that uses the serial ports of a (for example) esp32, to get data from a JSN-SR04T-2.0 ultra sonic sensor (see datasheet folder). It can be used to show the distance on an OLED-display.
The monitor program uses "Mode 3" (serial port mode with trigger) from the data sheet document.

## Simulator
In the jsn-sr04t-2.0_serial_simulator folder is a arduino-program, that uses the serial ports of a (for example) esp32, to simulate a JSN-SR04T-2.0 ultra sonic sensor (see datasheet folder). It can be used to test software without having the real hardware connected.
The simulator program uses "Mode 3" (serial port mode with trigger) from the data sheet document.

## Simulator-Monitor breadboard setup
![Alt text](https://github.com/baumrasen/esp32_projects/blob/develop/ultrasonic_jsn-sr04t-2.0/jsn-sr04t-2.0_sim_mon_breadboard/rs232_rsn-sr04t_breadboard.png)
