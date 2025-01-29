# Mainsale API

Library for use with Arduino compatible micro controllers (web enabled) to access the Mainsale API 



### What does it do?
You can connect to the Mainsale server via it's aAPI and pull back information about the state of the 3D printer

Crucially I use this to keep track of current print job completion percentage. 

This library will make it easy for you to get the information, I leave it up to you what and how you process this data in the most useful or pointless way!


## Getting Started

You will of course need a 3D printer which is running on M ainsale, and an Arduino compatible microcontroller that can connect to the internet/your network - basically an ESP8266!

## Installation

available via GitHub if you clone it or download as a ZIP. The downloaded ZIP code can be included as a new library into the IDE selecting the menu:

     Sketch / include Library / Add .Zip library

You will also have to install the  ArduinoJson library. 

Include Mainsale API in your project:

    #include <MainsaleAPI.h>


## Examples

### HelloWorldSerial
This is the first sketch to try out. Enter your WiFi credentials, your OctoPrint network info, API key, compile and upload. Open the serial monitor and you should start to see printer information coming back. Works on both ESP8266 and ESP32 boards.

### GetPrintJObInfo
Uses the getPrinterStatus() function of the class to get the current print job and returns most of the useful API variables. Gives a "real world" example of using the variables to print more human readable info once collected from the API.


## License

See the [LICENSE.md](LICENSE.md) file for details


## Release History
* 1.0



