#if !defined ESP32
 #error This sketch is only for an ESP32 Camera module
#endif

#include "esp_camera.h"         // https://github.com/espressif/esp32-camera
#include <Arduino.h>
#include <esp_task_wdt.h>       // watchdog timer   - see: https://iotassistant.io/esp32/enable-hardware-watchdog-timer-esp32-arduino-ide/

// Use this verions - https://github.com/teknynja/Adafruit_NeoPixel/tree/esp32_rmt_memory_allocation_fix_safe
// See: https://forum.arduino.cc/t/neopixel-crash-with-75-pixels-using-esp32-core-3-0-x/1273500/12
#include <Adafruit_NeoPixel.h>
#include <math.h>
//   ---------------------------------------------------------------------------------------------------------



//                          ====================================== 
//                                   Enter your wifi settings
//                          ====================================== 
// ESP32 WiFi Configuration
#define SSID_NAME "Sandwiches"
#define SSID_PASWORD "123456789m"
#define LAST_IP_OCTET 121 // Values are planned between 120 and 140 with no overlap
#define HAS_CAMERA false

// e.g. 192.168.147.120 is the start of the range
#define IP_PREFIX_1 192
#define IP_PREFIX_2 168
#define IP_PREFIX_3 96

//LED Strip Configuration
#define PIN 12
#define N_LEDS 190
#define MAX_BRIGHTNESS 127 // Choose a value between 2 and 255

/* See also these lines in this file
   int cameraImageExposure = 1200;                         // Camera exposure (0 - 1200)   If gain and exposure both set to zero then auto adjust is enabled
   int cameraImageGain = 30;                             // Image gain (0 - 30)
   int cameraImageBrightness = 2;                       // Image brightness (-2 to +2)

   and:
   framesize_t FRAME_SIZE_IMAGE = cyclingRes[4]; // { FRAMESIZE_SVGA, FRAMESIZE_XGA, FRAMESIZE_SXGA, FRAMESIZE_QVGA, FRAMESIZE_VGA }
*/

#define ENABLE_OTA 0                         // If OTA updating of this sketch is enabled (requires ota.h file)
const String OTAPassword = "password";       // Password for performing OTA update (i.e. http://x.x.x.x/ota)




//   ---------------------------------------------------------------------------------------------------------

#define STATIC_IP
#ifdef STATIC_IP
  // Configure this per device
  IPAddress ip(IP_PREFIX_1,IP_PREFIX_2,IP_PREFIX_3,LAST_IP_OCTET);
  IPAddress gateway(IP_PREFIX_1,IP_PREFIX_2,0,1);
  IPAddress subnet(255,255,255,0);
#endif


    // Required by PlatformIO

    // forward declarations
      bool initialiseCamera(bool);            // this sets up and enables the camera (if bool=1 standard settings are applied but 0 allows you to apply custom settings)
      bool cameraImageSettings();             // this applies the image settings to the camera (brightness etc.)
      void changeResolution();                // this changes the capture frame size
      String localTime();                     // returns the current time as a String
      void flashLED(int);                     // flashes the onboard indicator led
      byte storeImage();                      // stores an image in Spiffs or SD card
      void handleRoot();                      // the root web page
      void handlePhoto();                     // web page to capture an image from camera and save to spiffs or sd card
      bool handleImg();                       // Display a previously stored image 
      void handleNotFound();                  // if invalid web page is requested
      void readRGBImage();                    // demo capturing an image and reading its raw RGB data
      bool getNTPtime(int);                   // handle the NTP real time clock
      bool handleJPG();                       // display a raw jpg image
      void handleJpeg();                      // display a raw jpg image which periodically refreshes
      void handleStream();                    // stream live video (note: this can get the camera very hot)
      int requestWebPage(String*, String*, int);  // procedure allowing the sketch to read a web page its self
      void handleTest();                      // test procedure for experimenting with bits of code etc.
      void handleReboot();                    // handle request to restart device
      void handleData();                      // the root web page requests this periodically via Javascript in order to display updating information
      void readGrayscaleImage();              // demo capturing a grayscale image and reading its raw RGB data
      void resize_esp32cam_image_buffer(uint8_t*, int, int, uint8_t*, int, int);    // this resizes a captured grayscale image (used by above)


// ---------------------------------------------------------------
//                           -SETTINGS
// ---------------------------------------------------------------

 char* stitle = "ESP32Cam-demo";                        // title of this sketch
 char* sversion = "01Oct24";                            // Sketch version

 #define WDT_TIMEOUT 60                                 // timeout of watchdog timer (seconds) 

 const bool sendRGBfile = 0;                            // if set to 1 '/rgb' will just return raw rgb data which can then be saved as a file rather than display a HTML pag

 uint16_t dataRefresh = 2;                              // how often to refresh data on root web page (seconds)
 uint16_t imagerefresh = 2;                             // how often to refresh the image on root web page (seconds)

 const bool serialDebug = 1;                            // show debug info. on serial port (1=enabled, disable if using pins 1 and 3 as gpio)
 const int serialSpeed = 115200;                        // Serial data speed to use

 #define useMCP23017 0                                  // set if MCP23017 IO expander chip is being used (on pins 12 and 13)

 // Camera related
   bool flashRequired = 0;                              // If flash to be used when capturing image (1 = yes)
   const framesize_t cyclingRes[] = { FRAMESIZE_SVGA, FRAMESIZE_XGA, FRAMESIZE_SXGA, FRAMESIZE_QVGA, FRAMESIZE_VGA };    // resolutions to use
                                                        // Image resolutions available:
                                                        //               default = "const framesize_t FRAME_SIZE_IMAGE = FRAMESIZE_VGA"
                                                        //               160x120 (QQVGA), 128x160 (QQVGA2), 176x144 (QCIF), 240x176 (HQVGA), 240X240,
                                                        //               320x240 (QVGA), 400x296 (CIF), 640x480 (VGA, default), 800x600 (SVGA),
                                                        //               1024x768 (XGA), 1280x1024 (SXGA), 1600x1200 (UXGA)
   int cameraImageExposure = 0;                         // Camera exposure (0 - 1200)   If gain and exposure both set to zero then auto adjust is enabled
   int cameraImageGain = 0;                             // Image gain (0 - 30)
   int cameraImageBrightness = 0;                       // Image brightness (-2 to +2)
   const int camChangeDelay = 200;                      // delay when deinit camera executed

 const int TimeBetweenStatus = 600;                     // speed of flashing system running ok status light (milliseconds)

 const int indicatorLED = 33;                           // onboard small LED pin (33)

 // Bright LED (Flash)
   const int brightLED = 4;                             // onboard Illumination/flash LED pin (4)
   int brightLEDbrightness = 0;                         // initial brightness (0 - 255)
   const int ledFreq = 5000;                            // PWM settings
   const int ledChannel = 15;                           // camera uses timer1
   const int ledRresolution = 8;                        // resolution (8 = from 0 to 255)

 const int iopinA = 13;                                 // general io pin 13
 const int iopinB = 15;                                 // general io pin 12 (must not be high at boot)


// camera settings (for the standard - OV2640 - CAMERA_MODEL_AI_THINKER)
// see: https://randomnerdtutorials.com/esp32-cam-camera-pin-gpios/
// set camera resolution etc. in 'initialiseCamera()' and 'cameraImageSettings()'
 #define CAMERA_MODEL_AI_THINKER
 #define PWDN_GPIO_NUM     32      // power to camera (on/off)
 #define RESET_GPIO_NUM    -1      // -1 = not used
 #define XCLK_GPIO_NUM      0
 #define SIOD_GPIO_NUM     26      // i2c sda
 #define SIOC_GPIO_NUM     27      // i2c scl
 #define Y9_GPIO_NUM       35
 #define Y8_GPIO_NUM       34
 #define Y7_GPIO_NUM       39
 #define Y6_GPIO_NUM       36
 #define Y5_GPIO_NUM       21
 #define Y4_GPIO_NUM       19
 #define Y3_GPIO_NUM       18
 #define Y2_GPIO_NUM        5
 #define VSYNC_GPIO_NUM    25      // vsync_pin
 #define HREF_GPIO_NUM     23      // href_pin
 #define PCLK_GPIO_NUM     22      // pixel_clock_pin
 camera_config_t config;           // camera settings


// ******************************************************************************************************************


//#include "esp_camera.h"         // https://github.com/espressif/esp32-camera
// #include "camera_pins.h"
framesize_t FRAME_SIZE_IMAGE = cyclingRes[4]; // { FRAMESIZE_SVGA, FRAMESIZE_XGA, FRAMESIZE_SXGA, FRAMESIZE_QVGA, FRAMESIZE_VGA }
#include <WString.h>            // this is required for base64.h otherwise get errors with esp32 core 1.0.6 - jan23
#include <base64.h>             // for encoding buffer to display image on page
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include "driver/ledc.h"        // used to configure pwm on illumination led

// NTP - Internet time - see - https://randomnerdtutorials.com/esp32-ntp-timezones-daylight-saving/
  #include "time.h"
  struct tm timeinfo;
  const char* ntpServer = "pool.ntp.org";
  const char* TZ_INFO    = "EST5EDT";  // enter your time zone (https://remotemonitoringsystems.ca/time-zone-abbreviations.php)
  long unsigned lastNTPtime;
  time_t now;
   
// spiffs used to store images if no sd card present
 #include <SPIFFS.h>
 #include <FS.h>                               // gives file access on spiffs

WebServer server(80);                          // serve web pages on port 80

// Used to disable brownout detection
 #include "soc/soc.h"
 #include "soc/rtc_cntl_reg.h"

// sd-card
 #include "SD_MMC.h"                           // sd card - see https://randomnerdtutorials.com/esp32-cam-take-photo-save-microsd-card/
 #include <SPI.h>
 #include <FS.h>                               // gives file access
 #define SD_CS 5                               // sd chip select pin = 5

// MCP23017 IO expander on pins 12 and 13 (optional)
 #if useMCP23017 == 1
   #include <Wire.h>
   #include "Adafruit_MCP23017.h"
   Adafruit_MCP23017 mcp;
   // Wire.setClock(1700000);                  // set frequency to 1.7mhz
 #endif

// Define some global variables:
 uint32_t lastStatus = millis();               // last time status light changed status (to flash all ok led)
 bool sdcardPresent;                           // flag if an sd card is detected
 int imageCounter;                             // image file name on sd card counter
 const String spiffsFilename = "/image.jpg";   // image name to use when storing in spiffs
 String ImageResDetails = "Unknown";           // image resolution info

// OTA Stuff
  bool OTAEnabled = 0;                         // flag to show if OTA has been enabled (via supply of password in http://x.x.x.x/ota)
  #if ENABLE_OTA
      void sendHeader(WiFiClient &client, char* hTitle);      // forward declarations
      void sendFooter(WiFiClient &client);
      #include "ota.h"                         // Over The Air updates (OTA)
  #endif

Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_LEDS, PIN, NEO_GRB + NEO_KHZ800);
// Only used for inital blackout operation
Adafruit_NeoPixel excess_strip = Adafruit_NeoPixel(300, PIN, NEO_GRB + NEO_KHZ800);


// ---------------------------------------------------------------
//    -SETUP     SETUP     SETUP     SETUP     SETUP     SETUP
// ---------------------------------------------------------------

