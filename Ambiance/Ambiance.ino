/* 
 * ---------------------------------------------------------------------
 *  Design          : Ambiance
 * ---------------------------------------------------------------------
 *  Creation date   : 2017-07-24
 *  Author(s)       : JeeCee
 *  Description     : LED control
 * ---------------------------------------------------------------------
 *  Sources :
 *      https://www.arduino.cc/en/Main/ArduinoBoardProMini
 *      https://www.pjrc.com/teensy/td_libs_IRremote.html
 *      https://github.com/markszabo/IRremoteESP8266
 *      https://learn.adafruit.com/led-tricks-gamma-correction/
 *      https://github.com/Edzelf/Esp-radio
 *      https://www.netspotapp.com/what-is-rssi-level.html
 *      http://www.metageek.com/training/resources/understanding-rssi.html
 *      https://github.com/mertenats/open-home-automation/tree/master/ha_mqtt_sensor_dht22
 * ---------------------------------------------------------------------
 */



/* ---------------------------------------------------------------------
 *  Includes
 * --------------------------------------------------------------------- */

// Configuration
#include "Ambiance_config.h"

// Libraries
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <string.h>
#include <PubSubClient.h>
#include "DHT.h"
#include <ArduinoJson.h>

// ESP8266 SDK access (needed to set hostname)
extern "C" {
#include "user_interface.h"
}

// Webserver files
#include "remote_html.h"
#include "remote_css.h"
#include "remote_js.h"
#include "favicon_ico.h"
#include "favicon_png.h"



/* ---------------------------------------------------------------------
 *  Constants
 * --------------------------------------------------------------------- */

const uint16_t PROGMEM gamma_8to10bit[] = {
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  3,  3,
      3,  3,  4,  4,  4,  5,  5,  5,  6,  6,  7,  7,  7,  8,  8,  9,
     10, 10, 11, 11, 12, 13, 13, 14, 15, 15, 16, 17, 18, 19, 20, 20,
     21, 22, 23, 24, 25, 26, 27, 29, 30, 31, 32, 33, 35, 36, 37, 38,
     40, 41, 43, 44, 46, 47, 49, 50, 52, 54, 55, 57, 59, 61, 63, 64,
     66, 68, 70, 72, 74, 77, 79, 81, 83, 85, 88, 90, 92, 95, 97,100,
    102,105,107,110,113,115,118,121,124,127,130,133,136,139,142,145,
    149,152,155,158,162,165,169,172,176,180,183,187,191,195,199,203,
    207,211,215,219,223,227,232,236,240,245,249,254,258,263,268,273,
    277,282,287,292,297,302,308,313,318,323,329,334,340,345,351,357,
    362,368,374,380,386,392,398,404,410,417,423,429,436,442,449,455,
    462,469,476,483,490,497,504,511,518,525,533,540,548,555,563,571,
    578,586,594,602,610,618,626,634,643,651,660,668,677,685,694,703,
    712,721,730,739,748,757,766,776,785,795,804,814,824,833,843,853,
    863,873,884,894,904,915,925,936,946,957,968,979,990,1001,1012,1023
};



/* ---------------------------------------------------------------------
 *  Types
 * --------------------------------------------------------------------- */

enum LedMode {
    ledOff,
    ledStatic,
    ledTimeout,
    ledFlash,
    ledFire,
    ledSunset,
    ledChristmas_01,
    ledChristmas_02,
    ledFade_01
};


struct LedControl{
    LedMode         currentMode         = ledOff;
    LedMode         previousMode        = ledOff;
    int             subMode             = 0;
    boolean         pause               = false;
    boolean         offOverride         = false;
    boolean         updatePending       = false;
    int             rgb[3]              = {0,0,0};
    int             rgbReference[3]     = {0,0,0};
    int             rgbIncrValue        = 10;
    int             fader               = 0;
    int             faderMax            = 0;
    unsigned long   lastMillis          = millis();
    unsigned long   interval            = LED_FIRE_INTERVAL;
    unsigned long   intervalIncrValue   = LED_FIRE_INCR;
    unsigned long   intervalBaseValue   = LED_FIRE_INTERVAL;
    unsigned long   intervalMinimum     = LED_FIRE_MINIMUM;
};



/* ---------------------------------------------------------------------
 *  Global variables
 * --------------------------------------------------------------------- */

// WiFi
boolean             wifiConnected   = false;
AsyncWebServer      server(WIFI_WEBSERVER_PORT);


// MQTT
WiFiClient          wifiClient;
PubSubClient        client(wifiClient);
unsigned long       client_lastMillis;


// Sensor
DHT                 dht(DHT_PIN, DHT_TYPE);
unsigned long       dht_lastMillis  = millis();
float               dht_humidity    = 0.0;
float               dht_temperature = 0.0;


// LEDs
LedControl          ledControl;


// Infrared
IRrecv              irRecv(IR_RECEIVER);
decode_results      irResults;
uint64_t            NEC_Code        = NEC_power;
boolean             NEC_Repeatable  = false;


