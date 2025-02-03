# OctoPrint API Extended
<img align="left" style="padding-right:10px;" src="https://octoprint.org/assets/img/logo.png">
Library for use with Arduino compatible micro controllers (web enabled) to access the Octoprint API Extended on Raspberry Pi's (or any Linux based box) running the [OctoPrint](https://octoprint.org/) 3D printer web server by the brilliant Gina Häußge, aka @foosel.

### What's the difference to Octoprint Api ?
This is simply the base Octoprint Api from chunkysteveo (https://github.com/chunkysteveo/OctoPrintAPI) with additional access to the values the DisplayLayerProgress Plugin provides:

  String currentFilename
  String fanSpeed
  int feedrate
  int feedrateG0
  int feedrateG1
  double heightCurrent
  double heightCurrentFormatted
  double heightTotal
  double heightTotalFormatted;
  String averageLayerDuration;
  long int averageLayerDurationInSeconds
  int CurrentLayer
  String lastLayerDuration
  long int lastLayerDurationInSeconds
  int LayerTotal
  int changeFilamentCount
  String changeFilamentTimeLeft
  long int changeFilamentTimeLeftInSeconds
  String estimatedChangedFilamentTime
  String estimatedEndTime
  String m73progress
  String printerState
  int progress
  String timeLeft
  long int timeLeftInSeconds


You have to install DisplayLayerProgress Plugin to Octoprint.
call: <your_api_name>.getdlp()


### What does it do?
You can connect to the OctoPrint server via it's awesome REST API and pull back information about the state of the 3D printer, the temperatures of it's print bed, hotend(s), and any current print job progress. In fact, you can do most things via the REST API with some GET and POST commands.

Crucially I use this to keep track of current print job completion percentage. Using something simple like a Neopixel stick - you can have a portable progress bar sat next to you, while your 3D printer is printing away in your garage, basement, shed etc. Saves you time opening up a browser in your laptop or phone, and logging in and viewing the current status - just glace up at some LEDs and see how far it's gone.

This library will make it easy for you to get the information, I leave it up to you what and how you process this data in the most useful or pointless way!

[![Buy me a beer!](https://img.shields.io/badge/Buy_me_Beer!-PayPal-green.svg)](https://www.paypal.me/chunkysteveo/3.5)

## Getting Started

You will of course need a 3D printer which is running on OctoPrint, and an Arduino compatible microcontroller that can connect to the internet/your network - basically an ESP8266! Your OctoPi installation will need a valid API key, and you will also need to enable CORS.

### Create an API Key
In your OctoPrint server settings, go to Api and enable it, and copy the API Key.
![](http://docs.octoprint.org/en/master/_images/settings-global-api-key.png)

### Enable CORS (cross-origin resource sharing)
You need to enable this to access the API.
![](http://docs.octoprint.org/en/master/_images/settings-api-cors.png)

### Test your connection
You can check if your server is ready to accept request to the REST API by going to it in a browser on a PC/laptop and adding in the API key as a parameter. The URL would look something like this:

```
http://192.168.1.2/api/version?apikey=ABC123DEF456ETCYOUGETHEIDEA
```

*where 192.168.1.2 is the local IP of your OctoPrint server, and ABC123DEF456ETCYOUGETHEIDEA is your API key.*

Your browser should give you something like this:

```
{
  "api": "0.1",
  "server": "1.3.6"
}
```

Hooray, you can now talk to your OctoPrint server via it's API. And so can your Arduino.

### Connecting if External, e.g. Over the Internet
It works if you use your hostname and custom port that forwards... more instructions to follow :)

## Installation
Available in the official Arduino Library Manager - Sketch -> Include Library -> Manage Libraries... and then search for "octoprint" and click Install!

Also available via GitHub if you clone it or download as a ZIP. The downloaded ZIP code can be included as a new library into the IDE selecting the menu:

     Sketch / include Library / Add .Zip library

You will also have to install the awesome ArduinoJson library written by [Benoît Blanchon](https://github.com/bblanchon). Search **ArduinoJson** on the Arduino Library manager or get it from [here](https://github.com/bblanchon/ArduinoJson). If you've not got it already, what have you been doing?!

Include OctoPrint API in your project:

    #include <OctoPrintAPI.h>


## Examples

### HelloWorldSerial
This is the first sketch to try out. Enter your WiFi credentials, your OctoPrint network info, API key, compile and upload. Open the serial monitor and you should start to see printer information coming back. Works on both ESP8266 and ESP32 boards.

### GetPrintJObInfo
Uses the getPrintJob() function of the class to get the current print job and returns most of the useful API variables. Gives a "real world" example of using the variables to print more human readable info once collected from the API.


## Acknowledgments

* Hat tip to Brian Lough, aka @witnessmenow for is work on his Arduino API libraries which gave me the base to create my own. His [YouTube API](https://github.com/witnessmenow/arduino-youtube-api) is used in our office every day to keep score!
* Gina Häußge (@foosel) for her amazing work on OctoPrint, which without her, none of this would be even possible.
* Miles Nash for the first to use my library in a live Arduino project - [3D Printer Monitoring with Alexa and Arduino](https://www.hackster.io/Milesnash_/3d-printer-monitoring-with-alexa-and-arduino-024292)
* [sidddy](https://github.com/sidddy) for some big improvements to the core request function and various updates which contributed to the 1.1.2 release.
* [anfichtn](https://github.com/anfichtn) for the first contributed example (ESP8266/WS2812BProgress) and some endpoint function calls.

## Authors
* **Pit Hermann** - *This version* basing von the below - [PitHerm](https://github.com/PitHerm)
* **Stephen Ludgate** - *Initial work* - [chunkysteveo](https://github.com/chunkysteveo)
* **sidddy** - *Contributor* - [sidddy](https://github.com/sidddy)
* **anfichtn** - *Contributor* - [anfichtn](https://github.com/anfichtn)
* **Lars Simonsen** - *Contributor* - [LarsSimonsen](https://github.com/LarsSimonsen)
* **JCsGotThis** - *Contributor* - [jcassel](https://github.com/jcassel)

## License

See the [LICENSE.md](LICENSE.md) file for details


## Release History
* 1.0 basing on Octopi Api 1.1.6 