void setup() {

 if (serialDebug) {
   Serial.begin(serialSpeed);                     // Start serial communication
   // Serial.setDebugOutput(true);

   Serial.println("\n\n\n");                      // line feeds
   Serial.println("-----------------------------------");
   Serial.printf("Starting - %s - %s \n", stitle, sversion);
   Serial.println("-----------------------------------");
   // Serial.print("Reset reason: " + ESP.getResetReason());

  strip.begin();
  excess_strip.begin();
 }

 WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);     // Turn-off the 'brownout detector'

 // small indicator led on rear of esp32cam board
   pinMode(indicatorLED, OUTPUT);
   digitalWrite(indicatorLED,HIGH);

 // Connect to wifi
   digitalWrite(indicatorLED,LOW);               // small indicator led on
   if (serialDebug) {
     Serial.print("\nConnecting to ");
     Serial.print(SSID_NAME);
     Serial.print("\n   ");
   }
   WiFi.begin(SSID_NAME, SSID_PASWORD);

  #ifdef STATIC_IP  
    WiFi.config(ip, gateway, subnet);
  #endif

   
   while (WiFi.status() != WL_CONNECTED) {
       delay(500);
       if (serialDebug) Serial.print(".");
   }
   if (serialDebug) {
     Serial.print("\nWiFi connected, ");
     Serial.print("IP address: ");
     Serial.println(WiFi.localIP());
   }
   server.begin();                               // start web server
   digitalWrite(indicatorLED,HIGH);              // small indicator led off

 // define the web pages (i.e. call these procedures when url is requested)
   server.on("/", handleRoot);                   // root page
   server.on("/data", handleData);               // suplies data to periodically update root (AJAX)
   server.on("/jpg", handleJPG);                 // capture image and send as jpg
   server.on("/jpeg", handleJpeg);                // show updating image
   server.on("/stream", handleStream);           // stream live video
   server.on("/photo", handlePhoto);             // save image to sd card
   server.on("/img", handleImg);                 // show image from sd card
   server.on("/rgb", readRGBImage);              // demo converting image to RGB
   server.on("/graydata", readGrayscaleImage);   // look at grayscale image data
   server.on("/test", handleTest);               // Testing procedure
   server.on("/reboot", handleReboot);           // restart device
   server.onNotFound(handleNotFound);            // invalid url requested
   server.on("/green", handleLEDGreen); //Set all pixels to green
   server.on("/white", handleLEDWhite); //Set all pixels to white
   server.on("/black", handleLEDBlack); //Set all pixels to white
   server.on("/colorAll", handleColorChange); //Set all pixels to a given color
   server.on("/colorRange", handleRangeColorChange); //Set all pixels to a given color
   server.on("/notifyFaceDetected", handleNotifyFaceDetected); //Set screen to white as an override
   server.on("/ping", handlePing); //Return pong for every ping
   server.on("/fire",handleFire); //Fire animation (test)
   server.on("/rainbow",handleRainbow);
   server.on("/wave", handleWave);
   server.on("/heartbeat", handleHeartbeat);
   server.on("/fractal", handleFractal);
   server.on("/animate", startAnimationTask);

#if ENABLE_OTA   
  server.on("/ota", handleOTA);                 // ota updates web page
#endif  

 // NTP - internet time
   if (serialDebug) Serial.println("\nGetting real time (NTP)");
   //configTime(0, 0, ntpServer);
   setenv("TZ", TZ_INFO, 1);
   /*
   if (getNTPtime(10)) {  // wait up to 10 sec to sync
   } else {
     if (serialDebug) Serial.println("Time not set");
   } 
   */
   lastNTPtime = time(&now);


 // set up camera
     if (serialDebug) Serial.print(("\nInitialising camera: "));
     if (HAS_CAMERA && initialiseCamera(1)) {           // apply settings from 'config' and start camera
       if (serialDebug) Serial.println("OK");
     }
     else {
       if (serialDebug) Serial.println("failed");
     }

 // Spiffs - for storing images without an sd card
 //       see: https://circuits4you.com/2018/01/31/example-of-esp8266-flash-file-system-spiffs/
   if (!SPIFFS.begin(true)) {
     if (serialDebug) Serial.println(("An Error has occurred while mounting SPIFFS - restarting"));
     delay(5000);
     ESP.restart();                               // restart and try again
     delay(5000);
   } else {
     // SPIFFS.format();      // wipe spiffs
     delay(5000);
     if (serialDebug) {
       Serial.print(("SPIFFS mounted successfully: "));
       Serial.printf("total bytes: %d , used: %d \n", SPIFFS.totalBytes(), SPIFFS.usedBytes());
     }
   }

 // SD Card - if one is detected set 'sdcardPresent' High
     if (!SD_MMC.begin("/sdcard", true)) {        // if loading sd card fails
       // note: ('/sdcard", true)' = 1bit mode - see: https://dr-mntn.net/2021/02/using-the-sd-card-in-1-bit-mode-on-the-esp32-cam-from-ai-thinker
       if (serialDebug) Serial.println("No SD Card detected");
       sdcardPresent = 0;                        // flag no sd card available
     } else {
       uint8_t cardType = SD_MMC.cardType();
       if (cardType == CARD_NONE) {              // if invalid card found
           if (serialDebug) Serial.println("SD Card type detect failed");
           sdcardPresent = 0;                    // flag no sd card available
       } else {
         // valid sd card detected
         uint16_t SDfreeSpace = (uint64_t)(SD_MMC.totalBytes() - SD_MMC.usedBytes()) / (1024 * 1024);
         if (serialDebug) Serial.printf("SD Card found, free space = %dmB \n", SDfreeSpace);
         sdcardPresent = 1;                      // flag sd card available
       }
     }
     fs::FS &fs = SD_MMC;                        // sd card file system

 // discover the number of image files already stored in '/img' folder of the sd card and set image file counter accordingly
   imageCounter = 0;
   if (sdcardPresent) {
     int tq=fs.mkdir("/img");                    // create the '/img' folder on sd card (in case it is not already there)
     if (!tq) {
       if (serialDebug) Serial.println("Unable to create IMG folder on sd card");
     }

     // open the image folder and step through all files in it
       File root = fs.open("/img");
       while (true)
       {
           File entry =  root.openNextFile();    // open next file in the folder
           if (!entry) break;                    // if no more files in the folder
           imageCounter ++;                      // increment image counter
           entry.close();
       }
       root.close();
       if (serialDebug) Serial.printf("Image file count = %d \n",imageCounter);
   }

 // define i/o pins
   pinMode(indicatorLED, OUTPUT);            // defined again as sd card config can reset it
   digitalWrite(indicatorLED,HIGH);          // led off = High
   pinMode(iopinA, INPUT);                   // pin 13 - free io pin, can be used for input or output
   pinMode(iopinB, OUTPUT);                  // pin 15 - free io pin, can be used for input or output (must not be high at boot)
   pinMode(PIN, OUTPUT);                  // pin 12 - LED Output Pin

 // MCP23017 io expander (requires adafruit MCP23017 library)
 #if useMCP23017 == 1
   Wire.begin(12,13);             // use pins 12 and 13 for i2c
   mcp.begin(&Wire);              // use default address 0
   mcp.pinMode(0, OUTPUT);        // Define GPA0 (physical pin 21) as output pin
   mcp.pinMode(8, INPUT);         // Define GPB0 (physical pin 1) as input pin
   mcp.pullUp(8, HIGH);           // turn on a 100K pullup internally
   // change pin state with   mcp.digitalWrite(0, HIGH);
   // read pin state with     mcp.digitalRead(8)
 #endif

// configure PWM for the illumination LED
  pinMode(brightLED, OUTPUT);
  analogWrite(brightLED, brightLEDbrightness);

// ESP32 Watchdog timer -    Note: esp32 board manager v3.x.x requires different code
  #if defined ESP32
    esp_task_wdt_deinit();                  // ensure a watchdog is not already configured
    #if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR == 3
      // v3 board manager detected
        if (serialDebug) Serial.println("Watchdog timer: v3 esp32 board manager detected");
        esp_task_wdt_config_t wdt_config = {
            .timeout_ms = WDT_TIMEOUT * 1000, // Convert seconds to milliseconds
            .idle_core_mask = 1 << 0,         // Which core to monitor
            .trigger_panic = true             // Enable panic
        };
      // Initialize the WDT with the configuration structure
        esp_task_wdt_init(&wdt_config);       // Pass the pointer to the configuration structure
        esp_task_wdt_add(NULL);               // Add current thread to WDT watch    
        esp_task_wdt_reset();                 // reset timer
        if (serialDebug) Serial.println("Watchdog Timer initialized");
    #else
      // pre v3 board manager assumed
        if (serialDebug) Serial.println("Watchdog timer: Older esp32 board manager detected");
        esp_task_wdt_init(WDT_TIMEOUT, true);                      //enable panic so ESP32 restarts
        esp_task_wdt_add(NULL);                                    //add current thread to WDT watch   
    #endif
  #endif  

 // startup complete
   if (serialDebug) Serial.println("\nStarted...");
   flashLED(2);     // flash the onboard indicator led
   analogWrite(brightLED, 64);    // change bright LED
   delay(200);
   analogWrite(brightLED, 0);    // change bright LED

}  // setup


// ----------------------------------------------------------------
//   -LOOP     LOOP     LOOP     LOOP     LOOP     LOOP     LOOP
// ----------------------------------------------------------------


void loop() {

  if (!HAS_CAMERA){

    int r = 0;
    int g = 0;
    int b = 0;
    int brightness = 0; // 2 to 255 - 1 is not visible
    // Add 300 extra to ensure everything is really off
    //Ensure full blackness, for 300 pixels long, longer than the strip
    for(uint16_t i = 0; i < 300; i++) {
      excess_strip.setPixelColor(i, excess_strip.Color(0, 0, 0)); // Draw new pixel
    }
    excess_strip.show();
    startAnimationTask();
  }

 server.handleClient();          // handle any incoming web page requests

//  //  Capture an image and save to sd card every 5 seconds (i.e. time lapse)
//      static uint32_t lastCamera = millis();
//      if ( ((unsigned long)(millis() - lastCamera) >= 5000) && sdcardPresent ) {
//        lastCamera = millis();     // reset timer
//        storeImage();              // save an image to sd card
//        if (serialDebug) Serial.println("Time lapse image captured");
//      }

 // flash status LED to show sketch is running ok
   if ((unsigned long)(millis() - lastStatus) >= TimeBetweenStatus) {
     lastStatus = millis();                                               // reset timer
     esp_task_wdt_reset();                                                // reset watchdog timer (to prevent system restart)
     digitalWrite(indicatorLED,!digitalRead(indicatorLED));               // flip indicator led status
   }
   
}  // loop


// ----------------------------------------------------------------
//                        Initialise the camera
// ----------------------------------------------------------------
// returns TRUE if successful
// reset - if set to 1 all settings are reconfigured
//         if set to zero you can change the settings and call this procedure to apply them