// General
char                cmd[MAX_CMD_LENGTH];



/* ---------------------------------------------------------------------
 *  Function prototypes
 * --------------------------------------------------------------------- */

void handleLEDs(boolean argForced = false);
void handleMQTT(boolean argForced = false);



/* ---------------------------------------------------------------------
 *  Helper functions
 * --------------------------------------------------------------------- */

// Formatted debug print
char* dbgPrint(const char* argFormat, ...) {
    static char sbuf[DEBUG_BUFFER_SIZE];
    va_list varArgs;
    
    va_start(varArgs, argFormat);
    vsnprintf(sbuf, sizeof(sbuf), argFormat, varArgs);
    va_end(varArgs);
    
    if (DEBUG) {
        Serial.print  (DEBUG_PREFIX);
        Serial.println(sbuf);
    }
    return sbuf;
}


// Strip comments, trailing CR, leading/trailing spaces
String chomp(String str) {
    int i;
    
    if ((i = str.indexOf("#")) >= 0) {
        str.remove(i);
    }
    str.trim();
    return str;
}



/* ---------------------------------------------------------------------
 *  WiFi functions (connection, OTA, webserver...)
 * --------------------------------------------------------------------- */

// Connect to WiFi
boolean connectWifi() {
    WiFi.disconnect();                                  // After restart the router could
    WiFi.softAPdisconnect(true);                        // still keep the old connection
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    dbgPrint("Try connecting to WiFi network '%s'", WIFI_SSID);
    
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        dbgPrint("WiFi connection failed!");
        return false;
    }
    
    dbgPrint("IP = %d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
    return true;
}


// WiFi connection status via LED output
void statusWifi() {
    setLedMode(ledTimeout);
    setInterval(WIFI_STATUSLED_TIMEOUT, 0, WIFI_STATUSLED_TIMEOUT);
    if (!wifiConnected) {                               // Never connected
        setRGB(255,0,0);                                //  '-> red
        dbgPrint("WiFi connection failed!");
    } else if (WiFi.status() != WL_CONNECTED) {         // Was connected
        setRGB(0,127,255);                              //  '-> blue
        dbgPrint("WiFi connection currently down...");
    } else {
        long rssi = WiFi.RSSI();
        if (rssi > -55) {                               // High quality:    90% ~= -55db
            setRGB(0,255,0);                            //  '-> green
        } else if (rssi > -75) {                        // Medium quality:  50% ~= -75db
            setRGB(255,255,0);                          //  '-> yellow-green
        } else if (rssi > -85) {                        // Low quality:     30% ~= -85db
            setRGB(255,127,0);                          //  '-> orange
        } else {                                        // Unusable quality: 8% ~= -96db
            setRGB(255,0,0);                            //  '-> red
        }
        dbgPrint("WiFi connection up and running (RSSI=%d)", rssi);
    }
    setUpdateFlag();
}


// Over-The-Air update - actions when started
void otaStart() {
    dbgPrint("OTA started");
    setLedMode(ledOff);
    handleLEDs(true);
}


// Over-The-Air update - actions when ended
void otaEnd() {
    dbgPrint("OTA ended");
}


// MQTT - callback function, called when a MQTT message arrived
void mqttCallback(char* p_topic, byte* p_payload, unsigned int p_length) {
    // Concat the payload into a string
    String payload;
    for (uint8_t i = 0; i < p_length; i++) {
        payload.concat((char)p_payload[i]);
    }

    // Handle message topic
    if (String(MQTT_LIGTH_CMD_TOPIC).equals(p_topic)) {
        // Test if the payload is equal to "ON" or "OFF"
        if (payload.equals(String(MQTT_LIGHT_ON))) {
            if (ledControl.currentMode == ledOff) {
                setLedMode(ledOff);
            }
        } else if (payload.equals(String(MQTT_LIGHT_OFF))) {
            if (ledControl.currentMode != ledOff) {
                setLedMode(ledOff);
            }
        }
    }
}


// MQTT - try to reconnect
void mqttReconnect() {
    if (millis() - client_lastMillis < MQTT_RECONNECT_INTERVAL) {
        return;
    }
    client_lastMillis = millis();
    dbgPrint("Attempting MQTT connection...");
    if (client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD)) {
        dbgPrint("Reconnected to MQTT broker");
        client.subscribe(MQTT_LIGTH_CMD_TOPIC);
    } else {
        dbgPrint("Failed to reconnect to MQTT broker; %s", client.state());
    }
}


