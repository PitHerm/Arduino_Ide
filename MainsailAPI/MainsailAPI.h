/*
  Mainsail basic api by Pit Hermann https://github.com/PitHerm
*/

#ifndef MainsailApi_h
#define MainsailApi_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Client.h>

#define OPAPI_TIMEOUT       3000
#define POSTDATA_SIZE       256
#define POSTDATA_GCODE_SIZE 50
#define USER_AGENT          "MainsailAPI/1.0.0 (Arduino ide)"

struct PrinterStatus {
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
	int total_layer;
	int current_layer;
	int filament_used;
};

class MainsailApi {
 public:
  bool getPrinterStatus();
  PrinterStatus P_Status;
  
  MainsailApi(void);
  MainsailApi(Client &client, IPAddress MainsailIp, int MainsailPort);
  void init(Client &client, IPAddress MainsailIp, int MainsailPort);
  String sendGetToMainsail(String command);
  bool _debug          = false;
  int httpStatusCode   = 0;
  String httpErrorBody = "";
  
 private:
  Client *_client;
  IPAddress _MainsailIp;
  int _MainsailPort;
  const int maxMessageLength = 2000;
  void closeClient();
  int extractHttpCode(String statusCode, String body);
  String sendRequestToMainsail(String type, String command, const char *data);
};

#endif