bool initialiseCamera(bool reset) {

// set the camera parameters in 'config'
if (reset) {
   config.ledc_channel = LEDC_CHANNEL_0;
   config.ledc_timer = LEDC_TIMER_0;
   config.pin_d0 = Y2_GPIO_NUM;
   config.pin_d1 = Y3_GPIO_NUM;
   config.pin_d2 = Y4_GPIO_NUM;
   config.pin_d3 = Y5_GPIO_NUM;
   config.pin_d4 = Y6_GPIO_NUM;
   config.pin_d5 = Y7_GPIO_NUM;
   config.pin_d6 = Y8_GPIO_NUM;
   config.pin_d7 = Y9_GPIO_NUM;
   config.pin_xclk = XCLK_GPIO_NUM;
   config.pin_pclk = PCLK_GPIO_NUM;
   config.pin_vsync = VSYNC_GPIO_NUM;
   config.pin_href = HREF_GPIO_NUM;
   // variations in version of esp32 board manager (v3 changed the names for some reason)
     #if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR == 3  
        config.pin_sccb_sda = SIOD_GPIO_NUM;    // v3.x
        config.pin_sccb_scl = SIOC_GPIO_NUM;     
     #else
        config.pin_sscb_sda = SIOD_GPIO_NUM;    // pre v3
        config.pin_sscb_scl = SIOC_GPIO_NUM;
     #endif
   config.pin_pwdn = PWDN_GPIO_NUM;
   config.pin_reset = RESET_GPIO_NUM;   
   config.pin_pwdn = PWDN_GPIO_NUM;
   config.pin_reset = RESET_GPIO_NUM;
   config.xclk_freq_hz = 10000000;               // XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
   config.pixel_format = PIXFORMAT_JPEG;                         // colour jpg format
   config.frame_size = FRAME_SIZE_IMAGE;         // Image sizes: 160x120 (QQVGA), 128x160 (QQVGA2), 176x144 (QCIF), 240x176 (HQVGA), 320x240 (QVGA),
                                                 //              400x296 (CIF), 640x480 (VGA, default), 800x600 (SVGA), 1024x768 (XGA), 1280x1024 (SXGA),
                                                 //              1600x1200 (UXGA)
   config.jpeg_quality = 10;                     // 0-63 lower number means higher quality (can cause failed image capture if set too low at higher resolutions)
   config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
   //config.fb_location = CAMERA_FB_IN_PSRAM;      // store the captured frame in PSRAM
   config.fb_count = 1;                          // if more than one, i2s runs in continuous mode. Use only with JPEG
}

   // check the esp32cam board has a psram chip installed (extra memory used for storing captured images)
   //    Note: if not using "AI thinker esp32 cam" in the Arduino IDE, PSRAM must be enabled
   if (!psramFound()) {
     if (serialDebug) Serial.println("Warning: No PSRam found so defaulting to image size 'CIF'");
     config.frame_size = FRAMESIZE_VGA;
     config.fb_location = CAMERA_FB_IN_DRAM;
   }

   esp_err_t camerr = esp_camera_init(&config);  // initialise the camera
   if (camerr != ESP_OK) {
     if (serialDebug) Serial.printf("ERROR: Camera init failed with error 0x%x", camerr);
   }

   cameraImageSettings();                        // apply the camera image settings

   return (camerr == ESP_OK);                    // return boolean result of camera initialisation
}


// ----------------------------------------------------------------
//                   -Change camera image settings
// ----------------------------------------------------------------
// Adjust image properties (brightness etc.)
// Defaults to auto adjustments if exposure and gain are both set to zero
// - Returns TRUE if successful
// More info: https://randomnerdtutorials.com/esp32-cam-ov2640-camera-settings/
//            interesting info on exposure times here: https://github.com/raduprv/esp32-cam_ov2640-timelapse

bool cameraImageSettings() {

  if (serialDebug) Serial.println("Applying camera settings");

   sensor_t *s = esp_camera_sensor_get();
   // something to try?:     if (s->id.PID == OV3660_PID)
   if (s == NULL) {
     if (serialDebug) Serial.println("Error: problem reading camera sensor settings");
     return 0;
   }

   // if both set to zero enable auto adjust
   if (cameraImageExposure == 0 && cameraImageGain == 0) {
     // enable auto adjust
       s->set_gain_ctrl(s, 1);                       // auto gain on
       s->set_exposure_ctrl(s, 1);                   // auto exposure on 
       s->set_awb_gain(s, 1);                        // Auto White Balance enable (0 or 1)
       s->set_brightness(s, cameraImageBrightness);  // (-2 to 2) - set brightness
   } else {
     // Apply manual settings
       s->set_gain_ctrl(s, 0);                       // auto gain off
       s->set_awb_gain(s, 1);                        // Auto White Balance enable (0 or 1)
       s->set_exposure_ctrl(s, 0);                   // auto exposure off
       s->set_brightness(s, cameraImageBrightness);  // (-2 to 2) - set brightness
       s->set_agc_gain(s, cameraImageGain);          // set gain manually (0 - 30)
       s->set_aec_value(s, cameraImageExposure);     // set exposure manually  (0-1200)
   }
   
   //s->set_vflip(s, 1);                               // flip image vertically
   //s->set_hmirror(s, 1);                             // flip image horizontally

   return 1;
}  // cameraImageSettings


//    // More camera settings available:
//    // If you enable gain_ctrl or exposure_ctrl it will prevent a lot of the other settings having any effect
//    // more info on settings here: https://randomnerdtutorials.com/esp32-cam-ov2640-camera-settings/
//    s->set_gain_ctrl(s, 0);                       // auto gain off (1 or 0)
//    s->set_exposure_ctrl(s, 0);                   // auto exposure off (1 or 0)
//    s->set_agc_gain(s, 1);                        // set gain manually (0 - 30)
//    s->set_aec_value(s, 1);                       // set exposure manually  (0-1200)
//    s->set_vflip(s, 1);                           // Invert image (0 or 1)
//    s->set_quality(s, 10);                        // (0 - 63)
//    s->set_gainceiling(s, GAINCEILING_32X);       // Image gain (GAINCEILING_x2, x4, x8, x16, x32, x64 or x128)
//    s->set_brightness(s, 0);                      // (-2 to 2) - set brightness
//    s->set_lenc(s, 1);                            // lens correction? (1 or 0)
//    s->set_saturation(s, 0);                      // (-2 to 2)
//    s->set_contrast(s, 0);                        // (-2 to 2)
//    s->set_sharpness(s, 0);                       // (-2 to 2)
//    s->set_hmirror(s, 0);                         // (0 or 1) flip horizontally
//    s->set_colorbar(s, 0);                        // (0 or 1) - show a testcard
//    s->set_special_effect(s, 0);                  // (0 to 6?) apply special effect
//    s->set_whitebal(s, 0);                        // white balance enable (0 or 1)
//    s->set_awb_gain(s, 1);                        // Auto White Balance enable (0 or 1)
//    s->set_wb_mode(s, 0);                         // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
//    s->set_dcw(s, 0);                             // downsize enable? (1 or 0)?
//    s->set_raw_gma(s, 1);                         // (1 or 0)
//    s->set_aec2(s, 0);                            // automatic exposure sensor?  (0 or 1)
//    s->set_ae_level(s, 0);                        // auto exposure levels (-2 to 2)
//    s->set_bpc(s, 0);                             // black pixel correction
//    s->set_wpc(s, 0);                             // white pixel correction


// ----------------------------------------------------------------
//             returns the current time as a String
// ----------------------------------------------------------------
//   see: https://randomnerdtutorials.com/esp32-date-time-ntp-client-server-arduino/
String localTime() {
 struct tm timeinfo;
 char ttime[40];
 if(!getLocalTime(&timeinfo)) return"Failed to obtain time";
 strftime(ttime,40,  "%A %B %d %Y %H:%M:%S", &timeinfo);
 return ttime;
}


// ----------------------------------------------------------------
//        flash the indicator led 'reps' number of times
// ----------------------------------------------------------------
void flashLED(int reps) {
 for(int x=0; x < reps; x++) {
   digitalWrite(indicatorLED,LOW);
   delay(1000);
   digitalWrite(indicatorLED,HIGH);
   delay(500);
 }
}

void sendBasicHeader(WiFiClient &client, char* hTitle) {
  // Start page
      client.write("HTTP/1.1 200 OK\r\n");
      client.write("Content-Type: text/html\r\n");
      client.write("Connection: close\r\n");
      client.write("\r\n");
      client.write("<!DOCTYPE HTML><html lang='en'>\n");
    // HTML / CSS
      client.printf(R"=====(
        <head>
          <meta name='viewport' content='width=device-width, initial-scale=1.0'>
          <title>%s</title>
        </head>
        <body>
        )=====", hTitle);
}

// ----------------------------------------------------------------
//      send standard html header (i.e. start of web page)
// ----------------------------------------------------------------
void sendHeader(WiFiClient &client, char* hTitle) {
    // Start page
      client.write("HTTP/1.1 200 OK\r\n");
      client.write("Content-Type: text/html\r\n");
      client.write("Connection: close\r\n");
      client.write("\r\n");
      client.write("<!DOCTYPE HTML><html lang='en'>\n");
    // HTML / CSS
      client.printf(R"=====(
        <head>
          <meta name='viewport' content='width=device-width, initial-scale=1.0'>
          <title>%s</title>
          <style>
            body {
              color: black;
              background-color: #FFFF00;
              text-align: center;
            }
            input {
              background-color: #FF9900;
              border: 2px #FF9900;
              color: blue;
              padding: 3px 6px;
              text-align: center;
              text-decoration: none;
              display: inline-block;
              font-size: 16px;
              cursor: pointer;
              border-radius: 7px;
            }
            input:hover {
              background-color: #FF4400;
            }
          </style>
        </head>
        <body>
        <h1 style='color:red;'>%s</H1>
      )=====", hTitle, hTitle);
}


// ----------------------------------------------------------------
//      send a standard html footer (i.e. end of web page)
// ----------------------------------------------------------------
void sendFooter(WiFiClient &client) {
  client.write("</body></html>\n");
  delay(3);
  client.stop();
}


// ----------------------------------------------------------------
//  send line of text to both serial port and web page - used by readRGBImage
// ----------------------------------------------------------------
void sendText(WiFiClient &client, String theText) {
     if (!sendRGBfile) client.print(theText + "<br>\n");
     if (serialDebug || theText.indexOf("error") > 0) Serial.println(theText);   // if text contains "error"
}


// ----------------------------------------------------------------
//                        reset the camera
// ----------------------------------------------------------------
// either hardware(1) or software(0)
void resetCamera(bool type = 0) {
  if (type == 1) {
    // power cycle the camera module (handy if camera stops responding)
      digitalWrite(PWDN_GPIO_NUM, HIGH);    // turn power off to camera module
      delay(300);
      digitalWrite(PWDN_GPIO_NUM, LOW);
      delay(300);
      initialiseCamera(1);
    } else {
    // reset via software (handy if you wish to change resolution or image type etc. - see test procedure)
      esp_camera_deinit();
      delay(camChangeDelay);
      initialiseCamera(1);
    }
}


// ----------------------------------------------------------------
//                    -change image resolution
// ----------------------------------------------------------------
// cycles through the available resolutions (set in cyclingRes[])
//Note: there seems to be an issue with 1024x768 with later releases of esp software?
// Resolutions:  160x120 (QQVGA), 128x160 (QQVGA2), 176x144 (QCIF), 240x176 (HQVGA),
//               320x240 (QVGA), 400x296 (CIF), 640x480 (VGA, default), 800x600 (SVGA),
//               1024x768 (XGA), 1280x1024 (SXGA), 1600x1200 (UXGA)
void changeResolution() {
  //  const framesize_t cyclingRes[] = { FRAMESIZE_QVGA, FRAMESIZE_VGA, FRAMESIZE_SVGA, FRAMESIZE_XGA, FRAMESIZE_SXGA };    // resolutions to cycle through
  const int noAvail = sizeof(cyclingRes) / sizeof(cyclingRes[0]);
  static int currentRes = 0;  
  esp_camera_deinit();     // disable camera
  delay(camChangeDelay);
  currentRes++;                               // change to next resolution available
  if (currentRes >= noAvail) currentRes=0;    // reset loop
  FRAME_SIZE_IMAGE = cyclingRes[currentRes];

  initialiseCamera(1);
  if (serialDebug) Serial.println("Camera resolution changed to " + String(cyclingRes[currentRes]));
  ImageResDetails = "Unknown";   // set next time image captured
}


// ----------------------------------------------------------------
//     Capture image from camera and save to spiffs or sd card
// ----------------------------------------------------------------
// returns 0 if failed, 1 if stored in spiffs, 2 if stored on sd card

