/*
 * RESTful web server for responsive UI app (I/O control).
 * Author: Fraaco Fiorese
 * Date: 21-Aug-2016
 */

#include <FS.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>

// Fill in your WiFi router SSID and password

// global contstant
const char* ssid = "f2hex-hs";
const char* password = "BLACKTURKEY";
//const char* ssid = "aristofane";
//const char* password = "ms5kkvprk4ma8";

// global vars
MDNSResponder mdns;
ESP8266WebServer server(80);
bool run_ok = false;

// GPIO#0 is on pin 2
const int LEDPIN = 2;
int dport[10];

/*
 * read data from the specified channel (GPIO)
 */
String read_channel(int chan) {
  Serial.print("pin read: ");
  Serial.println(dport[0]);
  Serial.println(dport[0] ? "on": "off");
  return dport[0] ? "on": "off";
}

/*
 * write data to the specified channel (GPIO)
 */
void write_channel(int chan, bool val) {
    digitalWrite(LEDPIN, !val);
    dport[0] = val;
}

/*
 * Send HTTP response status and associated message
 */
void send_http_resp(int code, String msg) {
    server.sendHeader("Connection", "close");
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(code, "text/plain", msg + "\r\n");
}

/*
 * Send back an OK response to web client with associated status message
 */
void send_ok_resp(String sts_msg) {
    server.sendHeader("Connection", "close");
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", sts_msg + "\r\n");
}

void handle_root() {
    File rf = SPIFFS.open("/index.min.html.gz", "r");
    if (!rf) {
	Serial.println("file open failed");
	send_http_resp(500, "index file unavailable");
    }
    else {
	size_t ss = server.streamFile(rf, "text/html");
	if (ss != rf.size()) {
	    Serial.println("root page sent error: does not match real size");
	}
	rf.close();
    }

//    server.sendHeader("Content-Encoding", "gzip");
//    server.send_P(200, "text/html", index_html, index_html_len);
}

/*
 * Execute a Set command against a specific channel
 */
String execSetCommand(String cmd, int chan) {
    Serial.println("set: [" + cmd + "] chan: " + String(chan));

    write_channel(chan, cmd == "on" ? true:false);
    return read_channel(chan); 
}


/*
 * Execute a Get command against a specific channel
 */
String execGetCommand(String cmd, int chan) {
  String res = "err: inv cmd";

  Serial.println("get: [" + cmd + "] chan: " + String(chan));

  if (cmd == "status") {
      res = read_channel(chan);
  }
  return res; 
}

/*
 * Handle API oper call
 */
void handle_API_oper() {

    // check if no passed args, that case is clearly an error
    if (server.args() == 0) {
	return server.send(500, "text/plain", "BAD ARGS");
    }
    else {
	StaticJsonBuffer<200> jbuf;
	JsonObject& data = jbuf.parseObject(server.arg("plain"));
	if (data.success()) {
	    String oper = data["oper"];
	    String cmd = data["cmd"];
	    String chan = data["chan"]; 
	    Serial.println("oper: [" + oper + "]");
	    if (oper == "set") {
		send_ok_resp(execSetCommand(cmd, chan.toInt()));
	    }
	    else if (oper == "get") {
		send_ok_resp(execGetCommand(cmd, chan.toInt()));
	    }
	    else {
		send_http_resp(400, "unknown oper");    
	    }
	}
	else {
	    Serial.println("JSON error");
	    send_http_resp(500, "invalid json data");
	}
    }
}

/*
 * invalid request handler
 */
void handle_inv_request() {
    String message = "Invalid request\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET)?"GET":"POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i=0; i<server.args(); i++){
	message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    send_http_resp(404, message);
}


/*
 * Perform initial system setup
 */
void setup(void) {
    pinMode(LEDPIN, OUTPUT);
    write_channel(0, false);

    write_channel(1, true);
    
    Serial.begin(115200);
    WiFi.begin(ssid, password);
  
    // wait for connection to complete
    while (WiFi.status() != WL_CONNECTED) {
	delay(500);
	Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    if (mdns.begin("rwb", WiFi.localIP())) {
	Serial.println("MDNS responder started");
    }
    
    // setup server request handlers
    server.on("/", handle_root);
    server.on("/oper", handle_API_oper);
    server.onNotFound(handle_inv_request);

    server.begin();
    Serial.print("Connect to http://rwb.local or http://");
    Serial.println(WiFi.localIP());


    if (SPIFFS.begin()) {
	FSInfo fsi;
	// mount file system
	SPIFFS.info(fsi);
	// dump some useful info about file system current state
	Serial.print("== File system info ==\nTotal bytes: ");
	Serial.println(fsi.totalBytes);
	Serial.print("Used bytes: ");
	Serial.println(fsi.usedBytes);
	Serial.print("Block size: ");
	Serial.println(fsi.pageSize);
	Serial.print("maxOpenFiles: ");
	Serial.println(fsi.maxOpenFiles);
	Serial.print("maxPathLength: ");
	Serial.println(fsi.maxPathLength);

	Serial.println("SPIFFS contents");
	Dir dir = SPIFFS.openDir("/");
	while (dir.next()) {
	    Serial.print(dir.fileName());
	    File f = dir.openFile("r");
	    Serial.print("    ");
	    Serial.println(f.size());
	}
	run_ok = true;
    }
    else {
	Serial.println("=== Error mounting file system ==");
    }
}

/*
 * Main execution loop
 */
void loop(void) {
    if (run_ok) {
	server.handleClient();
    }
}

