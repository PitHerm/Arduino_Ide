/*
  Mainsail basic api by Pit Hermann https://github.com/PitHerm
*/

#include "MainsailAPI.h"

#include "Arduino.h"

MainsailApi::MainsailApi(void){
	if (_debug)
		Serial.println("Be sure to Call init to setup and start the MainsailApi instance");
	
}

MainsailApi::MainsailApi(Client &client, IPAddress MainsailIp, int MainsailPort) {
  init(client, MainsailIp, MainsailPort);
}

void MainsailApi::init(Client &client, IPAddress MainsailIp, int MainsailPort) {
  _client         = &client;
  _MainsailIp    = MainsailIp;
  _MainsailPort  = MainsailPort;
  
} 


/** GET  Mainsail...
 *
 * **/
String MainsailApi::sendRequestToMainsail(String type, String command, const char *data) {
  if (_debug)
    Serial.println("MainsailApi::sendRequestToMainsail() CALLED");

  if ((type != "GET") && (type != "POST")) {
    if (_debug)
      Serial.println("MainsailApi::sendRequestToMainsail() Only GET & POST are supported... exiting.");
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

  connected = _client->connect(_MainsailIp, _MainsailPort);
  
  if (connected) {
    if (_debug)
      Serial.println(".... connected to server");

    char useragent[64];
    snprintf(useragent, 64, "User-Agent: %s", USER_AGENT);

    _client->println(type + " " + command + " HTTP/1.1");
    _client->print("Host: ");
    _client->println(_MainsailIp);
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

String MainsailApi::sendGetToMainsail(String command) {
  if (_debug)
    Serial.println("MainsailApi::sendGetToMainsail() CALLED");

  return sendRequestToMainsail("GET", command, NULL);
}


bool MainsailApi::getPrinterStatus() {
  String response = sendGetToMainsail("/printer/objects/query?webhooks&virtual_sdcard&print_stats&heater_bed&extruder&display_status");
  JsonDocument doc;
  if (!deserializeJson(doc, response)) {
	P_Status.connectionState = "success";                                
    P_Status.bedTemp = doc["result"]["status"]["heater_bed"]["temperature"].as<String>().toFloat();
    P_Status.bedTarget = doc["result"]["status"]["heater_bed"]["target"].as<String>().toFloat();
    P_Status.extruderTemp = doc["result"]["status"]["extruder"]["temperature"].as<String>().toFloat();
    P_Status.extruderTarget = doc["result"]["status"]["extruder"]["target"].as<String>().toFloat();
    P_Status.isActive = (doc["result"]["status"]["virtual_sdcard"]["is_active"] == true);
    P_Status.progress = (doc["result"]["status"]["virtual_sdcard"]["progress"].as<String>().toFloat())*100;                
    P_Status.state = doc["result"]["status"]["print_stats"]["state"].as<String>();
    P_Status.filename = doc["result"]["status"]["print_stats"]["filename"].as<String>();
    P_Status.printDuration = doc["result"]["status"]["print_stats"]["print_duration"].as<String>().toInt();
    P_Status.total_layer = doc["result"]["status"]["print_stats"]["info"]["total_layer"].as<String>().toInt();
    P_Status.current_layer = doc["result"]["status"]["print_stats"]["info"]["current_layer"].as<String>().toInt();
	P_Status.filament_used = doc["result"]["status"]["print_stats"]["filament_used"].as<String>().toInt();
	return true;
  }
  return false;
}

/**
 * Extract the HTTP header response code. Used for error reporting - will print in serial monitor any non 200 response codes (i.e. if something has gone wrong!).
 * Thanks Brian for the start of this function, and the chuckle of watching you realise on a live stream that I didn't use the response code at that time! :)
 * */
int MainsailApi::extractHttpCode(String statusCode, String body) {
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

/**
 * Close the client
 * */
void MainsailApi::closeClient() { _client->stop(); }