byte storeImage() {

 byte sRes = 0;                // result flag
 fs::FS &fs = SD_MMC;          // sd card file system

 // capture the image from camera
   int currentBrightness = brightLEDbrightness;
   if (flashRequired) {
      analogWrite(brightLED, 255);   // change LED brightness (0 - 255)
      delay(100);
   }
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  // there is a bug where this buffer can be from previous capture so as workaround it is discarded and captured again
    esp_camera_fb_return(fb); // dispose the buffered image
    fb = NULL; // reset to capture errors
    fb = esp_camera_fb_get(); // get fresh image   
   if (flashRequired){
      delay(100);
      analogWrite(brightLED, currentBrightness);   // change LED brightness back to previous state
   }
   if (!fb) {
     if (serialDebug) Serial.println("Error: Camera capture failed");
     return 0;
   }

 // save image to Spiffs
   if (!sdcardPresent) {
     if (serialDebug) Serial.println("Storing image to spiffs only");
     SPIFFS.remove(spiffsFilename);                         // if file name already exists delete it
     File file = SPIFFS.open(spiffsFilename, FILE_WRITE);   // create new file
     if (!file) {
       if (serialDebug) Serial.println("Failed to create file in Spiffs - will format and try again");
       if (!SPIFFS.format()) {                              // format spiffs
         if (serialDebug) Serial.println("Spiffs format failed");
       } else {
         file = SPIFFS.open(spiffsFilename, FILE_WRITE);    // try again to create new file
         if (!file) {
           if (serialDebug) Serial.println("Still unable to create file in spiffs");
         }
       }
     }
     if (file) {       // if file has been created ok write image data to it
       if (file.write(fb->buf, fb->len)) {
         sRes = 1;    // flag as saved ok
       } else {
         if (serialDebug) Serial.println("Error: failed to write image data to spiffs file");
       }
     }
     esp_camera_fb_return(fb);                               // return camera frame buffer
     if (sRes == 1 && serialDebug) {
       Serial.print("The picture has been saved to Spiffs as " + spiffsFilename);
       Serial.print(" - Size: ");
       Serial.print(file.size());
       Serial.println(" bytes");
     }
     file.close();
   }


 // save the image to sd card
   if (sdcardPresent) {
     if (serialDebug) Serial.printf("Storing image #%d to sd card \n", imageCounter);
     String SDfilename = "/img/" + String(imageCounter + 1) + ".jpg";              // build the image file name
     File file = fs.open(SDfilename, FILE_WRITE);                                  // create file on sd card
     if (!file) {
       if (serialDebug) Serial.println("Error: Failed to create file on sd-card: " + SDfilename);
     } else {
       if (file.write(fb->buf, fb->len)) {                                         // File created ok so save image to it
         if (serialDebug) Serial.println("Image saved to sd card");
         imageCounter ++;                                                          // increment image counter
         sRes = 2;    // flag as saved ok
       } else {
         if (serialDebug) Serial.println("Error: failed to save image data file on sd card");
       }
       file.close();              // close image file on sd card
     }
   }

 esp_camera_fb_return(fb);        // return frame so memory can be released

 return sRes;

} // storeImage


// ----------------------------------------------------------------
//            -Action any user input on root web page
// ----------------------------------------------------------------

void rootUserInput(WiFiClient &client) {

    // if button1 was pressed (toggle io pin A)
    //        Note:  if using an input box etc. you would read the value with the command:    String Bvalue = server.arg("demobutton1");

    // if button1 was pressed (toggle io pin B)
      if (server.hasArg("button1")) {
        if (serialDebug) Serial.println("Button 1 pressed");
        digitalWrite(iopinB,!digitalRead(iopinB));             // toggle output pin on/off
      }

    // if button2 was pressed (Cycle illumination LED)
      if (server.hasArg("button2")) {
        if (serialDebug) Serial.println("Button 2 pressed");
        if (brightLEDbrightness == 0) brightLEDbrightness = 10;                // turn led on dim
        else if (brightLEDbrightness == 10) brightLEDbrightness = 40;          // turn led on medium
        else if (brightLEDbrightness == 40) brightLEDbrightness = 255;         // turn led on full
        else brightLEDbrightness = 0;                                          // turn led off
        analogWrite(brightLED, brightLEDbrightness);
      }

    // if button3 was pressed (toggle flash)
      if (server.hasArg("button3")) {
        if (serialDebug) Serial.println("Button 3 pressed");
        flashRequired = !flashRequired;
      }

    // if button3 was pressed (format SPIFFS)
      if (server.hasArg("button4")) {
        if (serialDebug) Serial.println("Button 4 pressed");
        if (!SPIFFS.format()) {
          if (serialDebug) Serial.println("Error: Unable to format Spiffs");
        } else {
          if (serialDebug) Serial.println("Spiffs memory has been formatted");
        }
      }

    // if button4 was pressed (change resolution)
      if (server.hasArg("button5")) {
        if (serialDebug) Serial.println("Button 5 pressed");
        changeResolution();   // cycle through some options
      }

    // if brightness was adjusted - cameraImageBrightness
        if (server.hasArg("bright")) {
          String Tvalue = server.arg("bright");   // read value
          if (Tvalue != NULL) {
            int val = Tvalue.toInt();
            if (val >= -2 && val <= 2 && val != cameraImageBrightness) {
              if (serialDebug) Serial.printf("Brightness changed to %d\n", val);
              cameraImageBrightness = val;
              cameraImageSettings();           // Apply camera image settings
            }
          }
        }

    // if exposure was adjusted - cameraImageExposure
        if (server.hasArg("exp")) {
          if (serialDebug) Serial.println("Exposure has been changed");
          String Tvalue = server.arg("exp");   // read value
          if (Tvalue != NULL) {
            int val = Tvalue.toInt();
            if (val >= 0 && val <= 1200 && val != cameraImageExposure) {
              if (serialDebug) Serial.printf("Exposure changed to %d\n", val);
              cameraImageExposure = val;
              cameraImageSettings();           // Apply camera image settings
            }
          }
        }

     // if image gain was adjusted - cameraImageGain
        if (server.hasArg("gain")) {
          if (serialDebug) Serial.println("Gain has been changed");
          String Tvalue = server.arg("gain");   // read value
            if (Tvalue != NULL) {
              int val = Tvalue.toInt();
              if (val >= 0 && val <= 31 && val != cameraImageGain) {
                if (serialDebug) Serial.printf("Gain changed to %d\n", val);
                cameraImageGain = val;
                cameraImageSettings();          // Apply camera image settings
              }
            }
         }
  }


// ----------------------------------------------------------------
//       -root web page requested    i.e. http://x.x.x.x/
// ----------------------------------------------------------------
// web page with control buttons, links etc.

void handleRoot() {

 getNTPtime(2);                                             // refresh current time from NTP server
 WiFiClient client = server.client();                       // open link with client

 rootUserInput(client);                                     // Action any user input from this web page

 // html header
   sendHeader(client, stitle);
   client.write("<FORM action='/' method='post'>\n");            // used by the buttons in the html (action = the web page to send it to


 // --------------------------------------------------------------------

 // html main body
 //                    Info on the arduino ethernet library:  https://www.arduino.cc/en/Reference/Ethernet
 //                                            Info in HTML:  https://www.w3schools.com/html/
 //     Info on Javascript (can be inserted in to the HTML):  https://www.w3schools.com/js/default.asp
 //                               Verify your HTML is valid:  https://validator.w3.org/


  // ---------------------------------------------------------------------------------------------
  //  info which is periodically updated using AJAX - https://www.w3schools.com/xml/ajax_intro.asp

    // empty lines which are populated via vbscript with live data from http://x.x.x.x/data in the form of comma separated text
      int noLines = 6;      // number of text lines to be populated by javascript
      for (int i = 0; i < noLines; i++) {
        client.println("<span id='uline" + String(i) + "'></span><br>");
      }

    // Javascript - to periodically update the above info lines from http://x.x.x.x/data
    // Note: You can compact the javascript to save flash memory via https://www.textfixer.com/html/compress-html-compression.php
    //       The below = client.printf(R"=====(  <script> function getData() { var xhttp = new XMLHttpRequest(); xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) { var receivedArr = this.responseText.split(','); for (let i = 0; i < receivedArr.length; i++) { document.getElementById('uline' + i).innerHTML = receivedArr[i]; } } }; xhttp.open('GET', 'data', true); xhttp.send();} getData(); setInterval(function() { getData(); }, %d); </script> )=====", dataRefresh * 1000);

      // get a comma seperated list from http://x.x.x.x/data and populate the blank lines in html above
      client.printf(R"=====(
         <script>
            function getData() {
              var xhttp = new XMLHttpRequest();
              xhttp.onreadystatechange = function() {
              if (this.readyState == 4 && this.status == 200) {
                var receivedArr = this.responseText.split(',');
                for (let i = 0; i < receivedArr.length; i++) {
                  document.getElementById('uline' + i).innerHTML = receivedArr[i];
                }
              }
            };
            xhttp.open('GET', 'data', true);
            xhttp.send();}
            getData();
            setInterval(function() { getData(); }, %d);
         </script>
      )=====", dataRefresh * 1000);


  // ---------------------------------------------------------------------------------------------


//    // touch input on the two gpio pins
//      client.printf("<p>Touch on pin 12: %d </p>\n", touchRead(T5) );
//      client.printf("<p>Touch on pin 13: %d </p>\n", touchRead(T4) );

   // OTA
      if (OTAEnabled) client.write("<br>OTA IS ENABLED!"); 

   // Control buttons
     client.write("<br><br>");
     client.write("<input style='height: 35px;' name='button1' value='Toggle pin 12' type='submit'> \n");
     client.write("<input style='height: 35px;' name='button2' value='Cycle illumination LED' type='submit'> \n");
     client.write("<input style='height: 35px;' name='button3' value='Toggle Flash' type='submit'> \n");
     client.write("<input style='height: 35px;' name='button4' value='Wipe SPIFFS memory' type='submit'> \n");
     client.write("<input style='height: 35px;' name='button5' value='Change Resolution' type='submit'><br> \n");

   // Image setting controls
     client.println("<br>CAMERA SETTINGS: ");
     client.printf("Brightness: <input type='number' style='width: 50px' name='bright' title='from -2 to +2' min='-2' max='2' value='%d'>  \n", cameraImageBrightness);
     client.printf("Exposure: <input type='number' style='width: 50px' name='exp' title='from 0 to 1200' min='0' max='1200' value='%d'>  \n", cameraImageExposure);
     client.printf("Gain: <input type='number' style='width: 50px' name='gain' title='from 0 to 30' min='0' max='30' value='%d'>\n", cameraImageGain);
     client.println(" <input type='submit' name='submit' value='Submit change / Refresh Image'>");
     client.println("<br>Set exposure and gain to zero for auto adjust");

   // links to the other pages available
     client.write("<br><br>LINKS: \n");
     client.write("<a href='/photo'>Store image</a> - \n");
     client.write("<a href='/img'>View stored image</a> - \n");
     client.write("<a href='/rgb'>RGB frame as data</a> - \n");
     client.write("<a href='/graydata'>Grayscale frame as data</a> \n");
     client.write("<br>");
     client.write("<a href='/stream'>Live stream</a> - \n");
     client.write("<a href='/jpg'>JPG</a> - \n");
     client.write("<a href='/jpeg'>Updating JPG</a> - \n");
     client.write("<a href='/test'>Test procedure</a>\n");
     #if ENABLE_OTA
        client.write(" - <a href='/ota'>Update via OTA</a>\n");
     #endif

    // addnl info if sd card present
     if (sdcardPresent) {
       client.write("<br>Note: You can view the individual stored images on sd card with:   http://x.x.x.x/img?img=1");
     }

    // capture and show a jpg image
      client.write("<br><br><a href='/jpg'>");         // make it a link
      client.write("<img id='image1' src='/jpg' width='320' height='240' /> </a>");     // show image from http://x.x.x.x/jpg

    // javascript to refresh the image periodically
      client.printf(R"=====(
         <script>
           function refreshImage(){
               var timestamp = new Date().getTime();
               var el = document.getElementById('image1');
               var queryString = '?t=' + timestamp;
               el.src = '/jpg' + queryString;
           }
           setInterval(function() { refreshImage(); }, %d);
         </script>
      )=====", imagerefresh * 1013);        // 1013 is just to stop it refreshing at the same time as /data

    client.println("<br><br><a href='https://github.com/alanesq/esp32cam-demo'>Sketch Info</a>");


 // --------------------------------------------------------------------


 sendFooter(client);     // close web page

}  // handleRoot