// MQTT - publish the temperature and the humidity
void publishSensorData(boolean argForced) {
    // Unless forced, publish only at defined intervals
    if ((argForced == false) && (millis() - dht_lastMillis < DHT_INTERVAL)) {
        return;
    }
    dht_lastMillis = millis();
        
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    dht_humidity = dht.readHumidity();
    // Read temperature as Celsius (the default)
    dht_temperature = dht.readTemperature();

    if (isnan(dht_humidity) || isnan(dht_temperature)) {
        dbgPrint("Failed to read from DHT sensor!");
        return;
    }
    
    dbgPrint("Publishing temperature = %d and humidity =%d to MQTT broker", (int)dht_temperature, (int)dht_humidity);
    
    // Create a JSON object
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    // INFO: the data must be converted into a string; a problem occurs when using floats...
    root["temperature"] = (String)dht_temperature;
    root["humidity"] = (String)dht_humidity;
    root.prettyPrintTo(Serial);
    dbgPrint("");
    /*
    {
        "temperature": "23.20" ,
        "humidity": "43.70"
    }
    */
    char data[200];
    root.printTo(data, root.measureLength() + 1);
    client.publish(MQTT_SENSOR_TOPIC, data, true);
    yield();
}


// Return what kind of file is requested
String getContentType(String filename) {
    if      (filename.endsWith(".html")) return "text/html";
    else if (filename.endsWith(".png" )) return "image/png";
    else if (filename.endsWith(".gif" )) return "image/gif";
    else if (filename.endsWith(".jpg" )) return "image/jpeg";
    else if (filename.endsWith(".ico" )) return "image/x-icon";
    else if (filename.endsWith(".css" )) return "text/css";
    else if (filename.endsWith(".js"  )) return "application/javascript";
    else if (filename.endsWith(".zip" )) return "application/x-zip";
    else if (filename.endsWith(".gz"  )) return "application/x-gzip";
    else if (filename.endsWith(".mp3" )) return "audio/mpeg";
    else if (filename.endsWith(".pw"  )) return "";                         // Passwords are secret
    return "text/plain";
}


// Handling of requested files
void handleFSf(AsyncWebServerRequest* request, const String& filename) {
    static String          contenttype;
    AsyncWebServerResponse *response;                                       // For extra headers
    
    dbgPrint("FileRequest received %s", filename.c_str());
    
    contenttype = getContentType(filename);
    if ((contenttype == "") || (filename == "")) {                          // Empty is illegal
        request->send(404, "text/plain", "404 - File not found");
        dbgPrint("Response sent - File not found");
        return;
    } else {
        if (filename.indexOf("remote.html") >= 0) {
            response = request->beginResponse_P(200, contenttype, remote_html);
        } else if (filename.indexOf("remote.css") >= 0) {
            response = request->beginResponse_P(200, contenttype, remote_css);
        } else if (filename.indexOf("remote.js") >= 0) {
            response = request->beginResponse_P(200, contenttype, remote_js);
        } else if (filename.indexOf("favicon.ico") >= 0) {
            response = request->beginResponse_P(200, contenttype, favicon_ico, sizeof(favicon_ico));
        } else if (filename.indexOf("favicon.png") >= 0) {
            response = request->beginResponse_P(200, contenttype, favicon_png, sizeof(favicon_png));
        } else {
            request->send(404, "text/plain", "404 - File not found");
            dbgPrint("Response sent - File not found");
            return;
        }
        // Add extra headers
        response->addHeader("Server", PROJECT_NAME);
        response->addHeader("Cache-Control", "max-age=3600");
        response->addHeader("Last-Modified", PROJECT_DATE" "PROJECT_TIME);
        request->send(response);
    }
    dbgPrint("Response sent");
}


// Handling of requested files
void handleFS(AsyncWebServerRequest* request) {
    handleFSf(request, request->url());
}


// Handling of remote commands with form "/?parameter[=value]" (multiple are ignored)
void handleCmd(AsyncWebServerRequest* request) {
    AsyncWebParameter* p;
    static String      argument;
    static String      value;
    const char*        reply;
    int                params;
    
    params = request->params();                         // Get number of arguments
    if (params == 0) {
        handleFSf(request, String("/remote.html"));     // No parameters, send the startpage
        return;
    }
    
    p = request->getParam(0);
    argument = p->name();
    argument.toLowerCase();
    value = p->value();
    
    reply = analyzeCmd(argument.c_str(), value.c_str());
    request->send(200, "text/plain", reply);
}



/* ---------------------------------------------------------------------
 *  Command handling functions
 * --------------------------------------------------------------------- */

