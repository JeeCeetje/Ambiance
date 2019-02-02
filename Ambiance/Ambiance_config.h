/* 
 * ---------------------------------------------------------------------
 *  Design          : Ambiance Configuration File
 * ---------------------------------------------------------------------
 *  Creation date   : 2017-07-25
 *  Author(s)       : JeeCee
 *  Description     : LED control
 * ---------------------------------------------------------------------
 */



/* ---------------------------------------------------------------------
 *  Project
 * --------------------------------------------------------------------- */

#define PROJECT_NAME            "Ambiance"
#define PROJECT_VERSION         "01.06"
#define PROJECT_DATE            __DATE__
#define PROJECT_TIME            __TIME__



/* ---------------------------------------------------------------------
 *  General
 * --------------------------------------------------------------------- */

#define RED                     0
#define GREEN                   1
#define BLUE                    2

#define DEBUG                   1
#define DEBUG_PREFIX            "<"PROJECT_NAME"> "
#define DEBUG_BUFFER_SIZE       100

#define MAX_CMD_LENGTH          130
#define MAX_REPLY_LENGTH        250



/* ---------------------------------------------------------------------
 *  Serial
 * --------------------------------------------------------------------- */

#define SERIAL_BAUD             115200



/* ---------------------------------------------------------------------
 *  WiFi
 * --------------------------------------------------------------------- */

#define WIFI_SSID               YOUR_SSID
#define WIFI_PASSWORD           YOUR_PASSWORD
#define WIFI_HOSTNAME           PROJECT_NAME

#define WIFI_STATUSLED_TIMEOUT  5000

#define WIFI_WEBSERVER_PORT     80



/* ---------------------------------------------------------------------
 *  MQTT
 * --------------------------------------------------------------------- */

#define MQTT_VERSION            MQTT_VERSION_3_1_1
#define MQTT_CLIENT_ID          "livingroom"
#define MQTT_SERVER_IP          YOUR_MQTT_SERVER_IP
#define MQTT_SERVER_PORT        1883
#define MQTT_USER               YOUR_MQTT_USER
#define MQTT_PASSWORD           YOUR_MQTT_PASSWORD
#define MQTT_SENSOR_TOPIC       "home/livingroom"
#define MQTT_RECONNECT_INTERVAL (1*60*1000)         // 1 min

#define MQTT_LIGTH_STATE_TOPIC  "home/ambiance/light/stat"
#define MQTT_LIGTH_CMD_TOPIC    "home/ambiance/light/cmd"

#define MQTT_LIGHT_ON           "ON"
#define MQTT_LIGHT_OFF          "OFF"



/* ---------------------------------------------------------------------
 *  Sensor
 * --------------------------------------------------------------------- */

#define DHT_PIN                 D2
#define DHT_TYPE                DHT11

#define DHT_INTERVAL            (5*60*1000)         // 5 min



/* ---------------------------------------------------------------------
 *  LEDs
 * --------------------------------------------------------------------- */

#define LED_RED                 D5
#define LED_GREEN               D1
#define LED_BLUE                D7

#define LED_FLASH_INTERVAL      500
#define LED_FLASH_INCR          25
#define LED_FLASH_MINIMUM       25

#define LED_FIRE_INTERVAL       150
#define LED_FIRE_INCR           10
#define LED_FIRE_MINIMUM        20

#define LED_SUNSET_INTERVAL     (2*60*60*1000)/503  // 2 hour in ms divided by 503 steps
#define LED_SUNSET_INCR         (  10*60*1000)/503  // 10 min in ms divided by 503 steps
#define LED_SUNSET_MINIMUM      500

#define LED_CHRISTMAS_01_INTERVAL   500
#define LED_CHRISTMAS_01_INCR       100
#define LED_CHRISTMAS_01_MINIMUM    100

#define LED_CHRISTMAS_02_INTERVAL   (10*1000)/(4*236)   // 10 seconds in ms divided by (4*236) steps
#define LED_CHRISTMAS_02_INCR       2
#define LED_CHRISTMAS_02_MINIMUM    2

#define LED_FADE_01_INTERVAL   4*1000               // 4 seconds in ms, yet to be divided in code
#define LED_FADE_01_INCR       2
#define LED_FADE_01_MINIMUM    2



/* ---------------------------------------------------------------------
 *  Infrared
 * --------------------------------------------------------------------- */

#define IR_RECEIVER             D4                  // D4 is also the system LED, causing it to blink on IR receive
#define IR_TYPE                 NEC

#define NEC_up_rgb              0x00FF3AC5          // Light up (8 levels)
#define NEC_down_rgb            0x00FFBA45          // Light down (8 levels)
#define NEC_play                0x00FF827D          // Pause/Run
#define NEC_power               0x00FF02FD          // On/Off
#define NEC_preset_01           0x00FF1AE5          // Red
#define NEC_preset_02           0x00FF9A65          // Green
#define NEC_preset_03           0x00FFA25D          // Blue
#define NEC_preset_04           0x00FF22DD          // White
#define NEC_preset_05           0x00FF2AD5          // Orange
#define NEC_preset_06           0x00FFAA55          // Light green
#define NEC_preset_07           0x00FF926D          // Deep blue
#define NEC_preset_08           0x00FF12ED          // Milk white
#define NEC_preset_09           0x00FF0AF5          // Deep yellow
#define NEC_preset_10           0x00FF8A75          // Cyan
#define NEC_preset_11           0x00FFB24D          // Blue-purple
#define NEC_preset_12           0x00FF32CD          // Pink white
#define NEC_preset_13           0x00FF38C7          // Yellow
#define NEC_preset_14           0x00FFB847          // Light blue
#define NEC_preset_15           0x00FF7887          // Purple
#define NEC_preset_16           0x00FFF807          // Green white
#define NEC_preset_17           0x00FF18E7          // Light yellow
#define NEC_preset_18           0x00FF9867          // Sky blue
#define NEC_preset_19           0x00FF58A7          // Brown
#define NEC_preset_20           0x00FFD827          // Blue white
#define NEC_up_r                0x00FF28D7          // Increase red
#define NEC_up_g                0x00FFA857          // Increase green
#define NEC_up_b                0x00FF6897          // Increase blue
#define NEC_up_s                0x00FFE817          // Faster
#define NEC_down_r              0x00FF08F7          // Reduce red
#define NEC_down_g              0x00FF8877          // Reduce green
#define NEC_down_b              0x00FF48B7          // Reduce blue
#define NEC_down_s              0x00FFC837          // Slower
#define NEC_diy_1               0x00FF30CF          // User defined 1
#define NEC_diy_2               0x00FFB04F          // User defined 2
#define NEC_diy_3               0x00FF708F          // User defined 3
#define NEC_auto                0x00FFF00F          // Auto color change
#define NEC_diy_4               0x00FF10EF          // User defined 4
#define NEC_diy_5               0x00FF906F          // User defined 5
#define NEC_diy_6               0x00FF50AF          // User defined 6
#define NEC_flash               0x00FFD02F          // Flash
#define NEC_jump_3              0x00FF20DF          // 3 color change (red -> green -> blue)
#define NEC_jump_7              0x00FFA05F          // 7 color change (red -> green -> blue -> yellow -> cyan -> purple -> white)
#define NEC_fade_3              0x00FF609F          // 3 color gradual change (red -> green -> blue)
#define NEC_fade_7              0x00FFE01F          // 7 color gradual change (red -> green -> blue -> yellow -> cyan -> purple -> white)
#define NEC_repeat              0xFFFFFFFFFFFFFFFF  // IRremoteESP8266 defines NEC as 32bit unsigned but uses a 64bit decoded value