// ----------------------------------------------------------------
//     -data web page requested     i.e. http://x.x.x.x/data
// ----------------------------------------------------------------
// the root web page requests this periodically via Javascript in order to display updating information.
// information in the form of comma seperated text is supplied which are then inserted in to blank lines on the web page
// This makes it very easy to modify the data shown without having to change the javascript or root page html
// Note: to change the number of lines displayed update variable 'noLines' in handleroot()

void handleData(){

  // sd sdcard info
    uint32_t SDusedSpace = 0;
    uint32_t SDtotalSpace = 0;
    uint32_t SDfreeSpace = 0;
    if (sdcardPresent) {
      SDusedSpace = SD_MMC.usedBytes() / (1024 * 1024);
      SDtotalSpace = SD_MMC.totalBytes() / (1024 * 1024);
      SDfreeSpace = SDtotalSpace - SDusedSpace;
    }
   String reply = "";

  // line1 - sd card
    if (!sdcardPresent) {
      reply += "<span style='color:blue;'>NO SD CARD DETECTED</span>";
    } else {
      reply += "SD Card: " + String(SDusedSpace) + "MB used - " + String(SDfreeSpace) + "MB free";
    }
    reply += ",";

  // line2 - illumination/flash led
    reply += "Illumination led brightness=" + String(brightLEDbrightness);
    reply += " &ensp; Flash is ";     // Note: '&ensp;' leaves a gap
    reply += (flashRequired) ? "Enabled" : "Off";
    reply += ",";

  // line3 - Current real time
    reply += "Current time: " + localTime();
    reply += ",";

  // line4 - gpio pin status
    reply += "GPIO output pin 15 is: ";
    reply += (digitalRead(iopinB)==1) ? "ON" : "OFF";
    reply += " &ensp; GPIO input pin 13 is: ";
    reply += (digitalRead(iopinA)==1) ? "ON" : "OFF";
    reply += ",";

  // line5 - image resolution
    reply += "Image size: " + ImageResDetails;
    reply += ",";

  // line6 - free memory
    reply += "Free memory: " + String(ESP.getFreeHeap() /1000) + "K";

   server.send(200, "text/plane", reply); //Send millis value only to client ajax request
}


// ----------------------------------------------------------------
//    -photo save to sd card/spiffs    i.e. http://x.x.x.x/photo
// ----------------------------------------------------------------
// web page to capture an image from camera and save to spiffs or sd card

void handlePhoto() {

 WiFiClient client = server.client();                                                        // open link with client

 // log page request including clients IP
   IPAddress cIP = client.remoteIP();
   if (serialDebug) Serial.println("Save photo requested by " + cIP.toString());

 byte sRes = storeImage();   // capture and save an image to sd card or spiffs (store sucess or failed flag - 0=fail, 1=spiffs only, 2=spiffs and sd card)

 // html header
   sendHeader(client, "Capture and save image");

 // html body
   if (sRes == 2) {
       client.printf("<p>Image saved to sd card as image number %d </p>\n", imageCounter);
   } else if (sRes == 1) {
       client.write("<p>Image saved in Spiffs</p>\n");
   } else {
       client.write("<p>Error: Failed to save image</p>\n");
   }

   client.write("<a href='/'>Return</a>\n");       // link back

 // close web page
   sendFooter(client);

}  // handlePhoto



// ----------------------------------------------------------------
// -display image stored on sd card or SPIFFS   i.e. http://x.x.x.x/img?img=x
// ----------------------------------------------------------------
// Display a previously stored image, default image = most recent
// returns 1 if image displayed ok

bool handleImg() {

   WiFiClient client = server.client();                 // open link with client
   bool pRes = 0;

   // log page request including clients IP
     IPAddress cIP = client.remoteIP();
     if (serialDebug) Serial.println("Display stored image requested by " + cIP.toString());

   int imgToShow = imageCounter;                        // default to showing most recent file

   // get image number from url parameter
     if (server.hasArg("img") && sdcardPresent) {
       String Tvalue = server.arg("img");               // read value
       imgToShow = Tvalue.toInt();                      // convert string to int
       if (imgToShow < 1 || imgToShow > imageCounter) imgToShow = imageCounter;    // validate image number
     }

   // if stored on sd card
   if (sdcardPresent) {
     if (serialDebug) Serial.printf("Displaying image #%d from sd card", imgToShow);

     String tFileName = "/img/" + String(imgToShow) + ".jpg";
     fs::FS &fs = SD_MMC;                                 // sd card file system
     File timg = fs.open(tFileName, "r");
     if (timg) {
         size_t sent = server.streamFile(timg, "image/jpeg");     // send the image
         timg.close();
         pRes = 1;                                                // flag sucess
     } else {
       if (serialDebug) Serial.println("Error: image file not found");
       sendHeader(client, "Display stored image");
       client.write("<p>Error: Image not found</p></html>\n");
       client.write("<br><a href='/'>Return</a>\n");       // link back
       sendFooter(client);     // close web page
     }
   }

   // if stored in SPIFFS
   if (!sdcardPresent) {
     if (serialDebug) Serial.println("Displaying image from spiffs");

     // check file exists
     if (!SPIFFS.exists(spiffsFilename)) {
       sendHeader(client, "Display stored image");
       client.write("Error: No image found to display\n");
       client.write("<br><a href='/'>Return</a>\n");       // link back
       sendFooter(client);     // close web page
       return 0;
     }

     File f = SPIFFS.open(spiffsFilename, "r");                         // read file from spiffs
         if (!f) {
           if (serialDebug) Serial.println("Error reading " + spiffsFilename);
           sendHeader(client, "Display stored image");
           client.write("Error reading file from Spiffs\n");
           client.write("<br><a href='/'>Return</a>\n");       // link back
           sendFooter(client);     // close web page
         }
         else {
             size_t sent = server.streamFile(f, "image/jpeg");     // send file to web page
             if (!sent) {
               if (serialDebug) Serial.println("Error sending " + spiffsFilename);
             } else {
               pRes = 1;                                           // flag sucess
             }
             f.close();
         }
   }
   return pRes;

}  // handleImg


// ----------------------------------------------------------------
//                      -invalid web page requested
// ----------------------------------------------------------------
// Note: shows a different way to send the HTML reply

void handleNotFound() {

 String tReply;

 if (serialDebug) Serial.print("Invalid page requested");

 tReply = "File Not Found\n\n";
 tReply += "URI: ";
 tReply += server.uri();
 tReply += "\nMethod: ";
 tReply += ( server.method() == HTTP_GET ) ? "GET" : "POST";
 tReply += "\nArguments: ";
 tReply += server.args();
 tReply += "\n";

 for ( uint8_t i = 0; i < server.args(); i++ ) {
   tReply += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
 }

 server.send ( 404, "text/plain", tReply );
 tReply = "";      // clear variable

}  // handleNotFound


// ----------------------------------------------------------------
//      -access image data as RGB - i.e. http://x.x.x.x/rgb
// ----------------------------------------------------------------
//Demonstration on how to access raw RGB data from the camera
// Notes:
//     Set sendRGBfile to 1 in the settings at top of sketch to just send the raw rgb data as a file which can then be used with
//       the Processing sketch: https://github.com/alanesq/esp32cam-demo/blob/master/Misc/displayRGB.pde
//       otherwise a web page is displayed showing some sample rgb data usage.
//     You may want to disable auto white balance when experimenting with RGB otherwise the camera is always trying to adjust the
//        image colours to mainly white.   (disable in the 'cameraImageSettings' procedure).
//     It will fail on the higher resolutions as it requires more than the 4mb of available psram to store the data (1600x1200x3 bytes)
//     See this sketch for example of saving and viewing RGB files: https://github.com/alanesq/misc/blob/main/saveAndViewRGBfiles.ino
//     I learned how to read the RGB data from: https://github.com/Makerfabs/Project_Touch-Screen-Camera/blob/master/Camera_v2/Camera_v2.ino

void readRGBImage() {
                                                                                            // used for timing operations
 WiFiClient client = server.client();
 uint32_t tTimer;     // used to time tasks                                                                    // open link with client

 if (!sendRGBfile) {
   // html header
    sendHeader(client, "Show RGB data");

   // page title including clients IP
     IPAddress cIP = client.remoteIP();
     sendText(client, "Live image as rgb data, requested by " + cIP.toString());                                                            // 'sendText' sends the String to both serial port and web page
 }

 // make sure psram is available
 if (!psramFound()) {
   sendText(client,"error: no psram available to store the RGB data");
   client.write("<br><a href='/'>Return</a>\n");       // link back
   if (!sendRGBfile) sendFooter(client);               // close web page
   return;
 }


 //   ****** the main code for converting an image to RGB data *****

   // capture a live image from camera (as a jpg)
      tTimer = millis();                                                                                    // store time that image capture started
      camera_fb_t * fb = NULL;
      fb = esp_camera_fb_get();
      // there is a bug where this buffer can be from previous capture so as workaround it is discarded and captured again
        esp_camera_fb_return(fb); // dispose the buffered image
        fb = NULL; // reset to capture errors
        fb = esp_camera_fb_get(); // get fresh image
     if (!fb) {
       sendText(client,"error: failed to capture image from camera");
       client.write("<br><a href='/'>Return</a>\n");       // link back
       if (!sendRGBfile) sendFooter(client);               // close web page
       return;
     } else {
       sendText(client, "JPG image capture took " + String(millis() - tTimer) + " milliseconds");              // report time it took to capture an image
       sendText(client, "Image resolution=" + String(fb->width) + "x" + String(fb->height));
       sendText(client, "Image size=" + String(fb->len) + " bytes");
       sendText(client, "Image format=" + String(fb->format));
       sendText(client, "Free memory=" + String(ESP.getFreeHeap()) + " bytes");
     }

/*
   // display captured image using base64 - seems a bit unreliable especially with larger images?
    if (!sendRGBfile) {
      client.print("<br>Displaying image direct from frame buffer");
      String base64data = base64::encode(fb->buf, fb->len);      // convert buffer to base64
      client.print(" - Base64 data length = " + String(base64data.length()) + " bytes\n" );
      client.print("<br><img src='data:image/jpg;base64," + base64data + "'></img><br>\n");
    }
*/

   // allocate memory to store the rgb data (in psram, 3 bytes per pixel)
     sendText(client,"<br>Free psram before rgb data allocated = " + String(heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / 1024) + "K");
     void *ptrVal = NULL;                                                                                 // create a pointer for memory location to store the data
     uint32_t ARRAY_LENGTH = fb->width * fb->height * 3;                                                  // calculate memory required to store the RGB data (i.e. number of pixels in the jpg image x 3)
     if (heap_caps_get_free_size( MALLOC_CAP_SPIRAM) <  ARRAY_LENGTH) {
       sendText(client,"error: not enough free psram to store the rgb data");
       if (!sendRGBfile) {
         client.write("<br><a href='/'>Return</a>\n");    // link back
         sendFooter(client);                              // close web page
       }
       return;
     }
     ptrVal = heap_caps_malloc(ARRAY_LENGTH, MALLOC_CAP_SPIRAM);                                          // allocate memory space for the rgb data
     uint8_t *rgb = (uint8_t *)ptrVal;                                                                    // create the 'rgb' array pointer to the allocated memory space
     sendText(client,"Free psram after rgb data allocated = " + String(heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / 1024) + "K");

   // convert the captured jpg image (fb) to rgb data (store in 'rgb' array)
     tTimer = millis();                                                                                   // store time that image conversion process started
     bool jpeg_converted = fmt2rgb888(fb->buf, fb->len, PIXFORMAT_JPEG, rgb);
     if (!jpeg_converted) {
       sendText(client,"error: failed to convert image to RGB data");
       if (!sendRGBfile) {
         client.write("<br><a href='/'>Return</a>\n");    // link back
         sendFooter(client);                              // close web page
       }
       return;
     }
     sendText(client, "Conversion from jpg to RGB took " + String(millis() - tTimer) + " milliseconds");// report how long the conversion took


   // if sendRGBfile is set then just send raw RGB data and close
   if (sendRGBfile) {
     client.write(rgb, ARRAY_LENGTH);          // send the raw rgb data
     esp_camera_fb_return(fb);                 // camera frame buffer
     delay(3);
     client.stop();
     return;
   }

 //   ****** examples of using the resulting RGB data *****

   // display some of the resulting data
       uint32_t resultsToShow = 50;                                                                       // how much data to display
       sendText(client,"<br>R,G,B data for first " + String(resultsToShow / 3) + " pixels of image");
       for (uint32_t i = 0; i < resultsToShow-2; i+=3) {
         sendText(client,String(rgb[i+2]) + "," + String(rgb[i+1]) + "," + String(rgb[i+0]));           // Red , Green , Blue
         // // calculate the x and y coordinate of the current pixel
         //   uint16_t x = (i / 3) % fb->width;
         //   uint16_t y = floor( (i / 3) / fb->width);
       }

   // find the average values for each colour over entire image
       uint32_t aRed = 0;
       uint32_t aGreen = 0;
       uint32_t aBlue = 0;
       for (uint32_t i = 0; i < (ARRAY_LENGTH - 2); i+=3) {                                               // go through all data and add up totals
         aBlue+=rgb[i];
         aGreen+=rgb[i+1];
         aRed+=rgb[i+2];
       }
       aRed = aRed / (fb->width * fb->height);                                                            // divide total by number of pixels to give the average value
       aGreen = aGreen / (fb->width * fb->height);
       aBlue = aBlue / (fb->width * fb->height);
       sendText(client,"Average Blue = " + String(aBlue));
       sendText(client,"Average Green = " + String(aGreen));
       sendText(client,"Average Red = " + String(aRed));
       sendText(client,"Image luminance = " + String( aRed * 0.3 + aGreen * 0.59 + aBlue * 0.11 ) );


 //   *******************************************************

 client.write("<br><a href='/'>Return</a>\n");       // link back
 sendFooter(client);     // close web page

 // finished with the data so free up the memory space used in psram
   esp_camera_fb_return(fb);   // camera frame buffer
   heap_caps_free(ptrVal);     // rgb data

}  // readRGBImage