// Handle command pair <parameter> and <value>
char* analyzeCmd(const char* par, const char* val) {
    String          argument;                           // Argument as string
    String          value;                              // Value of an argument as a string
    int             ivalue;                             // Value of argument as an integer
    static char     reply[MAX_REPLY_LENGTH];            // Reply to client, will be returned
    int             inx;                                // Index in string
    
    strcpy(reply, "Command accepted");                  // Default reply
    argument = chomp(par);                              // Get the argument
    if (argument.length() == 0) {                       // Empty commandline (comment?)
        return reply;
    }
    argument.toLowerCase();
    
    value = chomp(val);                                 // Get the specified value
    ivalue = value.toInt();                             // Also as an integer
    ivalue = abs(ivalue);                               // Make it absolute
    if (value.length()) {
        dbgPrint("Command '%s' with parameter '%s'", argument.c_str(), value.c_str());
    } else {
        dbgPrint("Command '%s' (without parameter)", argument.c_str());
    }
    
    if (argument == "power") {
        setLedMode(ledOff);
    } else if (argument == "play") {
        togglePause();
    } else if (argument == "up") {
        if (value.indexOf("s") >= 0) {
            decrInterval();
        } else {
            incrRGB(
                (value.indexOf("r") >= 0),
                (value.indexOf("g") >= 0),
                (value.indexOf("b") >= 0)
            );
        }
        NEC_Repeatable = true;
    } else if (argument == "down") {
        if (value.indexOf("s") >= 0) {
            incrInterval();
        } else {
            decrRGB(
                (value.indexOf("r") >= 0),
                (value.indexOf("g") >= 0),
                (value.indexOf("b") >= 0)
            );
        }
        NEC_Repeatable = true;
    } else if (argument == "preset") {
        setLedMode(ledStatic);
        switch (ivalue) {
            case  1 : setRGB(255,  0,  0); break;
            case  2 : setRGB(  0,255,  0); break;
            case  3 : setRGB(  0,  0,255); break;
            case  4 : setRGB(255,255,255); break;
            case  5 : setRGB(255, 85,  0); break;
            case  6 : setRGB(  0,255, 85); break;
            case  7 : setRGB( 85,  0,255); break;
            case  8 : setRGB(231,139,204); break;
            case  9 : setRGB(255,170,  0); break;
            case 10 : setRGB(  0,255,170); break;
            case 11 : setRGB(170,  0,255); break;
            case 12 : setRGB(240,187,224); break;
            case 13 : setRGB(255,255,  0); break;
            case 14 : setRGB(  0,255,255); break;
            case 15 : setRGB(255,  0,255); break;
            case 16 : setRGB(120,215,215); break;
            case 17 : setRGB(170,255,  0); break;
            case 18 : setRGB(  0,170,255); break;
            case 19 : setRGB(255,  0,170); break;
            case 20 : setRGB(170,245,245); break;
            default : setLedMode(ledOff);  break;
        }
    } else if (argument == "diy") {
        switch (ivalue) {
            case  1 :
                setLedMode(ledFire);
                setInterval(LED_FIRE_INTERVAL, LED_FIRE_INCR, LED_FIRE_MINIMUM);
                setRGB(255,153,0);
                break;
            case  2 :
                setLedMode(ledSunset);
                setInterval(LED_SUNSET_INTERVAL, LED_SUNSET_INCR, LED_SUNSET_MINIMUM);
                setRGB(120,215,215);
                break;
            case  3 :
                setLedMode(ledChristmas_01);
                setInterval(LED_CHRISTMAS_01_INTERVAL, LED_CHRISTMAS_01_INCR, LED_CHRISTMAS_01_MINIMUM);
                setRGB(255,0,0);
                break;
            case  4 :
                setLedMode(ledChristmas_02);
                setInterval(LED_CHRISTMAS_02_INTERVAL, LED_CHRISTMAS_02_INCR, LED_CHRISTMAS_02_MINIMUM);
                setRGB(255,0,0);
                break;
                break;
            case  5 :
                break;
            case  6 :
                statusWifi();
                break;
            default :
                break;
        }
    } else if (argument == "auto") {
        setLedMode(ledStatic);
        setRGB(random(0,256),random(0,256),random(0,256));
    } else if (argument == "flash") {
        setLedMode(ledFlash);
        setInterval(LED_FLASH_INTERVAL, LED_FLASH_INCR, LED_FLASH_MINIMUM);
    } else if (argument == "jump") {
        // switch (ivalue) {
            // case  3 : break;
            // case  7 : break;
            // default : break;
        // }
    } else if (argument == "fade") {
        setLedMode(ledFade_01);
        ledControl.rgbReference[RED]   = ledControl.rgb[RED]  ;
        ledControl.rgbReference[GREEN] = ledControl.rgb[GREEN];
        ledControl.rgbReference[BLUE]  = ledControl.rgb[BLUE] ;
        ledControl.faderMax            = ledControl.rgb[RED];
        if (ledControl.rgb[RED] < ledControl.rgb[GREEN]) {
            ledControl.faderMax        = ledControl.rgb[GREEN];
        }
        if (ledControl.rgb[GREEN] < ledControl.rgb[BLUE]) {
            ledControl.faderMax        = ledControl.rgb[BLUE];
        }
        ledControl.fader               = ledControl.faderMax;
        setInterval((LED_FADE_01_INTERVAL/ledControl.faderMax), LED_FADE_01_INCR, LED_FADE_01_MINIMUM);
        // switch (ivalue) {
            // case  3 : break;
            // case  7 : break;
            // default : break;
        // }
    } else if (argument == "rgb") {
        char str[11];
        strncpy(str, val, sizeof(str));
        int vals[3] = {0,0,0};
        int i = 0;
        char *p = strtok(str,",");
        while (p != NULL) {
            // http://www.cplusplus.com/reference/cstring/strstr/
            // http://www.cplusplus.com/reference/cstring/strncpy/
            // https://hackingmajenkoblog.wordpress.com/2017/04/08/splitting-up-text-in-c/
            // https://forum.arduino.cc/index.php?topic=387175.0
            // https://stackoverflow.com/questions/15472299/split-string-into-tokens-and-save-them-in-an-array
            vals[i] = chomp(p).toInt();
            i++;
            p = strtok(NULL,",");                       // Continue scanning where the last strtok call ended
        }
        setLedMode(ledStatic);
        setRGB(vals[0],vals[1],vals[2]);
    } else if (argument == "dht") {
        sprintf(reply, "Temperature = %d, humidity = %d", (int)dht_temperature, (int)dht_humidity);
        return reply;
    } else if (argument == "mqtt") {
        handleMQTT(true);
    } else if (argument == "color") {
        // Request the standard color reply
    } else {
        dbgPrint("Command ignored due to illegal parameter '%s'", argument.c_str());
    }
    
    setUpdateFlag();
    
    // Update reply
    if (ledControl.currentMode == ledOff) {
        sprintf(reply, "rgb(0,0,0)");
    } else {
        sprintf(reply, "rgb(%d,%d,%d)", ledControl.rgb[0], ledControl.rgb[1], ledControl.rgb[2]);
    }
    
    return reply;
}


