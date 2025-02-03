/* ___       _        ____       _       _      _    ____ ___
  / _ \  ___| |_ ___ |  _ \ _ __(_)_ __ | |_   / \  |  _ \_ _|
 | | | |/ __| __/ _ \| |_) | '__| | '_ \| __| / _ \ | |_) | |
 | |_| | (__| || (_) |  __/| |  | | | | | |_ / ___ \|  __/| |
  \___/ \___|\__\___/|_|   |_|  |_|_| |_|\__/_/   \_\_|  |___|
.......By Stephen Ludgate https://www.chunkymedia.co.uk.......

  extended by Pit Hermann https://github.com/PitHerm
  by adding the values of the DisplayLayerProgress plugin,
  You need to have the plugin installed.
  call: <your_api_name>.getdlp()
  retrieved values:
  String currentFilename;
  String fanSpeed;
  int feedrate;
  int feedrateG0;
  int feedrateG1;
  double heightCurrent;
  double heightCurrentFormatted;
  double heightTotal;
  double heightTotalFormatted;
  String averageLayerDuration;
  long int averageLayerDurationInSeconds;
  int CurrentLayer;
  String lastLayerDuration;
  long int lastLayerDurationInSeconds;
  int LayerTotal;
  int changeFilamentCount;
  String changeFilamentTimeLeft;
  long int changeFilamentTimeLeftInSeconds;
  String estimatedChangedFilamentTime;
  String estimatedEndTime;
  String m73progress;
  String printerState;
  int progress;
  String timeLeft;
  long int timeLeftInSeconds;
*/

#include "OctoPrintAPIExt.h"

#include "Arduino.h"

OctoPrintAPIExt::OctoPrintAPIExt(void){
	if (_debug)
		Serial.println("Be sure to Call init to setup and start the OctoprintApi instance");
	
}

/** OctoPrintAPIExt()
 * IP address version of the client connect function
 * */
OctoPrintAPIExt::OctoPrintAPIExt(Client &client, IPAddress octoPrintIp, int octoPrintPort, String apiKey) {
  init(client, octoPrintIp, octoPrintPort, apiKey);
}

void OctoPrintAPIExt::init(Client &client, IPAddress octoPrintIp, int octoPrintPort, String apiKey) {
  _client         = &client;
  _apiKey         = apiKey;
  _octoPrintIp    = octoPrintIp;
  _octoPrintPort  = octoPrintPort;
  _usingIpAddress = true;
} 

/** OctoPrintAPIExt()
 * Hostname version of the client connect function
 * */
 
OctoPrintAPIExt::OctoPrintAPIExt(Client &client, char *octoPrintUrl, int octoPrintPort, String apiKey) {
	init(client, octoPrintUrl,octoPrintPort,apiKey);
}

void OctoPrintAPIExt::init(Client &client, char *octoPrintUrl, int octoPrintPort, String apiKey) {
  _client         = &client;
  _apiKey         = apiKey;
  _octoPrintUrl   = octoPrintUrl;
  _octoPrintPort  = octoPrintPort;
  _usingIpAddress = false;
}

/** GET YOUR ASS TO OCTOPRINT...
 *
 * **/