// ----------------------------------------------------------------
//                      -get time from ntp server
// ----------------------------------------------------------------

bool getNTPtime(int sec) {

   uint32_t start = millis();      // timeout timer

   do {
     time(&now);
     localtime_r(&now, &timeinfo);
     if (serialDebug) Serial.print(".");
     delay(100);
   } while (((millis() - start) <= (1000 * sec)) && (timeinfo.tm_year < (2016 - 1900)));

   if (timeinfo.tm_year <= (2016 - 1900)) return false;  // the NTP call was not successful
   if (serialDebug) {
     Serial.print("now ");
     Serial.println(now);
   }

   // Display time
   if (serialDebug)  {
     char time_output[30];
     strftime(time_output, 30, "%a  %d-%m-%y %T", localtime(&now));
     Serial.println(time_output);
     Serial.println();
   }
 return true;
}


// ----------------------------------------------------------------
//     -capture jpg image and send    i.e. http://x.x.x.x/jpg
// ----------------------------------------------------------------

bool handleJPG() {

    WiFiClient client = server.client();          // open link with client
    char buf[32];

    // capture the jpg image from camera
        camera_fb_t * fb = NULL;
        fb = esp_camera_fb_get();
        // there is a bug where this buffer can be from previous capture so as workaround it is discarded and captured again
          esp_camera_fb_return(fb); // dispose the buffered image
          fb = NULL; // reset to capture errors
          fb = esp_camera_fb_get(); // get fresh image
        if (!fb) {
          if (serialDebug) Serial.println("Error: failed to capture image");
          return 0;
        }

    // store image resolution info.
      ImageResDetails = String(fb->width) + "x" + String(fb->height);

    // html to send a jpg
      const char HEADER[] = "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\n";
      const char CTNTTYPE[] = "Content-Type: image/jpeg\r\nContent-Length: ";
      const int hdrLen = strlen(HEADER);
      const int cntLen = strlen(CTNTTYPE);
      client.write(HEADER, hdrLen);
      client.write(CTNTTYPE, cntLen);
      sprintf( buf, "%d\r\n\r\n", fb->len);      // put text size in to 'buf' char array and send
      client.write(buf, strlen(buf));

    // send the captured jpg data
      client.write((char *)fb->buf, fb->len);

    // close network client connection
      delay(3);
      client.stop();

    esp_camera_fb_return(fb);                 // return camera frame buffer

    return 1;

}  // handleJPG



// ----------------------------------------------------------------
//      -stream requested     i.e. http://x.x.x.x/stream
// ----------------------------------------------------------------
// Sends video stream - thanks to Uwe Gerlach for the code showing me how to do this

void handleStream(){

  WiFiClient client = server.client();          // open link with client
  char buf[32];
  camera_fb_t * fb = NULL;

  // log page request including clients IP
    IPAddress cIP = client.remoteIP();
    if (serialDebug) Serial.println("Live stream requested by " + cIP.toString());

 // html
 const char HEADER[] = "HTTP/1.1 200 OK\r\n" \
                       "Access-Control-Allow-Origin: *\r\n" \
                       "Content-Type: multipart/x-mixed-replace; boundary=123456789000000000000987654321\r\n";
 const char BOUNDARY[] = "\r\n--123456789000000000000987654321\r\n";           // marks end of each image frame
 const char CTNTTYPE[] = "Content-Type: image/jpeg\r\nContent-Length: ";       // marks start of image data
 const int hdrLen = strlen(HEADER);         // length of the stored text, used when sending to web page
 const int bdrLen = strlen(BOUNDARY);
 const int cntLen = strlen(CTNTTYPE);
 client.write(HEADER, hdrLen);
 client.write(BOUNDARY, bdrLen);

 // send live jpg images until client disconnects
 while (true)
 {
   if (!client.connected()) break;
     fb = esp_camera_fb_get();                   // capture live image as jpg
     if (!fb) {
       if (serialDebug) Serial.println("Error: failed to capture jpg image");
     } else {
      // send image
       client.write(CTNTTYPE, cntLen);             // send content type html (i.e. jpg image)
       sprintf( buf, "%d\r\n\r\n", fb->len);       // format the image's size as html and put in to 'buf'
       client.write(buf, strlen(buf));             // send result (image size)
       client.write((char *)fb->buf, fb->len);     // send the image data
       client.write(BOUNDARY, bdrLen);             // send html boundary      see https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Content-Type
       esp_camera_fb_return(fb);                  // return camera frame buffer
   }
 }

 if (serialDebug) Serial.println("Video stream stopped");
 delay(3);
 client.stop();


}  // handleStream


// ----------------------------------------------------------------
//                        request a web page
// ----------------------------------------------------------------
//   @param    page         web page to request
//   @param    received     String to store response in
//   @param    maxWaitTime  maximum time to wait for reply (ms)
//   @returns  http code
// see:  https://randomnerdtutorials.com/esp32-http-get-post-arduino/#http-get-1
// to do:  limit size of reply

int requestWebPage(String* page, String* received, int maxWaitTime=5000){

  if (serialDebug) Serial.println("requesting web page: " + *page);

  WiFiClient client;
  HTTPClient http;     // see:  https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266HTTPClient
  http.setTimeout(maxWaitTime);
  http.begin(client, *page);      // for https requires (client, *page, thumbprint)  e.g.String thumbprint="08:3B:71:72:02:43:6E:CA:ED:42:86:93:BA:7E:DF:81:C4:BC:62:30";
  int httpCode = http.GET();      // http codes: https://en.wikipedia.org/wiki/List_of_HTTP_status_codes
  if (serialDebug) Serial.println("http code: " + String(httpCode));

  if (httpCode > 0) {
    *received = http.getString();
  } else {
    *received = "error:" + String(httpCode);
  }
  if (serialDebug) Serial.println(*received);

  http.end();   //Close connection
  if (serialDebug) Serial.println("Web connection closed");

  return httpCode;

}  // requestWebPage


// ----------------------------------------------------------------
//       -show refreshing image    i.e. http://x.x.x.x/jpeg
// ----------------------------------------------------------------

void handleJpeg() {

  const int refreshRate = 2000;     // image refresh rate (ms)

  WiFiClient client = server.client();                 // open link with client

    // Start page
      client.write("HTTP/1.1 200 OK\r\n");
      client.write("Content-Type: text/html\r\n");
      client.write("Connection: close\r\n");
      client.write("\r\n");
      client.write("<!DOCTYPE HTML><html lang='en'>\n");
      client.write("<head></head><body>");

    client.write("<FORM action='/' method='post'>\n");            // used by the buttons in the html (action = the web page to send it to

    // capture and show a jpg image
      client.write("<img id='image1' src='/jpg'/>");     // show image from http://x.x.x.x/jpg

    // javascript to refresh the image periodically
      client.printf(R"=====(
         <script>
           function refreshImage(){
               var timestamp = new Date().getTime();
               var el = document.getElementById('image1');
               var queryString = '?t=' + timestamp;
               el.src = '/jpg' + queryString;
           }
           setInterval(function() { refreshImage(); }, %d);
         </script>
      )=====", refreshRate);        

  sendFooter(client);     // close web page
}  // handleJpeg


// ----------------------------------------------------------------
//                     resize grayscale image
// ----------------------------------------------------------------
// Thanks to Bard A.I. for writing this for me ;-)
//   src_buf: The source image buffer.
//   src_width: The width of the source image buffer.
//   src_height: The height of the source image buffer.
//   dst_buf: The destination image buffer.
//   dst_width: The width of the destination image buffer.
//   dst_height: The height of the destination image buffer.
void resize_esp32cam_image_buffer(uint8_t* src_buf, int src_width, int src_height,
                                   uint8_t* dst_buf, int dst_width, int dst_height) {
  // Calculate the horizontal and vertical resize ratios.
  float h_ratio = (float)src_width / dst_width;
  float v_ratio = (float)src_height / dst_height;

  // Iterate over the destination image buffer and write the resized pixels.
  for (int y = 0; y < dst_height; y++) {
    for (int x = 0; x < dst_width; x++) {
      // Calculate the source pixel coordinates.
      int src_x = (int)(x * h_ratio);
      int src_y = (int)(y * v_ratio);

      // Read the source pixel value.
      uint8_t src_pixel = src_buf[src_y * src_width + src_x];

      // Write the resized pixel value to the destination image buffer.
      dst_buf[y * dst_width + x] = src_pixel;
    }
  }
}


// ----------------------------------------------------------------
//                  Capture grayscale image data
// ----------------------------------------------------------------

