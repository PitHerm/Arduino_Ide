/*******************************************************************
 *  A "Hello World" sketch to make sure your Arduino (ESP8266) and your
 *  OctoPrint server can speak to each other. This will show your
 *  OctoPrint Server, API and 3D printer statistics from the
 *  OctoPrint API.
 *
 *  You will need the IP or hostname of your OctoPrint server, a
 *  port number (will be 80 unless you are reaching it from an
 *  external source) and an API key from the OctoPrint
 *  installation - http://docs.octoprint.org/en/master/api/general.html#authorization                              
 *  You will also need to enable CORS - http://docs.octoprint.org/en/master/api/general.html#cross-origin-requests
 * 
 *  By Stephen Ludgate https://www.youtube.com/channel/UCVEEuAouZ6ua4oetLjjHAuw
 * 
 *  extended by Pit Hermann https://github.com/PitHerm
 *  by adding the values of the DisplayLayerProgress plugin,
 *  You need to have the plugin installed.
 *******************************************************************/


#include <OctoPrintAPIExt.h> //This is where the magic happens... shazam!

#include <ESP8266WiFi.h>
#include <WiFiClient.h>

const char* ssid = "SSID";          // your network SSID (name)
const char* password = "PASSWORD";  // your network password

WiFiClient client; 


// You only need to set one of the of follwowing:
IPAddress ip(192, 168, 123, 123);                         // Your IP address of your OctoPrint server (inernal or external)
// char* octoprint_host = "octoprint.example.com";  // Or your hostname. Comment out one or the other.

const int octoprint_httpPort = 80;  //If you are connecting through a router this will work, but you need a random port forwarded to the OctoPrint server from your router. Enter that port here if you are external
String octoprint_apikey = "API_KEY"; //See top of file or GIT Readme about getting API key

String printerOperational;
String printerPaused;
String printerPrinting;
String printerReady;
String printerText;
String printerHotend;
String printerTarget;
String payload;

// Use one of the following:
OctoPrintAPIExt api; //Be sure to call init in setup. 
//OctoPrintAPIExt api(client, ip, octoprint_httpPort, octoprint_apikey);               //If using IP address
//OctoPrintAPIExt api(client, octoprint_host, octoprint_httpPort, octoprint_apikey);//If using hostname. Comment out one or the other.

unsigned long api_mtbs = 60000; //mean time between api requests (60 seconds)
unsigned long api_lasttime = 0;   //last time api request has been done

void setup () {
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //if you get here you have connected to the WiFi
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  //Use one of these Only if you have not initialized the api object above with parameters"
  api.init(client, ip, octoprint_httpPort, octoprint_apikey);               //If using IP address
  //api.init(client, octoprint_host, octoprint_httpPort, octoprint_apikey);//If using hostname. Comment out one or the other.
  
}


void loop() {

  if (millis() - api_lasttime > api_mtbs || api_lasttime==0) {  //Check if time has expired to go check OctoPrint
    if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
      if(api.getOctoprintVersion()){
        Serial.println("---------Version---------");
        Serial.print("Octoprint API: ");
        Serial.println(api.octoprintVer.OctoPrintAPIExt);
        Serial.print("Octoprint Server: ");
        Serial.println(api.octoprintVer.octoprintServer);
        Serial.println("------------------------");
      }
      Serial.println();
      if(api.getPrinterStatistics()){
        Serial.println("---------States---------");
        Serial.print("Printer Current State: ");
        Serial.println(api.printerStats.printerState);
        Serial.print("Printer State - closedOrError:  ");
        Serial.println(api.printerStats.printerStateclosedOrError);
        Serial.print("Printer State - error:  ");
        Serial.println(api.printerStats.printerStateerror);
        Serial.print("Printer State - operational:  ");
        Serial.println(api.printerStats.printerStateoperational);
        Serial.print("Printer State - paused:  ");
        Serial.println(api.printerStats.printerStatepaused);
        Serial.print("Printer State - printing:  ");
        Serial.println(api.printerStats.printerStatePrinting);
        Serial.print("Printer State - ready:  ");
        Serial.println(api.printerStats.printerStateready);
        Serial.print("Printer State - sdReady:  ");
        Serial.println(api.printerStats.printerStatesdReady);
        Serial.println("------------------------");
        Serial.println();
        Serial.println("------Temperatures-----");
        Serial.print("Printer Temp - Tool0 (°C):  ");
        Serial.println(api.printerStats.printerTool0TempActual);
        Serial.print("Printer State - Tool1 (°C):  ");
        Serial.println(api.printerStats.printerTool1TempActual);
        Serial.print("Printer State - Bed (°C):  ");
        Serial.println(api.printerStats.printerBedTempActual);
        Serial.println("------------------------");
      }
      if(api.getDlp()){
        Serial.println("------DisplayLayerProgress values_------");
        Serial.print("currentFilename:\t");
        Serial.println(api.dlp.currentFilename);
        Serial.print("fanSpeed(%):\t");
        Serial.println(api.dlp.fanSpeed);
        Serial.print("feedrate:\t");
        Serial.println(api.dlp.feedrate);
        Serial.print("feedrateG0:\t");
        Serial.println(api.dlp.feedrateG0);
        Serial.print("feedrateG1:\t");
        Serial.println(api.dlp.feedrateG1);
        Serial.print("heightCurrent:\t");
        Serial.println(api.dlp.heightCurrent);
        Serial.print("heightCurrentFormatted:\t");
        Serial.println(api.dlp.heightCurrentFormatted, 1);
        Serial.print("heightTotal:\t");
        Serial.println(api.dlp.heightTotal);
        Serial.print("heightTotalFormatted:\t");
        Serial.println(api.dlp.heightTotalFormatted, 1);
        Serial.print("averageLayerDuration:\t");
        Serial.println(api.dlp.averageLayerDuration);
        Serial.print("averageLayerDurationInSeconds:\t");
        Serial.println(api.dlp.averageLayerDurationInSeconds);
        Serial.print("CurrentLayer:\t");
        Serial.println(api.dlp.CurrentLayer);
        Serial.print("lastLayerDuration:\t");
        Serial.println(api.dlp.lastLayerDuration);
        Serial.print("lastLayerDurationInSeconds:\t");
        Serial.println(api.dlp.lastLayerDurationInSeconds);
        Serial.print("LayerTotal:\t");
        Serial.println(api.dlp.LayerTotal);
        Serial.print("changeFilamentCount:\t");
        Serial.println(api.dlp.changeFilamentCount);
        Serial.print("changeFilamentTimeLeft:\t");
        Serial.println(api.dlp.changeFilamentTimeLeft);
        Serial.print("changeFilamentTimeLeftInSeconds:\t");
        Serial.println(api.dlp.changeFilamentTimeLeftInSeconds);
        Serial.print("estimatedChangedFilamentTime:\t");
        Serial.println(api.dlp.estimatedChangedFilamentTime);
        Serial.print("estimatedEndTime:\t");
        Serial.println(api.dlp.estimatedEndTime);
        Serial.print("m73progress:\t");
        Serial.println(api.dlp.m73progress);
        Serial.print("printerState:\t");
        Serial.println(api.dlp.printerState);
        Serial.print("progress (%):\t");
        Serial.println(api.dlp.progress);
        Serial.print("timeLeft:\t");
        Serial.println(api.dlp.timeLeft);
        Serial.print("timeLeftInSeconds:\t");
        Serial.println(api.dlp.timeLeftInSeconds);
        Serial.println("----------------------------------------"); 
        Serial.println();
      }
    }
    api_lasttime = millis();  //Set api_lasttime to current milliseconds run
  }
}