// Handle command string <parameter>=<value>
char* analyzeCmd(const char* str) {
    char* value;
    
    value = strstr(str, "=");                           // See if command contains a "="
    if (value) {
        *value = '\0' ;                                 // Separate command from value
        value++ ;                                       // Points to value after "="
    } else {
        value = (char*) "0" ;                           // No value, assume zero
    }
    return analyzeCmd(str, value);
}



/* ---------------------------------------------------------------------
 *  Main loop handling functions
 * --------------------------------------------------------------------- */

// Handle serial communication
void handleSerial() {
    static String serialcmd;
    char          c;
    char*         reply;
    uint16_t      len;
    
    while (Serial.available()) {
        c   = (char)Serial.read();
        len = serialcmd.length();
        if ((c == '\n') || (c == '\r')) {
            if (len) {
                strncpy(cmd, serialcmd.c_str(), sizeof(cmd));
                reply = analyzeCmd(cmd);
                dbgPrint(reply);
                serialcmd = "";
            }
        }
        if (c >= ' ') {                                 // Only accept useful characters
            serialcmd += c;
        }
        if (len >= (sizeof(cmd)-2)) {                   // Check for excessive length
            serialcmd = "";
        }
    }
}


// Handle IR commands
void handleIRreceiver() {
    // Attempt to receive a IR code
    if (irRecv.decode(&irResults)) {
        
        // Decode only if it is from the expected remote type
        if (irResults.decode_type == NEC) {
            dbgPrint("IR received : 0x%08X", (unsigned int) irResults.value);

            // Handle repeat request
            if (irResults.value == NEC_repeat && NEC_Repeatable == true) {
                // use previous code
            } else {
                NEC_Code = irResults.value;
            }

            // Handle NEC codes
            NEC_Repeatable = false;
            switch (NEC_Code) {
                case NEC_up_rgb    : strcpy(cmd, "up=rgb"   ); break;
                case NEC_down_rgb  : strcpy(cmd, "down=rgb" ); break;
                case NEC_play      : strcpy(cmd, "play"     ); break;
                case NEC_power     : strcpy(cmd, "power"    ); break;
                case NEC_preset_01 : strcpy(cmd, "preset=1" ); break;
                case NEC_preset_02 : strcpy(cmd, "preset=2" ); break;
                case NEC_preset_03 : strcpy(cmd, "preset=3" ); break;
                case NEC_preset_04 : strcpy(cmd, "preset=4" ); break;
                case NEC_preset_05 : strcpy(cmd, "preset=5" ); break;
                case NEC_preset_06 : strcpy(cmd, "preset=6" ); break;
                case NEC_preset_07 : strcpy(cmd, "preset=7" ); break;
                case NEC_preset_08 : strcpy(cmd, "preset=8" ); break;
                case NEC_preset_09 : strcpy(cmd, "preset=9" ); break;
                case NEC_preset_10 : strcpy(cmd, "preset=10"); break;
                case NEC_preset_11 : strcpy(cmd, "preset=11"); break;
                case NEC_preset_12 : strcpy(cmd, "preset=12"); break;
                case NEC_preset_13 : strcpy(cmd, "preset=13"); break;
                case NEC_preset_14 : strcpy(cmd, "preset=14"); break;
                case NEC_preset_15 : strcpy(cmd, "preset=15"); break;
                case NEC_preset_16 : strcpy(cmd, "preset=16"); break;
                case NEC_preset_17 : strcpy(cmd, "preset=17"); break;
                case NEC_preset_18 : strcpy(cmd, "preset=18"); break;
                case NEC_preset_19 : strcpy(cmd, "preset=19"); break;
                case NEC_preset_20 : strcpy(cmd, "preset=20"); break;
                case NEC_up_r      : strcpy(cmd, "up=r"     ); break;
                case NEC_up_g      : strcpy(cmd, "up=g"     ); break;
                case NEC_up_b      : strcpy(cmd, "up=b"     ); break;
                case NEC_up_s      : strcpy(cmd, "up=s"     ); break;
                case NEC_down_r    : strcpy(cmd, "down=r"   ); break;
                case NEC_down_g    : strcpy(cmd, "down=g"   ); break;
                case NEC_down_b    : strcpy(cmd, "down=b"   ); break;
                case NEC_down_s    : strcpy(cmd, "down=s"   ); break;
                case NEC_diy_1     : strcpy(cmd, "diy=1"    ); break;
                case NEC_diy_2     : strcpy(cmd, "diy=2"    ); break;
                case NEC_diy_3     : strcpy(cmd, "diy=3"    ); break;
                case NEC_auto      : strcpy(cmd, "auto"     ); break;
                case NEC_diy_4     : strcpy(cmd, "diy=4"    ); break;
                case NEC_diy_5     : strcpy(cmd, "diy=5"    ); break;
                case NEC_diy_6     : strcpy(cmd, "diy=6"    ); break;
                case NEC_flash     : strcpy(cmd, "flash"    ); break;
                case NEC_jump_3    : strcpy(cmd, "jump=3"   ); break;
                case NEC_jump_7    : strcpy(cmd, "jump=7"   ); break;
                case NEC_fade_3    : strcpy(cmd, "fade=3"   ); break;
                case NEC_fade_7    : strcpy(cmd, "fade=7"   ); break;
                case NEC_repeat    : strcpy(cmd, ""         ); break;
                default            : strcpy(cmd, ""         ); break;
            }
            analyzeCmd(cmd);

        // Inform non-NEC codes
        } else {
            dbgPrint("IR discarded : 0x%08X", (unsigned int) irResults.value);
        }
        
        // Reset the receiver to receive a new code
        irRecv.resume();
    }
}