void readGrayscaleImage() {

  WiFiClient client = server.client();                 // open link with client

  // html header
   sendHeader(client, "Access grayscale image data");  

  // change camera to grayscale mode  (by default it is in JPG colour mode)
    esp_camera_deinit();                           // disable camera
    delay(camChangeDelay);
    config.pixel_format = PIXFORMAT_GRAYSCALE;     // change camera setting to grayscale (default is JPG)
    initialiseCamera(0);                           // restart the camera (0 = without resetting all the other camera settings)
    cameraImageSettings();

  // capture the image and use flash if required
    int currentBrightness = brightLEDbrightness;
    if (flashRequired) {
        analogWrite(brightLED, 255);   // change LED brightness (0 - 255)
        delay(100);
    }
    camera_fb_t * fb = NULL;
    fb = esp_camera_fb_get();
    // there is a bug where this buffer can be from previous capture so as workaround it is discarded and captured again
      esp_camera_fb_return(fb); // dispose the buffered image
      fb = NULL;                // reset to capture errors
      fb = esp_camera_fb_get(); // get fresh image
    if (flashRequired){
        delay(100);
        analogWrite(brightLED, currentBrightness);            // change LED brightness back to previous state
    }
    if (!fb) client.println("Error: Camera image capture failed");

  // read image data and calculate average pixel value (as demonstration of reading the image data)
  //      note:   image x = i % WIDTH, image y = floor(i / WIDTH)
    unsigned long dataSize = fb->width * fb->height;
    byte minV=255; byte maxV=0;
    for (int y=0; y < fb->height; y++) {
      for (int x=0; x < fb->width; x++) {   
        byte pixelVal = fb->buf[(y * fb->width) + x];
        if (pixelVal > maxV) maxV = pixelVal;
        if (pixelVal < minV) minV = pixelVal;
      }
    }
    client.println("grayscale Image: The lowest value pixel is " + String(minV) + ", the highest is " + String(maxV));
    client.write("<br><br><a href='/'>Return</a>\n");       // link back    

  // resize the image
    int newWidth = 115;   int newHeight = 42;         // much bigger than this seems to cause problems, possible web page is too large?
    byte newBuf[newWidth * newHeight];
    resize_esp32cam_image_buffer(fb->buf, fb->width, fb->height, newBuf, newWidth, newHeight);

  // display image as asciiArt
    char asciiArt[] = {'@','#','S','%','?','*','+',';',':',',','.',' ',' '};       // characters to use 
    int noAsciiChars = sizeof(asciiArt) / sizeof(asciiArt[0]);                 // number of characters available
    client.write("<br><pre style='line-height: 1.1;'>");                       // 'pre' stops variable character spacing, 'line-heigh' adjusts spacing between lines - 0.2
    for (int y=0; y < newHeight; y++) {
      client.write("\n");                                                      // new line
      for (int x=0; x < newWidth; x++) {   
        int tpos = map(newBuf[y*newWidth+x], minV, maxV, 0, noAsciiChars - 1); // convert pixel brightness to ascii character
        client.write(asciiArt[tpos]);   
      }
    }
    client.write("</pre><br>");

  // close web page
    sendFooter(client);    
    
  // close network client connection
    delay(3);
    client.stop();
    
  // return image frame to free up memory
    esp_camera_fb_return(fb);                 // return camera frame buffer

  // change camera back to JPG mode
    esp_camera_deinit();  
    delay(camChangeDelay);
    initialiseCamera(1);    // reset settings (1=apply the cameras settings which includes JPG mode)
  }


// ----------------------------------------------------------------
//   -reboot web page requested        i.e. http://x.x.x.x/reboot
// ----------------------------------------------------------------
// note: this can fail if the esp has just been reflashed and not restarted

void handleReboot(){

      String message = "Rebooting....";
      server.send(200, "text/plain", message);   // send reply as plain text

      // rebooting
        delay(500);          // give time to send the above html
        ESP.restart();
        delay(5000);         // restart fails without this delay
}


// ----------------------------------------------------------------
//       -test procedure    i.e. http://x.x.x.x/test
// ----------------------------------------------------------------

void handleTest() {

 WiFiClient client = server.client();                                                        // open link with client

 // log page request including clients IP
   IPAddress cIP = client.remoteIP();
   if (serialDebug) Serial.println("Test page requested by " + cIP.toString());

 // html header
   sendHeader(client, "Testing");


 // html body


 // -------------------------------------------------------------------




                          // test code goes here




// demo of drawing on the camera image using javascript / html canvas
//   could be of use to show area of interest on the image etc. - see https://www.w3schools.com/html/html5_canvas.asp
// creat a DIV and put image in it with a html canvas on top of it
  int imageWidth = 640;   // image dimensions on web page
  int imageHeight = 480;
  client.println("<div style='display:inline-block;position:relative;'>");
  client.println("<img style='position:absolute;z-index:10;' src='/jpg' width='" + String(imageWidth) + "' height='" + String(imageHeight) + "' />");
  client.println("<canvas style='position:relative;z-index:20;' id='myCanvas' width='" + String(imageWidth) + "' height='" + String(imageHeight) + "'></canvas>");
  client.println("</div>");
// javascript to draw on the canvas
  client.println("<script>");
  client.println("var imageWidth = " + String(imageWidth) + ";");
  client.println("var imageHeight = " + String(imageHeight) + ";");
  client.print (R"=====(
    // connect to the canvas
      var c = document.getElementById("myCanvas");
      var ctx = c.getContext("2d");
      ctx.strokeStyle = "red";
    // draw on image
      ctx.rect(imageWidth / 2, imageHeight / 2, 60, 40);                              // box
      ctx.moveTo(20, 20); ctx.lineTo(200, 100);                                       // line
      ctx.font = "30px Arial";  ctx.fillText("Hello World", 50, imageHeight - 50);    // text
      ctx.stroke();
   </script>\n)=====");


// // flip image horizontally
//   sensor_t *s = esp_camera_sensor_get();
//   s->set_hmirror(s, 1);


/*
 // demo of how to request a web page
   String page = "http://urlhere.com";   // url to request
   String response;                             // reply will be stored here
   int httpCode = requestWebPage(&page, &response);
   // show results
     client.println("Web page requested: '" + page + "' - http code: " + String(httpCode));
     client.print("<xmp>'");     // enables the html code to be displayed
     client.print(response);
     client.println("'</xmp><br>");
*/


/*
//  // demo useage of the mcp23017 io chipnote: this stops PWM on the flash working for some reason
    #if useMCP23017 == 1
      while(1) {
          mcp.digitalWrite(0, HIGH);
          int q = mcp.digitalRead(8);
          client.print("<p>HIGH, input =" + String(q) + "</p>");
          delay(1000);
          mcp.digitalWrite(0, LOW);
          client.print("<p>LOW</p>");
          delay(1000);
      }
    #endif
*/


 // -------------------------------------------------------------------

 client.println("<br><br><a href='/'>Return</a>");       // link back
 sendFooter(client);     // close web page

}  // handleTest


// LED STRIP FUNCTIONS

void writeColor(int r, int g, int b, int brightness, int startLED, int stopLED) {
  if (brightness > MAX_BRIGHTNESS) {
    brightness = MAX_BRIGHTNESS;
  }

  // Apply brightness to RGB values directly
  r = (r * brightness) >> 8;
  g = (g * brightness) >> 8;
  b = (b * brightness) >> 8;
  for(uint16_t i = startLED; i < stopLED; i++) {
    strip.setPixelColor(i, strip.Color(r, g, b)); // Draw new pixel
  }
  strip.show();
}

void writePixelNoShow(int n, int r, int g, int b, int brightness) {
  brightness = (brightness > MAX_BRIGHTNESS) ? MAX_BRIGHTNESS : brightness;

  // Apply brightness to RGB values directly using a more efficient bit shift
  r = (r * brightness) >> 8;
  g = (g * brightness) >> 8;
  b = (b * brightness) >> 8;

  strip.setPixelColor(n, r, g, b); // Draw new pixel
}


// ----------------------------------------------------------------
//     -set LEDs to a green    i.e. http://x.x.x.x/green
// ----------------------------------------------------------------

bool handleLEDGreen() {

  int r = 0;
  int g = 255;
  int b = 0;
  int brightness = 2; // 2 to 255 - 1 is not visible
  writeColor(r,g,b,brightness, 0, strip.numPixels()-1);

  WiFiClient client = server.client();          // open link with client

  sendBasicHeader(client, "Green LED");

  client.printf(R"=====(
    <h1>
      Green LEDs Written
    </h1>
    )=====");
  
  sendFooter(client);
  return 1;

}  // handleLEDGreen

// ----------------------------------------------------------------
//     -set LEDs to a white    i.e. http://x.x.x.x/white
// ----------------------------------------------------------------

bool handleLEDWhite() {

  int r = 255;
  int g = 255;
  int b = 255;
  int brightness = 255; // 2 to 255 - 1 is not visible
  writeColor(r,g,b,brightness, 0, strip.numPixels());

  WiFiClient client = server.client();          // open link with client

  sendBasicHeader(client, "White LED");

  client.printf(R"=====(
    <h1>
      White LEDs Written
    </h1>
    )=====");
  
  sendFooter(client);
  return 1;

}  // handleLEDWhite

// ----------------------------------------------------------------
//     -set LEDs to a black    i.e. http://x.x.x.x/black
// ----------------------------------------------------------------

bool handleLEDBlack() {

  int r = 0;
  int g = 0;
  int b = 0;
  int brightness = 0; // 2 to 255 - 1 is not visible
  // Add 300 extra to ensure everything is really off
  //Ensure full blackness, for 300 pixels long, longer than the strip
  for(uint16_t i = 0; i < 300; i++) {
    excess_strip.setPixelColor(i, excess_strip.Color(0, 0, 0)); // Draw new pixel
  }
  excess_strip.show();
  
  WiFiClient client = server.client();          // open link with client

  sendBasicHeader(client, "Black LED");

  client.printf(R"=====(
    <h1>
      Black LEDs Written
    </h1>
    )=====");
  
  sendFooter(client);
  return 1;

}  // handleLEDWhite



// ----------------------------------------------------------------
//     -set LEDs to a green    i.e. http://x.x.x.x/colorAll?r=255&g=255&b=255&brightness=127
// ----------------------------------------------------------------

bool handleColorChange(){
  // Parse the input values from the client
  WiFiClient client = server.client();
  if (!client) return 0;

  Serial.println("receiving from remote server");
  Serial.println(server.arg("r"));
  Serial.println(server.arg("g"));
  Serial.println(server.arg("b"));
  Serial.println(server.arg("brightness"));


  int r = server.arg("r").toInt();
  int g = server.arg("g").toInt();
  int b = server.arg("b").toInt();
  int brightness = server.arg("brightness").toInt();

  writeColor(r, g, b, brightness, 0, strip.numPixels());
  
  sendBasicHeader(client, "LED Control");
  client.printf(R"=====(<h1>LEDs Color Set</h1><p>R=%d G=%d B=%d Brightness=%d</p>)=====", r, g, b, brightness);
  sendFooter(client);
  return 1;
}  // handleColorChange


// ----------------------------------------------------------------
//     -set LEDs to a given color, in a given range of LED indicies    i.e. http://x.x.x.x/colorRange?i1r=255&i1g=255&i1b=255&i1brightness=127&i2r=255&i2g=255&i2b=255&i2brightness=127
//      Each i-th index in the URL is an LED index with an r,g,b, and brightness value required
// ----------------------------------------------------------------