String OctoPrintAPIExt::sendRequestToOctoprint(String type, String command, const char *data) {
  if (_debug)
    Serial.println("OctoPrintAPIExt::sendRequestToOctoprint() CALLED");

  if ((type != "GET") && (type != "POST")) {
    if (_debug)
      Serial.println("OctoPrintAPIExt::sendRequestToOctoprint() Only GET & POST are supported... exiting.");
    return "";
  }

  String statusCode       = "";
  String headers          = "";
  String body             = "";
  bool finishedStatusCode = false;
  bool finishedHeaders    = false;
  bool currentLineIsBlank = true;
  int ch_count            = 0;
  int headerCount         = 0;
  int headerLineStart     = 0;
  int bodySize            = -1;
  unsigned long now;

  bool connected;

  if (_usingIpAddress)
    connected = _client->connect(_octoPrintIp, _octoPrintPort);
  else
    connected = _client->connect(_octoPrintUrl, _octoPrintPort);

  if (connected) {
    if (_debug)
      Serial.println(".... connected to server");

    char useragent[64];
    snprintf(useragent, 64, "User-Agent: %s", USER_AGENT);

    _client->println(type + " " + command + " HTTP/1.1");
    _client->print("Host: ");
    if (_usingIpAddress)
      _client->println(_octoPrintIp);
    else
      _client->println(_octoPrintUrl);
    _client->print("X-Api-Key: ");
    _client->println(_apiKey);
    _client->println(useragent);
    _client->println("Connection: keep-alive");
    if (data != NULL) {
      _client->println("Content-Type: application/json");
      _client->print("Content-Length: ");
      _client->println(strlen(data));  // number of bytes in the payload
      _client->println();              // important need an empty line here
      _client->println(data);          // the payload
    } else
      _client->println();

    now = millis();
    while (millis() - now < OPAPI_TIMEOUT) {
      while (_client->available()) {
        char c = _client->read();

        if (_debug)
          Serial.print(c);

        if (!finishedStatusCode) {
          if (c == '\n')
            finishedStatusCode = true;
          else
            statusCode = statusCode + c;
        }

        if (!finishedHeaders) {
          if (c == '\n') {
            if (currentLineIsBlank)
              finishedHeaders = true;
            else {
              if (headers.substring(headerLineStart).startsWith("Content-Length: "))
                bodySize = (headers.substring(headerLineStart + 16)).toInt();
              headers = headers + c;
              headerCount++;
              headerLineStart = headerCount;
            }
          } else {
            headers = headers + c;
            headerCount++;
          }
        } else {
          if (ch_count < maxMessageLength) {
            body = body + c;
            ch_count++;
            if (ch_count == bodySize)
              break;
          }
        }
        if (c == '\n')
          currentLineIsBlank = true;
        else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
      if (ch_count == bodySize)
        break;
    }
  } else {
    if (_debug) {
      Serial.println("connection failed");
      Serial.println(connected);
    }
  }

  closeClient();

  int httpCode = extractHttpCode(statusCode, body);
  if (_debug) {
    Serial.print("\nhttpCode:");
    Serial.println(httpCode);
  }
  httpStatusCode = httpCode;
  if(httpStatusCode <= 199 || httpStatusCode >= 300){httpErrorBody = body;} //account for any error codes. Client can decide what to do with Body Contents as a fall back.

  return body;
}

String OctoPrintAPIExt::sendGetToOctoprint(String command) {
  if (_debug)
    Serial.println("OctoPrintAPIExt::sendGetToOctoprint() CALLED");

  return sendRequestToOctoprint("GET", command, NULL);
}

/** getOctoprintVersion()
 * http://docs.octoprint.org/en/master/api/version.html#version-information
 * Retrieve information regarding server and API version. Returns a JSON object with two keys, api containing the API version, server containing the server version.
 * Status Codes:
 * 200 OK – No error
 * */
bool OctoPrintAPIExt::getOctoprintVersion() {
  String response = sendGetToOctoprint("/api/version");
  StaticJsonDocument<JSONDOCUMENT_SIZE> root;

  if (!deserializeJson(root, response) && root.containsKey("api")) {
    octoprintVer.OctoPrintAPIExt    = (const char *)root["api"];
    octoprintVer.octoprintServer = (const char *)root["server"];
    return true;
  }
  return false;
}

/** getPrinterStatistics()
 * http://docs.octoprint.org/en/master/api/printer.html#retrieve-the-current-printer-state
 * Retrieves the current state of the printer.
 * Returns a 200 OK with a Full State Response in the body upon success.
 * */
bool OctoPrintAPIExt::getPrinterStatistics() {
  String response = sendGetToOctoprint("/api/printer");  //recieve reply from OctoPrint
  printerStats.printerState = ""; // inserted by PitHe
  StaticJsonDocument<JSONDOCUMENT_SIZE> root;
  if (!deserializeJson(root, response)) {
    if (root.containsKey("state")) {
      printerStats.printerState              = (const char *)root["state"]["text"];
      printerStats.printerStateclosedOrError = root["state"]["flags"]["closedOrError"];
      printerStats.printerStateerror         = root["state"]["flags"]["error"];
      printerStats.printerStatefinishing     = root["state"]["flags"]["finishing"];
      printerStats.printerStateoperational   = root["state"]["flags"]["operational"];
      printerStats.printerStatepaused        = root["state"]["flags"]["paused"];
      printerStats.printerStatepausing       = root["state"]["flags"]["pausing"];
      printerStats.printerStatePrinting      = root["state"]["flags"]["printing"];
      printerStats.printerStateready         = root["state"]["flags"]["ready"];
      printerStats.printerStateresuming      = root["state"]["flags"]["resuming"];
      printerStats.printerStatesdReady       = root["state"]["flags"]["sdReady"];
    }

    printerStats.printerBedAvailable   = false;
    printerStats.printerTool0Available = false;
    printerStats.printerTool1Available = false;
    if (root.containsKey("temperature")) {
      if (root["temperature"].containsKey("bed")) {
        printerStats.printerBedTempActual = root["temperature"]["bed"]["actual"];
        printerStats.printerBedTempTarget = root["temperature"]["bed"]["target"];
        printerStats.printerBedTempOffset = root["temperature"]["bed"]["offset"];
        printerStats.printerBedAvailable  = true;
      }

      if (root["temperature"].containsKey("tool0")) {
        printerStats.printerTool0TempActual = root["temperature"]["tool0"]["actual"];
        printerStats.printerTool0TempTarget = root["temperature"]["tool0"]["target"];
        printerStats.printerTool0TempOffset = root["temperature"]["tool0"]["offset"];
        printerStats.printerTool0Available  = true;
      }

      if (root["temperature"].containsKey("tool1")) {
        printerStats.printerTool1TempActual = root["temperature"]["tool1"]["actual"];
        printerStats.printerTool1TempTarget = root["temperature"]["tool1"]["target"];
        printerStats.printerTool1TempOffset = root["temperature"]["tool1"]["offset"];
        printerStats.printerTool1Available  = true;
      }
    }
    return true;
  } else {
    printerStats.printerStateoperational = false;
    if (response == "Printer is not operational") {
      printerStats.printerState = response;
      return true;
    }
  }
  return false;
}

/***** PRINT JOB OPPERATIONS *****/
/**
 * http://docs.octoprint.org/en/devel/api/job.html#issue-a-job-command
 * Job commands allow starting, pausing and cancelling print jobs.
 * Available commands are: start, cancel, restart, pause - Accepts one optional additional parameter action specifying
 * which action to take - pause, resume, toggle
 * If no print job is active (either paused or printing), a 409 Conflict will be returned.
 * Upon success, a status code of 204 No Content and an empty body is returned.
 * */
bool OctoPrintAPIExt::octoPrintJobStart() {
  sendPostToOctoPrint("/api/job", "{\"command\": \"start\"}");
  return (httpStatusCode == 204);
}

bool OctoPrintAPIExt::octoPrintJobCancel() {
  sendPostToOctoPrint("/api/job", "{\"command\": \"cancel\"}");
  return (httpStatusCode == 204);
}

bool OctoPrintAPIExt::octoPrintJobRestart() {
  sendPostToOctoPrint("/api/job", "{\"command\": \"restart\"}");
  return (httpStatusCode == 204);
}

bool OctoPrintAPIExt::octoPrintJobPauseResume() {
  sendPostToOctoPrint("/api/job", "{\"command\": \"pause\"}");
  return (httpStatusCode == 204);
}

bool OctoPrintAPIExt::octoPrintJobPause() {
  sendPostToOctoPrint("/api/job", "{\"command\": \"pause\", \"action\": \"pause\"}");
  return (httpStatusCode == 204);
}

bool OctoPrintAPIExt::octoPrintJobResume() {
  sendPostToOctoPrint("/api/job", "{\"command\": \"pause\", \"action\": \"resume\"}");
  return (httpStatusCode == 204);
}

bool OctoPrintAPIExt::octoPrintFileSelect(String &path) {
  sendPostToOctoPrint("/api/files/local" + path, "{\"command\": \"select\", \"print\": false }");
  return (httpStatusCode == 204);
}

//bool OctoPrintAPIExt::octoPrintJobPause(String actionCommand){}

/***** SYSTEM COMMANDS *****/
/**
 * https://docs.octoprint.org/en/master/api/system.html
 * All system operations require the SYSTEM permission.
 * action: shutdown, reboot, restart 
 * */
bool OctoPrintAPIExt::octoPrintCoreShutdown() {
  sendPostToOctoPrint("/api/system/commands/core/shutdown","");
  return (httpStatusCode == 204);
}

bool OctoPrintAPIExt::octoPrintCoreReboot() {
  sendPostToOctoPrint("/api/system/commands/core/reboot","");
  return (httpStatusCode == 204);
}

bool OctoPrintAPIExt::octoPrintCoreRestart() {
  sendPostToOctoPrint("/api/system/commands/core/restart","");
  return (httpStatusCode == 204);
}

/* added by Pit Hermann begin*/
bool OctoPrintAPIExt::getDlp() {
  String response = sendGetToOctoprint("/plugin/DisplayLayerProgress/values");
  StaticJsonDocument<JSONDOCUMENT_SIZE> root;
  if (!deserializeJson(root, response)) {
	dlp.currentFilename = (const char *)root["currentFilename"];
	dlp.fanSpeed = (const char *)root["fanSpeed"];
	dlp.feedrate = root["feedrate"];
	dlp.feedrateG0 = root["feedrateG0"];
	dlp.feedrateG1 = root["feedrateG1"];
	dlp.heightCurrent = root["height"]["current"];
	dlp.heightCurrentFormatted = root["height"]["currentFormatted"];
	dlp.heightTotal = root["height"]["total"];
	dlp.heightTotalFormatted = root["height"]["totalFormatted"];
	dlp.averageLayerDuration = (const char *)root["layer"]["averageLayerDuration"];
	dlp.averageLayerDurationInSeconds = root["layer"]["averageLayerDurationInSeconds"];
	dlp.CurrentLayer = root["layer"]["current"];
	dlp.lastLayerDuration = (const char *)root["layer"]["lastLayerDuration"];
	dlp.lastLayerDurationInSeconds = root["layer"]["lastLayerDurationInSeconds"];
	dlp.LayerTotal = root["layer"]["total"];
	dlp.changeFilamentCount = root["print"]["changeFilamentCount"];
	dlp.changeFilamentTimeLeft = (const char *)root["print"]["changeFilamentTimeLeft"];
	dlp.changeFilamentTimeLeftInSeconds = root["print"]["changeFilamentTimeLeftInSeconds"];
	dlp.estimatedChangedFilamentTime = (const char *)root["print"]["estimatedChangedFilamentTime"];
	dlp.estimatedEndTime = (const char *)root["print"]["estimatedEndTime"];
	dlp.m73progress = (const char *)root["print"]["m73progress"];
	dlp.printerState = (const char *)root["print"]["printerState"];
	dlp.progress = root["print"]["progress"];
	dlp.timeLeft = (const char *)root["print"]["timeLeft"];
	dlp.timeLeftInSeconds = root["print"]["timeLeftInSeconds"];
	return true;
  }
  return false;
}
/* added by Pit Hermann end */


/** getPrintJob
 * http://docs.octoprint.org/en/master/api/job.html#retrieve-information-about-the-current-job
 * Retrieve information about the current job (if there is one).
 * Returns a 200 OK with a Job information response in the body.
 * */
bool OctoPrintAPIExt::getPrintJob() {
  String response = sendGetToOctoprint("/api/job");

  StaticJsonDocument<JSONDOCUMENT_SIZE> root;
  if (!deserializeJson(root, response)) {
    printJob.printerState = (const char *)root["state"];

    if (root.containsKey("job")) {
      printJob.estimatedPrintTime = root["job"]["estimatedPrintTime"];

      printJob.jobFileDate   = root["job"]["file"]["date"];
      printJob.jobFileName   = (const char *)(root["job"]["file"]["name"] | "");
      printJob.jobFileOrigin = (const char *)(root["job"]["file"]["origin"] | "");
      printJob.jobFileSize   = root["job"]["file"]["size"];
	  printJob.jobFilePath = (const char *)(root["job"]["file"]["path"] | "");

      printJob.jobFilamentTool0Length = root["job"]["filament"]["tool0"]["length"] | 0.0;
      printJob.jobFilamentTool0Volume = root["job"]["filament"]["tool0"]["volume"] | 0.0;
      printJob.jobFilamentTool1Length = root["job"]["filament"]["tool1"]["length"] | 0.0;
      printJob.jobFilamentTool1Volume = root["job"]["filament"]["tool1"]["volume"] | 0.0;
    }
    if (root.containsKey("progress")) {
      printJob.progressCompletion          = root["progress"]["completion"] | 0.0;
      printJob.progressFilepos             = root["progress"]["filepos"];
      printJob.progressPrintTime           = root["progress"]["printTime"];
      printJob.progressPrintTimeLeft       = root["progress"]["printTimeLeft"];
      printJob.progressprintTimeLeftOrigin = (const char *)root["progress"]["printTimeLeftOrigin"];
    }
    return true;
  }
  return false;
}

/** getOctoprintEndpointResults()
 * General function to get any GET endpoint of the API and return body as a string for you to format or view as you wish.
 * */
String OctoPrintAPIExt::getOctoprintEndpointResults(String command) {
  if (_debug)
    Serial.println("OctoPrintAPIExt::getOctoprintEndpointResults() CALLED");
  return sendGetToOctoprint("/api/" + command);
}

/** POST TIME
 *
 * **/
String OctoPrintAPIExt::sendPostToOctoPrint(String command, const char *postData) {
  if (_debug)
    Serial.println("OctoPrintAPIExt::sendPostToOctoPrint() CALLED");
  return sendRequestToOctoprint("POST", command, postData);
}

/***** CONNECTION HANDLING *****/
/**
 * http://docs.octoprint.org/en/master/api/connection.html#issue-a-connection-command
 * Issue a connection command. Currently available command are: connect, disconnect, fake_ack
 * Status Codes:
 * 204 No Content – No error
 * 400 Bad Request – If the selected port or baudrate for a connect command are not part of the available options.
 * */
bool OctoPrintAPIExt::octoPrintConnectionAutoConnect() {
  sendPostToOctoPrint("/api/connection", "{\"command\": \"connect\"}");
  return (httpStatusCode == 204);
}
bool OctoPrintAPIExt::octoPrintConnectionDisconnect() {
  sendPostToOctoPrint("/api/connection", "{\"command\": \"disconnect\"}");
  return (httpStatusCode == 204);
}
bool OctoPrintAPIExt::octoPrintConnectionFakeAck() {
  sendPostToOctoPrint("/api/connection", "{\"command\": \"fake_ack\"}");
  return (httpStatusCode == 204);
}

/***** PRINT HEAD *****/
/**
 * http://docs.octoprint.org/en/master/api/printer.html#issue-a-print-head-command
 * Print head commands allow jogging and homing the print head in all three axes.
 * Available commands are: jog, home, feedrate
 * All of these commands except feedrate may only be sent if the printer is currently operational and not printing. Otherwise a 409 Conflict is returned.
 * Upon success, a status code of 204 No Content and an empty body is returned.
 * */
bool OctoPrintAPIExt::octoPrintPrintHeadHome() {
  //   {
  //   "command": "home",
  //   "axes": ["x", "y", "z"]
  // }
  sendPostToOctoPrint("/api/printer/printhead", "{\"command\": \"home\",\"axes\": [\"x\", \"y\"]}");
  return (httpStatusCode == 204);
}

bool OctoPrintAPIExt::octoPrintPrintHeadAbsoluteJog(double x, double y, double z, double f) {
	return octoPrintPrintHeadJog(x,y,z,f,true);
}
bool OctoPrintAPIExt::octoPrintPrintHeadRelativeJog(double x, double y, double z, double f) {
	return octoPrintPrintHeadJog(x,y,z,f,false);
}
	
bool OctoPrintAPIExt::octoPrintPrintHeadJog(double x, double y, double z, double f,bool absolute) {
  //  {
  // "command": "jog",
  // "x": 10,
  // "y": -5,
  // "z": 0.02,
  // "absolute": false,
  // "speed": 30
  // }
  char postData[1024];
  char tmp[128];
  postData[0] = '\0';

  strcat(postData, "{\"command\": \"jog\"");
  if (x != 0) {
    snprintf(tmp, 128, ", \"x\": %f", x);
    strcat(postData, tmp);
  }
  if (y != 0) {
    snprintf(tmp, 128, ", \"y\": %f", y);
    strcat(postData, tmp);
  }
  if (z != 0) {
    snprintf(tmp, 128, ", \"z\": %f", z);
    strcat(postData, tmp);
  }
  if (f != 0) {
    snprintf(tmp, 128, ", \"speed\": %f", f);
    strcat(postData, tmp);
  }
  if(absolute){strcat(postData, ", \"absolute\": true");}
  else{strcat(postData, ", \"absolute\": false");}
	
  strcat(postData, " }");
  if (_debug)
    Serial.println(postData);

  sendPostToOctoPrint("/api/printer/printhead", postData);
  return (httpStatusCode == 204);
}

bool OctoPrintAPIExt::octoPrintExtrude(double amount) {
  char postData[POSTDATA_SIZE];
  snprintf(postData, POSTDATA_SIZE, "{ \"command\": \"extrude\", \"amount\": %f }", amount);

  sendPostToOctoPrint("/api/printer/tool", postData);
  return (httpStatusCode == 204);
}

bool OctoPrintAPIExt::octoPrintSetBedTemperature(uint16_t t) {
  char postData[POSTDATA_SIZE];
  snprintf(postData, POSTDATA_SIZE, "{ \"command\": \"target\", \"target\": %d }", t);

  sendPostToOctoPrint("/api/printer/bed", postData);
  return (httpStatusCode == 204);
}

bool OctoPrintAPIExt::octoPrintSetTool0Temperature(uint16_t t) {
  char postData[POSTDATA_SIZE];
  snprintf(postData, POSTDATA_SIZE, "{ \"command\": \"target\", \"targets\": { \"tool0\": %d } }", t);

  sendPostToOctoPrint("/api/printer/tool", postData);
  return (httpStatusCode == 204);
}

bool OctoPrintAPIExt::octoPrintSetTool1Temperature(uint16_t t) {
  char postData[POSTDATA_SIZE];
  snprintf(postData, POSTDATA_SIZE, "{ \"command\": \"target\", \"targets\": { \"tool1\": %d } }", t);

  sendPostToOctoPrint("/api/printer/tool", postData);
  return (httpStatusCode == 204);
}

/***** PRINT BED *****/
/** octoPrintGetPrinterBed()
 * http://docs.octoprint.org/en/master/api/printer.html#retrieve-the-current-bed-state
 * Retrieves the current temperature data (actual, target and offset) plus optionally a (limited) history (actual, target, timestamp) for the printer’s heated bed.
 * It’s also possible to retrieve the temperature history by supplying the history query parameter set to true.
 * The amount of returned history data points can be limited using the limit query parameter.
 * Returns a 200 OK with a Temperature Response in the body upon success.
 * If no heated bed is configured for the currently selected printer profile, the resource will return an 409 Conflict.
 * */
bool OctoPrintAPIExt::octoPrintGetPrinterBed() {
  String response = sendGetToOctoprint("/api/printer/bed?history=true&limit=2");

  StaticJsonDocument<JSONDOCUMENT_SIZE> root;
  if (!deserializeJson(root, response)) {
    if (root.containsKey("bed")) {
      printerBed.printerBedTempActual = root["bed"]["actual"];
      printerBed.printerBedTempOffset = root["bed"]["offset"];
      printerBed.printerBedTempTarget = root["bed"]["target"];
    }
    if (root.containsKey("history")) {
      printerBed.printerBedTempHistoryTimestamp = root["history"][0]["time"];
      printerBed.printerBedTempHistoryActual    = root["history"][0]["bed"]["actual"];
    }
    return true;
  }
  return false;
}

/***** SD FUNCTIONS *****/
/*
 * http://docs.octoprint.org/en/master/api/printer.html#issue-an-sd-command
 * SD commands allow initialization, refresh and release of the printer’s SD card (if available).
 * Available commands are: init, refresh, release
*/
bool OctoPrintAPIExt::octoPrintPrinterSDInit() {
  sendPostToOctoPrint("/api/printer/sd", "{\"command\": \"init\"}");
  return (httpStatusCode == 204);
}
bool OctoPrintAPIExt::octoPrintPrinterSDRefresh() {
  sendPostToOctoPrint("/api/printer/sd", "{\"command\": \"refresh\"}");
  return (httpStatusCode == 204);
}
bool OctoPrintAPIExt::octoPrintPrinterSDRelease() {
  sendPostToOctoPrint("/api/printer/sd", "{\"command\": \"release\"}");
  return (httpStatusCode == 204);
}

/*
http://docs.octoprint.org/en/master/api/printer.html#retrieve-the-current-sd-state
Retrieves the current state of the printer’s SD card.
If SD support has been disabled in OctoPrint’s settings, a 404 Not Found is returned.
Returns a 200 OK with an SD State Response in the body upon success.
*/
bool OctoPrintAPIExt::octoPrintGetPrinterSD() {
  String command  = "/api/printer/sd";
  String response = sendGetToOctoprint(command);

  StaticJsonDocument<JSONDOCUMENT_SIZE> root;
  if (!deserializeJson(root, response)) {
    bool printerStatesdReady         = root["ready"];
    printerStats.printerStatesdReady = printerStatesdReady;
    return true;
  }
  return false;
}

/***** COMMANDS *****/
/*
http://docs.octoprint.org/en/master/api/printer.html#send-an-arbitrary-command-to-the-printer
Sends any command to the printer via the serial interface. Should be used with some care as some commands can interfere with or even stop a running print job.
If successful returns a 204 No Content and an empty body.
*/
bool OctoPrintAPIExt::octoPrintPrinterCommand(char *gcodeCommand) {
  String command = "/api/printer/command";
  char postData[POSTDATA_GCODE_SIZE];

  postData[0] = '\0';
  snprintf(postData, POSTDATA_GCODE_SIZE, "{\"command\": \"%s\"}", gcodeCommand);

  sendPostToOctoPrint(command, postData);

  return (httpStatusCode == 204);
}

/***** GENERAL FUNCTIONS *****/

/**
 * Close the client
 * */
void OctoPrintAPIExt::closeClient() { _client->stop(); }

/**
 * Extract the HTTP header response code. Used for error reporting - will print in serial monitor any non 200 response codes (i.e. if something has gone wrong!).
 * Thanks Brian for the start of this function, and the chuckle of watching you realise on a live stream that I didn't use the response code at that time! :)
 * */
int OctoPrintAPIExt::extractHttpCode(String statusCode, String body) {
  if (_debug) {
    Serial.print("\nStatus code to extract: ");
    Serial.println(statusCode);
  }
  int firstSpace = statusCode.indexOf(" ");
  int lastSpace  = statusCode.lastIndexOf(" ");
  if (firstSpace > -1 && lastSpace > -1 && firstSpace != lastSpace) {
    String statusCodeALL     = statusCode.substring(firstSpace + 1);             //"400 BAD REQUEST"
    String statusCodeExtract = statusCode.substring(firstSpace + 1, lastSpace);  //May end up being e.g. "400 BAD"
    int statusCodeInt        = statusCodeExtract.toInt();                        //Converts to "400" integer - i.e. strips out rest of text characters "fix"
    if (_debug and statusCodeInt != 200 and statusCodeInt != 201 and statusCodeInt != 202 and statusCodeInt != 204) {
      Serial.print("\nSERVER RESPONSE CODE: " + String(statusCodeALL));
      if (body != "")
        Serial.println(" - " + body);
      else
        Serial.println();
    }
    return statusCodeInt;
  } else
    return -1;
}
