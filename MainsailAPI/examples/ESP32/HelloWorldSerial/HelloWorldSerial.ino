/*******************************************************************
 *  A simple example to show which values are avaiable
 *******************************************************************/


#include <MainsailAPI.h> 

#include <WiFi.h>
#include <WiFiClient.h>

const char* ssid = "SSID";	// your network SSID (name)
const char* password = "PASSWORD";	// your network password

WiFiClient client;



IPAddress ip(192, 168, 123, 123);	// Your IP address of your Mainsale server (inernal or external)

const int Mainsail_httpPort = 80;  

bool isActive;
float progress;
String state;
String filename;   
float extruderTemp;
float extruderTarget;
float bedTemp;
float bedTarget;
String connectionState;
int printDuration;
String payload;
int total_layer;
int current_layer;

char Text_printDuration[100];

MainsailApi api(client, ip, Mainsail_httpPort);

unsigned long api_mtbs = 30000; //mean time between api requests (30 seconds)
unsigned long api_lasttime = 0;   //last time api request has been done

void setup () {
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  /* Explicitly set the ESP32 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  api.init(client, ip, Mainsail_httpPort);
}


void loop() {

  if (millis() - api_lasttime > api_mtbs || api_lasttime==0) {  //Check if time has expired to go check OctoPrint
    if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
      if(api.getPrinterStatus()){
        
        Serial.println("---------States---------");
        Serial.print("Printer Current State: ");
        Serial.println(api.P_Status.state);
        Serial.println();
        Serial.println("------Termperatures-----");
        Serial.print("Printer Temp - Tool0 actual (°C):  ");
        Serial.println(api.P_Status.extruderTemp);
        Serial.print("Printer Temp - Tool0 target (°C):  ");
        Serial.println(api.P_Status.extruderTarget);
        Serial.print("Printer Temp - Bed actual (°C):  ");
        Serial.println(api.P_Status.bedTemp);
        Serial.print("Printer Temp - Bed target (°C):  ");
        Serial.println(api.P_Status.bedTarget);
        Serial.print("currentFilename:\t");
        Serial.println(api.P_Status.filename);
        Serial.print("progress (%):\t");
        Serial.println(api.P_Status.progress);
        Serial.print("isActive:\t");
        Serial.println(api.P_Status.isActive);
        Serial.print("connectionState:\t");
        Serial.println(api.P_Status.connectionState);
        Serial.print("printDuration (s):\t");
        Serial.println(api.P_Status.printDuration);
        int _Days = 0,_Hours = 0,_Minutes = 0,_Seconds = 0,_Reminder = 0,_Reminder1 = 0;
		if (api.P_Status.printDuration > 0){
			_Days = api.P_Status.printDuration / 86400;
			_Reminder = api.P_Status.printDuration % 86400;
			if (_Reminder > 0){
				_Hours = _Reminder / 3600;
				_Reminder1 =  _Reminder % 3600;
				if(_Reminder1 > 0){
					_Minutes = _Reminder1 / 60;
					_Seconds =  _Reminder1 % 60;
				}
			}
		}
		sprintf(Text_printDuration,"%d Days :%d Hours :%d Minutes :%d Secoonds",_Days,_Hours,_Minutes,_Seconds);
        Serial.print("printDuration String:\t");
        Serial.println(Text_printDuration);
        Serial.print("total layer:\t");
        Serial.println(api.P_Status.total_layer);
        Serial.print("current layer:\t");
        Serial.println(api.P_Status.current_layer);
        Serial.println("------------------------");
      }
      
    }
    api_lasttime = millis();  //Set api_lasttime to current milliseconds run
  }
}