bool handleRangeColorChange() {
    // Parse the input values from the client
    WiFiClient client = server.client();
    if (!client) return 0;

    int declaredPixels = 0;
    String arg_prefix, r_str, g_str, b_str, brightness_str;
    int r, g, b, brightness;

    for (int i = 0; i < N_LEDS; i++) {

        writePixelNoShow(i, 0, 0, 0, 0);
        arg_prefix = "i" + String(i);
        
        r_str = server.arg(arg_prefix + "r");
        g_str = server.arg(arg_prefix + "g");
        b_str = server.arg(arg_prefix + "b");

        brightness_str = server.arg(arg_prefix + "brightness");
        
        if (!r_str.isEmpty() && !g_str.isEmpty() && !b_str.isEmpty() && !brightness_str.isEmpty()) {
            r = r_str.toInt();
            g = g_str.toInt();
            b = b_str.toInt();
            brightness = brightness_str.toInt();
            //writeColor(r, g, b, brightness, i, i+1);
            writePixelNoShow(i, r, g, b, brightness);
            declaredPixels++;
        }
    }
    // Need to avoid flickers. Delay should be proportional to number of pixels.
    delay(30);

    strip.show();

    
  

    sendBasicHeader(client, "Range Color Change");
    client.printf(R"=====(<h1>Range Color Set</h1><p>Updated LED colors successfully.</p><p>Number of LEDs Updated: %d</p>)=====", declaredPixels);
    sendFooter(client);

    return 1;
}


// ----------------------------------------------------------------
//     -set LEDs to a white, in a given range of LED indicies    i.e. http://x.x.x.x/notifyFaceDetected
// ----------------------------------------------------------------

bool handleNotifyFaceDetected() {


 // Parse the input values from the client
    WiFiClient client = server.client();
    if (!client) return 0;

    for (int i = 0; i < N_LEDS; i++) {

        writePixelNoShow(i, 255, 255, 255, 127);
    }
    // Need to avoid flickers. Delay should be proportional to number of pixels.
    delay(30);

    strip.show();

    sendBasicHeader(client, "Face detected");
    client.printf(R"=====(<h1>Face Detected<h1>)=====");
    sendFooter(client);

    delay(5000);


    return 1;
}

bool handlePing() {
    if (serialDebug) Serial.println("In handle ping.");

 // Parse the input values from the client
    WiFiClient client = server.client();
    if (!client) return 0;

    sendBasicHeader(client, "Face detected");
    client.printf(R"=====(<h1>Pong<h1>)=====");
    sendFooter(client);

    if (serialDebug) Serial.println("Leaving handle ping.");

    return 1;
}

void hsvToRgb(int h, int s, int v, int* r, int* g, int* b) {
  float hh = h / 256.0;
  int i = int(hh * 6);
  float ff = hh * 6 - i;
  float p = v * (1 - s / 255.0);
  float q = v * (1 - ff * s / 255.0);
  float t = v * (1 - (1 - ff) * s / 255.0);
  
  switch(i % 6) {
    case 0: *r = v, *g = t, *b = p; break;
    case 1: *r = q, *g = v, *b = p; break;
    case 2: *r = p, *g = v, *b = t; break;
    case 3: *r = p, *g = q, *b = v; break;
    case 4: *r = t, *g = p, *b = v; break;
    case 5: *r = v, *g = p, *b = q; break;
  }

}

bool handleRainbow() {
  for(int j = 0; j < 256; j++) {
    for(int i = 0; i < N_LEDS; i++) {
      int pixelHue = (i * 256 / N_LEDS) + j;
      int pixelIndex = (pixelHue >> 8) & 0xFF;
      int r, g, b;
      hsvToRgb(pixelIndex, 255, 255, &r, &g, &b);
      writePixelNoShow(i, r, g, b, 255);
    }
    strip.show();
    delay(20);
  }
}

void handleRainbowCalled(int baseR, int baseG, int baseB) {
  for (int j = 0; j < 256; j++) {
    for (int i = 0; i < N_LEDS; i++) {
      int pixelHue = (i * 256 / N_LEDS) + j;
      int pixelIndex = (pixelHue >> 8) & 0xFF;
      int r, g, b;
      hsvToRgb(pixelIndex, 255, 255, &r, &g, &b);

      int adjR = (r * baseR) / 255;
      int adjG = (g * baseG) / 255;
      int adjB = (b * baseB) / 255;

      writePixelNoShow(i, adjR, adjG, adjB, 255);
    }
    strip.show();
    delay(20);  // Reduced delay for faster updates
  }
}

void handleWaveCalled(int r, int g, int b) {
  float frequency = 1.0;  // Increase frequency for faster wave
  long startTime = millis();
  while (millis() - startTime < 15000) {  // Run for 15 seconds
    for (int j = 0; j < 256; j++) {
      for (int i = 0; i < N_LEDS; i++) {
        int brightness = int((sin(i * frequency + j) + 1) * 127.5);
        int adjR = (r * brightness) / 255;
        int adjG = (g * brightness) / 255;
        int adjB = (b * brightness) / 255;
        writePixelNoShow(i, adjR, adjG, adjB, 255);  // Use writePixelNoShow for color manipulation
      }
      strip.show();
      delay(20);  // Reduce delay for faster updates
    }
  }
}




void handleWave() {
  float frequency = 1.0;  // Increase frequency for faster wave
  long startTime = millis();
  while (millis() - startTime < 15000) {  // Run for 15 seconds
    for (int j = 0; j < 256; j++) {
      for (int i = 0; i < N_LEDS; i++) {
        int brightness = int((sin(i * frequency + j) + 1) * 127.5);
        writePixelNoShow(i, 0, 0, brightness, 255);
      }
      strip.show();
      delay(20);  // Reduce delay for faster updates
    }
  }
}


void handleFire() {
  int heat[N_LEDS];
  int cooling = 55;
  int sparking = 120;
  long startTime = millis();

  while (millis() - startTime < 15000) {  // Run for 15 seconds
    // Step 1: Cool down every cell
    for (int i = 0; i < N_LEDS; i++) {
      heat[i] = (heat[i] - random(0, ((cooling * 10) / N_LEDS) + 2));
      if (heat[i] < 0) heat[i] = 0;  // Ensure heat doesn't go below 0
    }

    // Step 2: Heat from each cell drifts up and diffuses
    for (int i = N_LEDS - 1; i >= 2; i--) {
      heat[i] = (heat[i - 1] + heat[i - 2] + heat[i - 2]) / 3;
    }

    // Step 3: Randomly ignite new sparks near the bottom
    if (random(255) < sparking) {
      int y = random(7);
      heat[y] = heat[y] + random(160, 255);
    }

    // Step 4: Convert heat to LED colors with random flame colors
    for (int i = 0; i < N_LEDS; i++) {
      setPixelRandomColor(i, heat[i]);
    }

    strip.show();
    delay(50);
  }
}

void handleFireCalled(int r, int g, int b) {
  int heat[N_LEDS];
  int cooling = 55;
  int sparking = 120;
  long startTime = millis();

  while (millis() - startTime < 15000) {  // Run for 15 seconds
    // Step 1: Cool down every cell
    for (int i = 0; i < N_LEDS; i++) {
      heat[i] = (heat[i] - random(0, ((cooling * 10) / N_LEDS) + 2));
      if (heat[i] < 0) heat[i] = 0;  // Ensure heat doesn't go below 0
    }

    // Step 2: Heat from each cell drifts up and diffuses
    for (int i = N_LEDS - 1; i >= 2; i--) {
      heat[i] = (heat[i - 1] + heat[i - 2] + heat[i - 2]) / 3;
    }

    // Step 3: Randomly ignite new sparks near the bottom
    if (random(255) < sparking) {
      int y = random(7);
      heat[y] = heat[y] + random(160, 255);
    }

    // Step 4: Convert heat to LED colors with input flame colors
    for (int i = 0; i < N_LEDS; i++) {
      setPixelHeatColor(i, heat[i], r, g, b);
    }

    strip.show();
    delay(50);
  }
}

void setPixelHeatColor(int pixel, int temperature, int r, int g, int b) {
  int t192 = round((temperature / 255.0) * 191);

  byte heatramp = t192 & 0x3F;  // 0..63
  heatramp <<= 2;  // Scale up to 0..252

  if (t192 > 128) {
    writePixelNoShow(pixel, r, g, heatramp, 255);
  } else if (t192 > 64) {
    writePixelNoShow(pixel, r, heatramp, b, 255);
  } else {
    writePixelNoShow(pixel, heatramp, g, b, 255);
  }
}


void setPixelRandomColor(int pixel, int temperature) {
  int r = random(100, 256);
  int g = random(0, 100);
  int b = random(0, 100);
  int brightness = (temperature > 255) ? 255 : temperature;

  writePixelNoShow(pixel, r, g, b, brightness);
}

void handleHeartbeat() {
  long startTime = millis();
  while (millis() - startTime < 15000) {  // Run for 15 seconds
    for (int i = 0; i < N_LEDS; i++) {
      int brightness = 255 * (0.5 * sin(2 * 3.14 * (millis() % 1500) / 1500) + 0.5);  // Sinusoidal pulse
      writePixelNoShow(i, brightness, 0, 0, 255);  // Red color for heartbeat
    }
    strip.show();
    delay(30);
  }
}

void handleHeartbeatCalled(int r, int g, int b) {
  long startTime = millis();
  while (millis() - startTime < 15000) {  // Run for 15 seconds
    for (int i = 0; i < N_LEDS; i++) {
      long currentMillis = millis();
      int brightness = 255 * (0.5 * sin(2 * 3.1415 * (currentMillis % 1500) / 1500.0) + 0.5);  // Sinusoidal pulse
      int adjR = (r * brightness) / 255;
      int adjG = (g * brightness) / 255;
      int adjB = (b * brightness) / 255;
      writePixelNoShow(i, adjR, adjG, adjB, 255);  // Use writePixelNoShow for color manipulation
    }
    strip.show();
    delay(30);  // Update delay as needed
  }
}


void handleFractal() {
  long startTime = millis();
  while (millis() - startTime < 15000) {  // Run for 15 seconds
    for (int offset = 0; offset < N_LEDS; offset++) {
      for (int i = 0; i < N_LEDS; i++) {
        int fractalValue = (int)(128.0 + 128.0 * sin(i * 3.1415 / 16.0 + millis() / 1000.0 + offset / 10.0));
        writePixelNoShow(i, 0, 0, fractalValue, 255); // Use writePixelNoShow for color manipulation
      }
      strip.show();
      delay(20);  // Reduced delay for faster updates
    }
  }
}

void handleFractalCalled(int r, int g, int b) {
  long startTime = millis();
  while (millis() - startTime < 15000) {  // Run for 15 seconds
    for (int offset = 0; offset < N_LEDS; offset++) {
      for (int i = 0; i < N_LEDS; i++) {
        int fractalValue = (int)(128.0 + 128.0 * sin(i * 3.1415 / 16.0 + millis() / 1000.0 + offset / 10.0));
        int adjR = (r * fractalValue) / 255;
        int adjG = (g * fractalValue) / 255;
        int adjB = (b * fractalValue) / 255;
        writePixelNoShow(i, adjR, adjG, adjB, 255); // Use writePixelNoShow for color manipulation
      }
      strip.show();
      delay(20);  // Reduced delay for faster updates
    }
  }
}



void startAnimationTask() {
    int currentColor[3] = {255, 255, 255};
    while (true) {
      int animationType = random(0, 5);  // Randomly pick an animation
      // Set a random color
      currentColor[0] = random(0, 256);
      currentColor[1] = random(0, 256);
      currentColor[2] = random(0, 256);
      switch (animationType) {
          case 0:
              handleFireCalled(currentColor[0], currentColor[1], currentColor[2]);
              break;
          case 1:
              handleRainbowCalled(currentColor[0], currentColor[1], currentColor[2]);
              break;
          case 2:
              handleWaveCalled(currentColor[0], currentColor[1], currentColor[2]);
              break;
          case 3:
              handleHeartbeatCalled(currentColor[0], currentColor[1], currentColor[2]);
              break;
          case 4:
              handleFractalCalled(currentColor[0], currentColor[1], currentColor[2]);
              break;
      }
    }
}
    



// ******************************************************************************************************************
// end