// Control the LED strip
void handleLEDs(boolean argForced) {
    // Only update if there is an update or when we are forced
    if (ledControl.updatePending == true || argForced == true) {
        ledControl.updatePending = false;

        // If pause, do nothing, always
        if (ledControl.pause == true) {
            return;
        }

        // Set PWM values
        if (ledControl.currentMode == ledOff || ledControl.offOverride == true) {
            analogWrite(LED_RED,   0);
            analogWrite(LED_GREEN, 0);
            analogWrite(LED_BLUE,  0);
        } else {
            analogWrite(LED_RED,   pgm_read_word(&gamma_8to10bit[ledControl.rgb[RED]]));
            analogWrite(LED_GREEN, pgm_read_word(&gamma_8to10bit[ledControl.rgb[GREEN]]));
            analogWrite(LED_BLUE,  pgm_read_word(&gamma_8to10bit[ledControl.rgb[BLUE]]));
        }
    }
}


// Handle the automatic color changes depending on the mode
void handleColorChange() {
    // Check if the interval is elapsed
    if (millis() - ledControl.lastMillis < ledControl.interval) {
        return;
    }

    // If pause, do nothing, always
    if (ledControl.pause == true) {
        return;
    }

    // Act depending on mode
    int     r;
    boolean t;
    switch (ledControl.currentMode) {
        case ledTimeout:
            setLedMode(ledOff);
            break;
            
        case ledFlash:
            if (ledControl.offOverride == true) {
                ledControl.offOverride = false;
            } else {
                ledControl.offOverride = true;
            }
            break;

        case ledFire:
            r = random(10);
            setRGB(255-2*r,153-r,0);
            ledControl.interval = random(ledControl.intervalBaseValue);

        case ledSunset:
            t = true;
            switch (ledControl.subMode) {
                case 0: // Skyblue to yellow
                    if (ledControl.rgb[RED] < 255) {
                        ledControl.rgb[RED] += 3;
                        t = false;
                    }
                    if (ledControl.rgb[GREEN] > 170) {
                        ledControl.rgb[GREEN] -= 1;
                        t = false;
                    }
                    if (ledControl.rgb[BLUE] > 0) {
                        ledControl.rgb[BLUE] -= 3;
                        t = false;
                    }
                    break;
                case 1: // Yellow to reddish
                    if (ledControl.rgb[GREEN] > 85) {
                        ledControl.rgb[GREEN] -= 1;
                        t = false;
                    }
                    break;
                case 2: // Reddish to purple
                    if (ledControl.rgb[RED] > 85) {
                        ledControl.rgb[RED] -= 1;
                        t = false;
                    }
                    if (ledControl.rgb[GREEN] > 0) {
                        ledControl.rgb[GREEN] -= 1;
                        t = false;
                    }
                    if (ledControl.rgb[BLUE] < 170) {
                        ledControl.rgb[BLUE] += 3;
                        t = false;
                    }
                    break;
                case 3: // Purple to darkblue to black
                    if (ledControl.rgb[RED] > 0) {
                        ledControl.rgb[RED] -= 3;
                        t = false;
                    }
                    if (ledControl.rgb[BLUE] > 0) {
                        ledControl.rgb[BLUE] -= 1;
                        t = false;
                    }
                    break;
                default:
                    setLedMode(ledOff);
                    break;
            }
            if (t == true) {
                ledControl.subMode += 1;
            }
            break;
            
        case ledChristmas_01:
            switch (ledControl.subMode) {
                case 0:
                    setRGB(0,255,0);
                    ledControl.subMode += 1;
                    break;
                case 1:
                    setRGB(255,0,0);
                    ledControl.subMode = 0;
                    break;
                default:
                    setLedMode(ledOff);
                    break;
            }
            break;
            
        case ledChristmas_02:
            t = true;
            switch (ledControl.subMode) {
                case 0:
                    if (ledControl.rgb[RED] > 20) {
                        ledControl.rgb[RED] -= 1;
                        t = false;
                    }
                    break;
                case 1:
                    if (ledControl.rgb[GREEN] < 255) {
                        ledControl.rgb[GREEN] += 1;
                        t = false;
                    }
                    break;
                case 2:
                    if (ledControl.rgb[GREEN] > 20) {
                        ledControl.rgb[GREEN] -= 1;
                        t = false;
                    }
                    break;
                case 3:
                    if (ledControl.rgb[RED] < 255) {
                        ledControl.rgb[RED] += 1;
                        t = false;
                    }
                    break;
                default:
                    setLedMode(ledOff);
                    break;
            }
            if (t == true) {
                ledControl.subMode += 1;
                if (ledControl.subMode >= 4) {
                    ledControl.subMode = 0;
                }
            }
            break;
            
        case ledFade_01:
            t = true;
            switch (ledControl.subMode) {
                case 0:
                    if (ledControl.fader > 20) {
                        ledControl.fader -= 1;
                        t = false;
                    }
                    break;
                case 1:
                    if (ledControl.fader < ledControl.faderMax) {
                        ledControl.fader += 1;
                        t = false;
                    }
                    break;
                default:
                    setLedMode(ledOff);
                    break;
            }
            ledControl.rgb[RED]   = ledControl.rgbReference[RED]   * ledControl.fader / ledControl.faderMax;
            ledControl.rgb[GREEN] = ledControl.rgbReference[GREEN] * ledControl.fader / ledControl.faderMax;
            ledControl.rgb[BLUE]  = ledControl.rgbReference[BLUE]  * ledControl.fader / ledControl.faderMax;
            
            
            if (t == true) {
                ledControl.subMode += 1;
                if (ledControl.subMode >= 2) {
                    ledControl.subMode = 0;
                }
            }
            break;
            
        default:
            break;
    }

    // Boundary check
    if (ledControl.rgb[RED]   > 255) ledControl.rgb[RED]   = 255;
    if (ledControl.rgb[GREEN] > 255) ledControl.rgb[GREEN] = 255;
    if (ledControl.rgb[BLUE]  > 255) ledControl.rgb[BLUE]  = 255;
    if (ledControl.rgb[RED]   <   0) ledControl.rgb[RED]   =   0;
    if (ledControl.rgb[GREEN] <   0) ledControl.rgb[GREEN] =   0;
    if (ledControl.rgb[BLUE]  <   0) ledControl.rgb[BLUE]  =   0;

    // Update
    setUpdateFlag();

    // Save time
    ledControl.lastMillis = millis();
}


// Handle MQTT messages
void handleMQTT(boolean argForced) {
    if (!client.connected()) {
        mqttReconnect();
    } else {
        client.loop();
        publishSensorData(argForced);
    }
}



/* ---------------------------------------------------------------------
 *  Led control functions
 * --------------------------------------------------------------------- */

// Set led mode
void setLedMode(LedMode argLedMode) {
    ledControl.pause       = false;
    ledControl.offOverride = false;
    ledControl.lastMillis  = millis();
    ledControl.subMode     = 0;
    if (ledControl.currentMode == ledOff && argLedMode == ledOff) {
        ledControl.currentMode  = ledControl.previousMode;
        ledControl.previousMode = ledOff;
        if (client.connected()) {
            client.publish(MQTT_LIGTH_STATE_TOPIC, "ON", true);
        }
    } else {
        ledControl.previousMode = ledControl.currentMode;
        ledControl.currentMode  = argLedMode;
        if (client.connected()) {
            if (ledControl.currentMode == ledOff) {
                client.publish(MQTT_LIGTH_STATE_TOPIC, "OFF", true);
            } else {
                client.publish(MQTT_LIGTH_STATE_TOPIC, "ON", true);
            }
        }
    }
}


// Toggle pause
void togglePause() {
    if (ledControl.pause == true) {
        ledControl.pause = false;
    } else {
        ledControl.pause = true;
    }
}


// Set red, green and blue values
void setRGB(int argRed, int argGreen, int argBlue) {
    ledControl.rgb[RED]     = argRed;
    ledControl.rgb[GREEN]   = argGreen;
    ledControl.rgb[BLUE]    = argBlue;
}


// Increment/decrement red and/or green and/or blue values
void incrRGB(boolean argRed, boolean argGreen, boolean argBlue) {
    if (argRed)   ledControl.rgb[RED]   += ledControl.rgbIncrValue;
    if (argGreen) ledControl.rgb[GREEN] += ledControl.rgbIncrValue;
    if (argBlue)  ledControl.rgb[BLUE]  += ledControl.rgbIncrValue;
    if (ledControl.rgb[RED]   > 255) ledControl.rgb[RED]   = 255;
    if (ledControl.rgb[GREEN] > 255) ledControl.rgb[GREEN] = 255;
    if (ledControl.rgb[BLUE]  > 255) ledControl.rgb[BLUE]  = 255;
}
void decrRGB(boolean argRed, boolean argGreen, boolean argBlue) {
    if (argRed)   ledControl.rgb[RED]   -= ledControl.rgbIncrValue;
    if (argGreen) ledControl.rgb[GREEN] -= ledControl.rgbIncrValue;
    if (argBlue)  ledControl.rgb[BLUE]  -= ledControl.rgbIncrValue;
    if (ledControl.rgb[RED]   < 0) ledControl.rgb[RED]   = 0;
    if (ledControl.rgb[GREEN] < 0) ledControl.rgb[GREEN] = 0;
    if (ledControl.rgb[BLUE]  < 0) ledControl.rgb[BLUE]  = 0;
}


// Set interval
void setInterval(unsigned long argInterval, unsigned long argIntervalIncrValue, unsigned long argIntervalMinimum) {
    ledControl.interval          = argInterval;
    ledControl.intervalBaseValue = argInterval;
    ledControl.intervalIncrValue = argIntervalIncrValue;
    ledControl.intervalMinimum   = argIntervalMinimum;
}


// Increment/decrement interval
void incrInterval() {
    ledControl.interval          += ledControl.intervalIncrValue;
    ledControl.intervalBaseValue += ledControl.intervalIncrValue;
}
void decrInterval() {
    ledControl.interval          -= ledControl.intervalIncrValue;
    ledControl.intervalBaseValue -= ledControl.intervalIncrValue;
    if (ledControl.interval          < ledControl.intervalMinimum) ledControl.interval          = ledControl.intervalMinimum;
    if (ledControl.intervalBaseValue < ledControl.intervalMinimum) ledControl.intervalBaseValue = ledControl.intervalMinimum;
}


// Set to update
void setUpdateFlag() {
    ledControl.updatePending = true;
}



/* ---------------------------------------------------------------------
 *  Arduino IDE setup() and loop()
 * --------------------------------------------------------------------- */

// Initialize
void setup() {
    // Serial : start at requested baud rate
    Serial.begin(SERIAL_BAUD);
    Serial.println();
    
    // Project information
    dbgPrint("'%s' v%s, compiled at %s %s", PROJECT_NAME, PROJECT_VERSION, PROJECT_DATE, PROJECT_TIME);
    
    // Sensor
    dht.begin();
    
    // LEDs : define pin modes and default pin states
    pinMode     (LED_RED,   OUTPUT);
    pinMode     (LED_GREEN, OUTPUT);
    pinMode     (LED_BLUE,  OUTPUT);
    digitalWrite(LED_RED,   0);
    digitalWrite(LED_GREEN, 0);
    digitalWrite(LED_BLUE,  0);
    
    // WiFi : connection, OTA setup and MQTT setup
    WiFi.persistent(false);                             // Do not save SSID and password
    WiFi.disconnect();                                  // After restart the router could still keep the old connection
    WiFi.mode(WIFI_STA);                                // This ESP is a station
    wifi_station_set_hostname((char*)WIFI_HOSTNAME);
    wifiConnected = connectWifi();
    if (wifiConnected) {
        // OTA
        ArduinoOTA.setHostname(WIFI_HOSTNAME);
        ArduinoOTA.onStart(otaStart);
        ArduinoOTA.onEnd(otaEnd);
        ArduinoOTA.onError([](ota_error_t error) {
            dbgPrint("Error[%u]: ", error);
            if      (error == OTA_AUTH_ERROR   ) dbgPrint("Auth Failed");
            else if (error == OTA_BEGIN_ERROR  ) dbgPrint("Begin Failed");
            else if (error == OTA_CONNECT_ERROR) dbgPrint("Connect Failed");
            else if (error == OTA_RECEIVE_ERROR) dbgPrint("Receive Failed");
            else if (error == OTA_END_ERROR    ) dbgPrint("End Failed");
        });
        ArduinoOTA.begin();
        // MQTT
        client.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
        client.setCallback(mqttCallback);
        client_lastMillis = 0;
    }
    statusWifi();
    
    // WiFi : webserver
    server.on("/", handleCmd);
    server.onNotFound(handleFS); 
    server.begin();
    dbgPrint("Webserver started");
    
    // Infrared : start receiver
    irRecv.enableIRIn();
}


// Main loop
void loop() {
    handleSerial();
    handleIRreceiver();
    handleLEDs();
    handleColorChange();
    handleMQTT();
    ArduinoOTA.handle();
}

