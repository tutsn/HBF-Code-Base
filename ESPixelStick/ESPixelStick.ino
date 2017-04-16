/*
* ESPixelStick.ino
*
* Project: ESPixelStick - An ESP8266 and E1.31 based pixel driver
* Copyright (c) 2016 Shelby Merrick
* http://www.forkineye.com
*
*  This program is provided free for you to use in any way that you wish,
*  subject to the laws and regulations where you are using it.  Due diligence
*  is strongly suggested before using this code.  Please give credit where due.
*
*  The Author makes no warranty of any kind, express or implied, with regard
*  to this program or the documentation contained in this document.  The
*  Author shall not be liable in any event for incidental or consequential
*  damages in connection with, or arising out of, the furnishing, performance
*  or use of these programs.
*
*/


/*****************************************/
/*        BEGIN - Configuration          */
/*****************************************/

/* Output Mode - There can be only one! (-Conor MacLeod) */
#define ESPS_MODE_PIXEL
//#define ESPS_MODE_SERIAL

/* Fallback configuration if config.json is empty or fails */
const char ssid[] = "capetown";
const char passphrase[] = "strangersrest123";

/*****************************************/
/*         END - Configuration           */
/*****************************************/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFiUdp.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <Hash.h>
#include <SPI.h>
#include <E131.h>
#include "ESPixelStick.h"
#include "EFUpdate.h"
#include "wshandler.h"
#include <Wire.h>



extern "C" {
#include <user_interface.h>
}

uint8_t             *seqTracker;        /* Current sequence numbers for each Universe */
uint32_t             lastUpdate;         /* Update timeout tracker */



/* Forward Declarations */
void loadConfig();
int initWifi();
void initWeb();
void updateConfig();

RF_PRE_INIT() {
    //wifi_set_phy_mode(PHY_MODE_11G);    // Force 802.11g mode
    system_phy_set_powerup_option(31);  // Do full RF calibration on power-up
    system_phy_set_max_tpw(82);         // Set max TX power
}

void setup() {
    /* Configure SDK params */
    wifi_set_sleep_type(NONE_SLEEP_T);

    /* Generate and set hostname */
    char chipId[7] = { 0 };
    snprintf(chipId, sizeof(chipId), "%06x", ESP.getChipId());
    String hostname = "esps_" + String(chipId);
    WiFi.hostname(hostname);

    /* Initial pin states */
    pinMode(DATA_PIN, OUTPUT);
    digitalWrite(DATA_PIN, LOW);

    /* Setup serial log port */
    LOG_PORT.begin(115200);
    delay(10);

    /* Enable SPIFFS */
    SPIFFS.begin();

    LOG_PORT.println("");
    LOG_PORT.print(F("ESPixelStick v"));
    for (uint8_t i = 0; i < strlen_P(VERSION); i++)
        LOG_PORT.print((char)(pgm_read_byte(VERSION + i)));
    LOG_PORT.println("");

    /* Load configuration from SPIFFS */
    loadConfig();
    config.testmode = TestMode::DISABLED;

   
    int status = initWifi();
     /* Fallback to default SSID and passphrase if we fail to connect 
    if (status != WL_CONNECTED) {
        LOG_PORT.println(F("*** Timeout - Reverting to default SSID ***"));
        config.ssid = ssid;
        config.passphrase = passphrase;
        status = initWifi();
    } */

    /* If we fail again, go SoftAP or reboot */
    if (status != WL_CONNECTED) {
        if (config.ap_fallback) {
            LOG_PORT.println(F("**** FAILED TO ASSOCIATE WITH AP, GOING SOFTAP ****"));
            WiFi.mode(WIFI_AP);
            String ssid = "ESPixel " + String(chipId);
            WiFi.softAP(ssid.c_str());
        } else {
            LOG_PORT.println(F("**** FAILED TO ASSOCIATE WITH AP, REBOOTING ****"));
            ESP.eraseConfig();
            ESP.reset();
        }
    }

    /* Configure and start the web server */
    initWeb();

    /* Setup mDNS / DNS-SD */
    //TODO: Reboot or restart mdns when config.id is changed?
    MDNS.setInstanceName(config.id + " (" + String(chipId) + ")");
    if (MDNS.begin(hostname.c_str())) {
        MDNS.addService("http", "tcp", HTTP_PORT);
        MDNS.addService("e131", "udp", E131_DEFAULT_PORT);
        MDNS.addServiceTxt("e131", "udp", "TxtVers", String(RDMNET_DNSSD_TXTVERS));
        MDNS.addServiceTxt("e131", "udp", "ConfScope", RDMNET_DEFAULT_SCOPE);
        MDNS.addServiceTxt("e131", "udp", "E133Vers", String(RDMNET_DNSSD_E133VERS));
        MDNS.addServiceTxt("e131", "udp", "CID", String(chipId));
        MDNS.addServiceTxt("e131", "udp", "Model", "ESPixelStick");
        MDNS.addServiceTxt("e131", "udp", "Manuf", "Forkineye");
    } else {
        LOG_PORT.println(F("*** Error setting up mDNS responder ***"));
    }

    // I^2C communication setup
    Wire.begin(4,5);

    //SonarStairs


    /* Configure the outputs */
    #if defined (ESPS_MODE_PIXEL)
        pixels.setPin(DATA_PIN);
        updateConfig();
        pixels.show();
    #else
        updateConfig();
    #endif



  

// setup the FastLed fallback animations when in WS2812 mode
    fastled_setup(config.channel_count / 3);
    stairs_matrix_setup();

    backup_fire.setupAnimation(config.fire_cooling , config.fire_sparking);
    backup_fire.setEnvironment(config.channel_count/3, 0, false);    
    backup_sparkle.setupAnimation(config.sparkle_fps, config.sparkle_cooling, config.sparkle_twinkle, config.sparkle_flicker, config.sparkle_bpm, config.sparkle_hue);
    backup_sparkle.setEnvironment(config.channel_count/3, 0, false);
   
    setupBackupAnimations();

    // set D3 to OUTPUT LOW to open the LLS and show status led 
    pinMode(D3, OUTPUT);
    digitalWrite(D3, LOW);
}

/////////////////////////////////////////////
// update backup animations
/////////////////////////////////////////////
void updateBackupanimations()
{
    if (AnimationFire.size() >0)
    {
      LOG_PORT.print("Fuckup Fire! Size: ");
      LOG_PORT.println(AnimationFire.size());
      
      for (int fu=0;fu < AnimationFire.size() ; fu++)
      {
        AnimationFire[fu].fuckupEnvironment();   
      }
      AnimationFire.clear();
    }
    if (AnimationSparkle.size() >0)
    {
      LOG_PORT.print("Fuckup Sparkle! Size: ");
      LOG_PORT.println(AnimationSparkle.size());
      
      for (int fu=0;fu < AnimationSparkle.size() ; fu++)
      {
        AnimationSparkle[fu].fuckupEnvironment();   
      }
      AnimationSparkle.clear();
    }
    setupBackupAnimations();    
     
    
}

/////////////////////////////////////////////
// setup backup animations
/////////////////////////////////////////////
void setupBackupAnimations()
{
// testing looped setup of backup animation objects
    for (int i = 0 ; i < 16 ; i++)
    {
      LOG_PORT.print("setting up animation slot nr ");  
      LOG_PORT.print(i);
      LOG_PORT.print(": ");
      int slotmode = (static_cast<uint8_t>(config.fb_mode[i]));
    
      switch(config.fb_mode[i])
      {
        case NeopixelMode::disabled:
          LOG_PORT.println("disabled");
        break;
    
        case NeopixelMode::Fire:
          LOG_PORT.print("Fire --> num_leds: ");
          LOG_PORT.print(config.fb_numleds[i]);
          LOG_PORT.print(" / offset: ");
          LOG_PORT.print(config.fb_offset[i]);
          LOG_PORT.print(" / direction: ");
          LOG_PORT.println(config.fb_reverse[i]);
          
          AnimationFire.push_back(Fire());
    
          LOG_PORT.print("-- Fire vector size ");
          LOG_PORT.println(AnimationFire.size());  
    
          AnimationFire.back().setupAnimation(config.fire_cooling, config.fire_sparking);
          AnimationFire.back().setEnvironment(config.fb_numleds[i], config.fb_offset[i], config.fb_reverse[i]);
        
        break;
    
        case NeopixelMode::Sparkle:  
          LOG_PORT.print("Sparkle --> num_leds: ");
          LOG_PORT.print(config.fb_numleds[i]);
          LOG_PORT.print(" / offset: ");
          LOG_PORT.print(config.fb_offset[i]);
          LOG_PORT.print(" / direction: ");
          LOG_PORT.println(config.fb_reverse[i]);  
    
          AnimationSparkle.push_back(Sparkle());
    
          LOG_PORT.print("-- Sparkle vector size ");
          LOG_PORT.println(AnimationSparkle.size()); 
    
          AnimationSparkle.back().setupAnimation(config.sparkle_fps, config.sparkle_cooling, config.sparkle_twinkle, config.sparkle_flicker, config.sparkle_bpm, config.sparkle_hue);
          AnimationSparkle.back().setEnvironment(config.fb_numleds[i], config.fb_offset[i], config.fb_reverse[i]);       
        break;
    
        case NeopixelMode::Other:        
          LOG_PORT.print("Other --> num_leds: ");
          LOG_PORT.print(config.fb_numleds[i]);
          LOG_PORT.print(" / offset: ");
          LOG_PORT.print(config.fb_offset[i]);
          LOG_PORT.print(" / direction: ");
          LOG_PORT.println(config.fb_reverse[i]);      
        break;            
      }   
    }
}

int initWifi() {
    /* Switch to station mode and disconnect just in case */
    WiFi.mode(WIFI_STA);
    delay(secureRandom(100,500));
    WiFi.disconnect();
    delay(secureRandom(2000,5000));


    LOG_PORT.println("");
    LOG_PORT.print(F("Connecting to "));
    LOG_PORT.println(config.ssid);

    WiFi.begin(config.ssid.c_str(), config.passphrase.c_str());
    if (config.dhcp) {
        LOG_PORT.print(F("Connecting with DHCP"));
    } else {
        /* We don't use DNS, so just set it to our gateway */
        WiFi.config(IPAddress(config.ip[0], config.ip[1], config.ip[2], config.ip[3]),
                    IPAddress(config.gateway[0], config.gateway[1], config.gateway[2], config.gateway[3]),
                    IPAddress(config.netmask[0], config.netmask[1], config.netmask[2], config.netmask[3]),
                    IPAddress(config.gateway[0], config.gateway[1], config.gateway[2], config.gateway[3])
        );
        LOG_PORT.print(F("Connecting with Static IP"));
    }

    uint32_t timeout = millis();
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        LOG_PORT.print(".");
        if (millis() - timeout > CONNECT_TIMEOUT) {
            LOG_PORT.println("");
            LOG_PORT.println(F("*** Failed to connect ***"));
            break;
        }
    }

    if (WiFi.status() == WL_CONNECTED) {
        LOG_PORT.println("");
        LOG_PORT.print(F("Connected with IP: "));
        LOG_PORT.println(WiFi.localIP());

        if (config.multicast)
            e131.begin(E131_MULTICAST, config.universe,
                    uniLast - config.universe + 1);
        else
            e131.begin(E131_UNICAST);
    }

    return WiFi.status();


}

/* Configure and start the web server */
void initWeb() {
    /* Handle OTA update from asynchronous callbacks */
    Update.runAsync(true);

    /* Setup WebSockets */
    ws.onEvent(wsEvent);
    web.addHandler(&ws);

    /* Heap status handler */
    web.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", String(ESP.getFreeHeap()));
    });

    /* JSON Config Handler */
    web.on("/conf", HTTP_GET, [](AsyncWebServerRequest *request) {
        String jsonString;
        serializeConfig(jsonString);
        request->send(200, "text/json", jsonString);
    });

    /* Firmware upload handler */
    web.on("/updatefw", HTTP_POST, [](AsyncWebServerRequest *request) {
        ws.textAll("X6");
    }, handle_fw_upload);

    /* Static Handler */
    web.serveStatic("/", SPIFFS, "/www/").setDefaultFile("index.html");

    web.onNotFound([](AsyncWebServerRequest *request) {
        request->send(404, "text/plain", "Page not found");
    });

    web.begin();

    LOG_PORT.print(F("- Web Server started on port "));
    LOG_PORT.println(HTTP_PORT);
}

/* Configuration Validations */
void validateConfig() {
    /* E1.31 Limits */
    if (config.universe < 1)
        config.universe = 1;

    if (config.channel_start < 1)
        config.channel_start = 1;
    else if (config.channel_start > UNIVERSE_LIMIT)
        config.channel_start = UNIVERSE_LIMIT;

#if defined(ESPS_MODE_PIXEL)
    /* Set Mode */
    config.devmode = DevMode::MPIXEL;

    /* Generic channel limits for pixels */
    if (config.channel_count % 3)
        config.channel_count = (config.channel_count / 3) * 3;

    if (config.channel_count > PIXEL_LIMIT * 3)
        config.channel_count = PIXEL_LIMIT * 3;
    else if (config.channel_count < 1)
        config.channel_count = 1;

    /* GECE Limits */
    if (config.pixel_type == PixelType::GECE) {
        config.pixel_color = PixelColor::RGB;
        if (config.channel_count > 63 * 3)
            config.channel_count = 63 * 3;
    }

#elif defined(ESPS_MODE_SERIAL)
    /* Set Mode */
    config.devmode = DevMode::MSERIAL;

    /* Generic serial channel limits */
    if (config.channel_count > RENARD_LIMIT)
        config.channel_count = RENARD_LIMIT;
    else if (config.channel_count < 1)
        config.channel_count = 1;

    if (config.serial_type == SerialType::DMX512 && config.channel_count > UNIVERSE_LIMIT)
        config.channel_count = UNIVERSE_LIMIT;

    /* Baud rate check */
    if (config.baudrate > BaudRate::BR_460800)
        config.baudrate = BaudRate::BR_460800;
    else if (config.baudrate < BaudRate::BR_38400)
        config.baudrate = BaudRate::BR_57600;
#endif
}

void updateConfig() {
    /* Validate first */
    validateConfig();

    /* Find the last universe we should listen for */
    uint16_t span = config.channel_start + config.channel_count - 1;
    if (span % UNIVERSE_LIMIT)
        uniLast = config.universe + span / UNIVERSE_LIMIT;
    else
        uniLast = config.universe + span / UNIVERSE_LIMIT - 1;

    /* Setup the sequence error tracker */
    uint8_t uniTotal = (uniLast + 1) - config.universe;

    if (seqTracker) free(seqTracker);
    if (seqTracker = static_cast<uint8_t *>(malloc(uniTotal)))
        memset(seqTracker, 0x00, uniTotal);

    if (seqError) free(seqError);
    if (seqError = static_cast<uint32_t *>(malloc(uniTotal * 4)))
        memset(seqError, 0x00, uniTotal * 4);

    /* Zero out packet stats */
    e131.stats.num_packets = 0;

    /* Send Animation config Data to PixelDriver */
    if (config.ultrasonic) {
        LOG_PORT.println(F("Ultrasonic ON"));
        pixels.ultrasonic = true;
    } else {
        LOG_PORT.println(F("Ultrasonic OFF"));
        pixels.ultrasonic = false;
    }
    config.peri_universe = 3;
    config.num_peri_dimmers = 2;

    // Load stair config data into Pixel Driver
    pixels.stairPixelData = {config.num_stairs, config.step_length, config.num_pixels, config.trigger_dist};

    /* Initialize for our pixel type */
#if defined(ESPS_MODE_PIXEL)
    pixels.begin(config.pixel_type, config.pixel_color, config.channel_count / 3);
    pixels.setGamma(config.gamma);
#elif defined(ESPS_MODE_SERIAL)
    serial.begin(&SEROUT_PORT, config.serial_type, config.channel_count, config.baudrate);
#endif
    LOG_PORT.print(F("- Listening for "));
    LOG_PORT.print(config.channel_count);
    LOG_PORT.print(F(" channels, from Universe "));
    LOG_PORT.print(config.universe);
    LOG_PORT.print(F(" to "));
    LOG_PORT.println(uniLast);

    backup_fire.updateConfig(config.fire_cooling, config.fire_sparking, false);
    backup_sparkle.updateConfig(config.sparkle_fps, config.sparkle_cooling, config.sparkle_twinkle, config.sparkle_flicker, config.sparkle_bpm, config.sparkle_hue);

    updateBackupanimations();
}

/* De-Serialize Network config */
void dsNetworkConfig(JsonObject &json) {
    /* Fallback to embedded ssid and passphrase if null in config */
    if (strlen(json["network"]["ssid"]))
        config.ssid = json["network"]["ssid"].as<String>();
    else
        config.ssid = ssid;

    if (strlen(json["network"]["passphrase"]))
        config.passphrase = json["network"]["passphrase"].as<String>();
    else
        config.passphrase = passphrase;

    /* Network */
    for (int i = 0; i < 4; i++) {
        config.ip[i] = json["network"]["ip"][i];
        config.netmask[i] = json["network"]["netmask"][i];
        config.gateway[i] = json["network"]["gateway"][i];
    }
    config.dhcp = json["network"]["dhcp"];
    config.ap_fallback = json["network"]["ap_fallback"];
}

void dsDeviceConfig(JsonObject &json) {
    /* Device */
    config.id = json["device"]["id"].as<String>();

    /* E131 */
    config.universe = json["e131"]["universe"];
    config.channel_start = json["e131"]["channel_start"];
    config.channel_count = json["e131"]["channel_count"];
    config.multicast = json["e131"]["multicast"];

    /* HBF */
     /* Fetch Ultrasonic */
    config.ultrasonic = json["hbf"]["ultrasonic"];

    /* Fallback Mode */
    for (int i = 0; i < 16; i++) {
        config.fb_mode[i] = NeopixelMode(static_cast<uint8_t>(json["fallback"]["fb_mode"][i]));
        config.fb_numleds[i] = json["fallback"]["fb_numleds"][i];
        config.fb_offset[i] = json["fallback"]["fb_offset"][i];
        config.fb_reverse[i] = json["fallback"]["fb_reverse"][i];
    }

    /* NOISEMATRIX */
    config.matrix_xdim = json["noisematrix"]["xdim"];
    config.matrix_ydim = json["noisematrix"]["ydim"];    
    config.matrix_fps = json["noisematrix"]["fps"];
    config.matrix_spp = json["noisematrix"]["spp"];
    config.matrix_scale = json["noisematrix"]["scale"];
    config.matrix_speed = json["noisematrix"]["speed"];    

    /* Fire */
    config.fire_fps = json["fire"]["fps"];
    config.fire_cooling = json["fire"]["cooling"];
    config.fire_sparking = json["fire"]["sparking"];   

    /* Sparkle */
    config.sparkle_fps = json["sparkle"]["fps"];
    config.sparkle_cooling = json["sparkle"]["cooling"];
    config.sparkle_twinkle = json["sparkle"]["twinkle"]; 
    config.sparkle_flicker = json["sparkle"]["flicker"];
    config.sparkle_bpm = json["sparkle"]["bpm"];
    config.sparkle_hue = json["sparkle"]["hue"];       

    /* Stairs */
    config.num_stairs = json["noisematrix"]["ydim"];                          
    config.step_length = json["stairs"]["leng"];                       
    config.num_pixels = json["noisematrix"]["xdim"];                          
    config.trigger_dist = json["stairs"]["triggerdist"];                        

    /* Peripheral Universe */
    config.use_peripherial_dmx = json["stairs"]["peridmx"];                
    config.peri_universe = json["stairs"]["periuniverse"];                       
    config.num_peri_dimmers = json["stairs"]["peridimmers"];                   

#if defined(ESPS_MODE_PIXEL)
    /* Pixel */
    config.pixel_type = PixelType(static_cast<uint8_t>(json["pixel"]["type"]));
    config.pixel_color = PixelColor(static_cast<uint8_t>(json["pixel"]["color"]));
    config.gamma = json["pixel"]["gamma"];

#elif defined(ESPS_MODE_SERIAL)
    /* Serial */
    config.serial_type = SerialType(static_cast<uint8_t>(json["serial"]["type"]));
    config.baudrate = BaudRate(static_cast<uint32_t>(json["serial"]["baudrate"]));
#endif
}

/* Load configugration JSON file */
void loadConfig() {
    /* Zeroize Config struct */
    memset(&config, 0, sizeof(config));

    /* Load CONFIG_FILE json. Create and init with defaults if not found */
    File file = SPIFFS.open(CONFIG_FILE, "r");
    if (!file) {
        LOG_PORT.println(F("- No configuration file found."));
        config.ssid = ssid;
        config.passphrase = passphrase;
        saveConfig();
    } else {
        /* Parse CONFIG_FILE json */
        size_t size = file.size();
        if (size > CONFIG_MAX_SIZE) {
            LOG_PORT.println(F("*** Configuration File too large ***"));
            return;
        }

        std::unique_ptr<char[]> buf(new char[size]);
        file.readBytes(buf.get(), size);

        DynamicJsonBuffer jsonBuffer;
        JsonObject &json = jsonBuffer.parseObject(buf.get());
        if (!json.success()) {
            LOG_PORT.println(F("*** Configuration File Format Error ***"));
            return;
        }

        dsNetworkConfig(json);
        dsDeviceConfig(json);

        LOG_PORT.println(F("- Configuration loaded."));
    }

    /* Validate it */
    validateConfig();
}

/* Serialize the current config into a JSON string */
void serializeConfig(String &jsonString, bool pretty, bool creds) {
    /* Create buffer and root object */
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.createObject();

    /* Device */
    JsonObject &device = json.createNestedObject("device");
    device["id"] = config.id.c_str();
    device["mode"] = static_cast<uint8_t>(config.devmode);

    /* Network */
    JsonObject &network = json.createNestedObject("network");
    network["ssid"] = config.ssid.c_str();
    if (creds)
        network["passphrase"] = config.passphrase.c_str();
    JsonArray &ip = network.createNestedArray("ip");
    JsonArray &netmask = network.createNestedArray("netmask");
    JsonArray &gateway = network.createNestedArray("gateway");
    for (int i = 0; i < 4; i++) {
        ip.add(config.ip[i]);
        netmask.add(config.netmask[i]);
        gateway.add(config.gateway[i]);
    }
    network["dhcp"] = config.dhcp;
    network["ap_fallback"] = config.ap_fallback;

    /* E131 */
    JsonObject &e131 = json.createNestedObject("e131");
    e131["universe"] = config.universe;
    e131["channel_start"] = config.channel_start;
    e131["channel_count"] = config.channel_count;
    e131["multicast"] = config.multicast;

#if defined(ESPS_MODE_PIXEL)
    /* Pixel */
    JsonObject &pixel = json.createNestedObject("pixel");
    pixel["type"] = static_cast<uint8_t>(config.pixel_type);
    pixel["color"] = static_cast<uint8_t>(config.pixel_color);
    pixel["gamma"] = config.gamma;

#elif defined(ESPS_MODE_SERIAL)
    /* Serial */
    JsonObject &serial = json.createNestedObject("serial");
    serial["type"] = static_cast<uint8_t>(config.serial_type);
    serial["baudrate"] = static_cast<uint32_t>(config.baudrate);
#endif

    /* HBF */
    JsonObject &hbf = json.createNestedObject("hbf");
    /* Fetch Ultrasonic */
    hbf["ultrasonic"] = config.ultrasonic;

    /* Stairs */
    JsonObject &stairs = json.createNestedObject("stairs");
    stairs["leng"] = config.step_length;
    stairs["triggerdist"] = config.trigger_dist;
    stairs["peridmx"] = config.use_peripherial_dmx;
    stairs["periuniverse"] = config.peri_universe;
    stairs["peridimmers"] = config.num_peri_dimmers;
    

    /* Fallback Mode */
    JsonObject &fallback = json.createNestedObject("fallback");
    JsonArray &fb_mode = fallback.createNestedArray("fb_mode");
    JsonArray &fb_numleds = fallback.createNestedArray("fb_numleds");
    JsonArray &fb_offset = fallback.createNestedArray("fb_offset");
    JsonArray &fb_reverse = fallback.createNestedArray("fb_reverse");
    for (int i = 0; i < 16; i++) {
        fb_mode.add(static_cast<uint8_t>(config.fb_mode[i]));
        fb_numleds.add(config.fb_numleds[i]);
        fb_offset.add(config.fb_offset[i]);
        fb_reverse.add(config.fb_reverse[i]);
    }
    
    /* NOISEMATRIX */
    JsonObject &noisematrix = json.createNestedObject("noisematrix");
    noisematrix["xdim"] = config.matrix_xdim;
    noisematrix["ydim"] = config.matrix_ydim;
    noisematrix["fps"] = config.matrix_fps;
    noisematrix["spp"] = config.matrix_spp;
    noisematrix["scale"] = config.matrix_scale;
    noisematrix["speed"] = config.matrix_speed;    

    /* Fire */
    JsonObject &fire = json.createNestedObject("fire");
    fire["fps"] = config.fire_fps;
    fire["cooling"] = config.fire_cooling;
    fire["sparking"] = config.fire_sparking;

    /* Sparkle */
    JsonObject &sparkle = json.createNestedObject("sparkle");
    sparkle["fps"] = config.sparkle_fps;
    sparkle["cooling"] = config.sparkle_cooling;
    sparkle["twinkle"] = config.sparkle_twinkle;
    sparkle["flicker"] = config.sparkle_flicker;
    sparkle["bpm"] = config.sparkle_bpm;
    sparkle["hue"] = config.sparkle_hue;

    if (pretty)
        json.prettyPrintTo(jsonString);
    else
        json.printTo(jsonString);
}

/* Save configuration JSON file */
void saveConfig() {
    /* Update Config */
    updateConfig();

    /* Serialize Config */
    String jsonString;
    serializeConfig(jsonString, true, true);

    /* Save Config */
    File file = SPIFFS.open(CONFIG_FILE, "w");
    if (!file) {
        LOG_PORT.println(F("*** Error creating configuration file ***"));
        return;
    } else {
        file.println(jsonString);
        LOG_PORT.println(F("* Configuration saved."));
    }
}

/* Main Loop */
void loop() {
    /* Reboot handler */
    if (reboot) {
        delay(REBOOT_DELAY);
        ESP.restart();
    }

    if (config.testmode == TestMode::DISABLED || config.testmode == TestMode::VIEW_STREAM) {

        /* Parse a packet and update pixels */
        if (e131.parsePacket()) {

            if ((e131.universe >= config.universe) && (e131.universe <= uniLast + 1)) {
                /* Universe offset and sequence tracking */
                uint8_t uniOffset = (e131.universe - config.universe);
                if (e131.packet->sequence_number != seqTracker[uniOffset]++) {
                    seqError[uniOffset]++;
                    seqTracker[uniOffset] = e131.packet->sequence_number + 1;
                }

                /* Offset the channels if required */
                uint16_t offset = 0;
                offset = config.channel_start - 1;

                /* Find start of data based off the Universe */
                int16_t dataStart = uniOffset * UNIVERSE_LIMIT - offset;

                /* Calculate how much data we need for this buffer */
                uint16_t dataStop = config.channel_count;
                if ((dataStart + UNIVERSE_LIMIT) < dataStop)
                    dataStop = dataStart + UNIVERSE_LIMIT;

                /* Set the data */
                uint16_t buffloc = 0;
                
                /* ignore data from start of first Universe before channel_start */
                if(dataStart<0) {
                    dataStart=0;
                    buffloc = config.channel_start - 1;
                }  

                pixels.peri_dimmer.clear();
                if (e131.universe == config.peri_universe){
                    for (int i = dataStart; i < dataStart + config.num_peri_dimmers; i++) {
                        pixels.peri_dimmer.push_back(e131.data[buffloc]);
                        buffloc++;
                    }
                }
                else {
                    for (int i = dataStart; i < dataStop; i++) {
        #if defined(ESPS_MODE_PIXEL)
                        pixels.setValue(i, e131.data[buffloc]);
        #elif defined(ESPS_MODE_SERIAL)
                        serial.setValue(i, e131.data[buffloc]);
        #endif
                        buffloc++;
                    }
                }
            }
        }
    }
    else { //some other testmode
    
      //keep feeding server so we don't overrun with packets
      e131.parsePacket();
    
      uint16_t num_pixels;
      switch(config.testmode){

        case TestMode::NOISEMATRIX:
          
          if(millis() - testing.last > (1000 / config.matrix_fps)){
            //time for new step
            testing.last = millis();

          // call customized FastLed-routine and build a single frame
          stairs_matrix(config.matrix_spp);
          num_pixels = config.channel_count/3;
          
        #if defined(ESPS_MODE_PIXEL)    
            // now copy the FastLed frame to PixelDriver
            // NUM_LEDS comes from FastLed-routine btw.

            // ++ check if NUM_LEDS exceeds Pixel count ! 
            for (int i = 0 ; i < num_pixels ; i++)
            {
              int ch_offset = i*3;
              pixels.setValue(ch_offset++, leds[i].r);
              pixels.setValue(ch_offset++, leds[i].g);
              pixels.setValue(ch_offset, leds[i].b);
            }
        
        #elif defined(ESPS_MODE_SERIAL)
            
            // ++ check if NUM_LEDS exceeds Pixel count !
            for (int i = 0 ; i < num_pixels ; i++)
            {
              LOG_PORT.print(leds[i].r);
              LOG_PORT.print(" : ");
              LOG_PORT.print(leds[i].g);
              LOG_PORT.print(" : ");
              LOG_PORT.print(leds[i].b);
              LOG_PORT.println(" - ");
            }
        #endif 
          }
        break;

case TestMode::FIRE:

          
          if(millis() - testing.last > (1000 / config.fire_fps)){
            //time for new step
            testing.last = millis();

          // call customized FastLed-routine and build a single frame
          backup_fire.getFrame();
          num_pixels = config.channel_count/3;
          
        #if defined(ESPS_MODE_PIXEL)    
            // now copy the FastLed frame to PixelDriver

            num_pixels = config.channel_count/3;
            for (int i = 0 ; i < num_pixels ; i++)
            {
              int ch_offset = i*3;
              pixels.setValue(ch_offset++, backup_fire.leds[i].r);
              pixels.setValue(ch_offset++, backup_fire.leds[i].g);
              pixels.setValue(ch_offset, backup_fire.leds[i].b);  
            }
        
        #elif defined(ESPS_MODE_SERIAL)
            
            // ++ check if NUM_LEDS exceeds Pixel count !
            for (int i = 0 ; i < NUM_LEDS ; i++)
            {
              LOG_PORT.print(leds[i].r);
              LOG_PORT.print(" : ");
              LOG_PORT.print(leds[i].g);
              LOG_PORT.print(" : ");
              LOG_PORT.print(leds[i].b);
              LOG_PORT.println(" - ");
            }
        #endif 
          }
        break;

case TestMode::SPARKLE:
          
          if(millis() - testing.last > (1000 / config.sparkle_fps)){
            //time for new step
            testing.last = millis();

          // call customized FastLed-routine and build a single frame
          backup_sparkle.getFrame();
          num_pixels = config.channel_count/3;
        #if defined(ESPS_MODE_PIXEL)    
            // now copy the FastLed frame to PixelDriver
                        
            for (int i = 0 ; i < num_pixels ; i++)
            {
              int ch_offset = i*3;
              pixels.setValue(ch_offset++, backup_sparkle.leds[i].r);
              pixels.setValue(ch_offset++, backup_sparkle.leds[i].g);
              pixels.setValue(ch_offset, backup_sparkle.leds[i].b);
            }
        
        #elif defined(ESPS_MODE_SERIAL)
            
            // ++ check if NUM_LEDS exceeds Pixel count !
            for (int i = 0 ; i < num_pixels ; i++)
            {
              LOG_PORT.print(leds[i].r);
              LOG_PORT.print(" : ");
              LOG_PORT.print(leds[i].g);
              LOG_PORT.print(" : ");
              LOG_PORT.print(leds[i].b);
              LOG_PORT.println(" - ");
            }
        #endif 
          }
        break;        
        

case TestMode::FALLBACK:
          //run configured fallback animation
          // Fire::
          if(millis() - testing.last > (1000 / config.fire_fps))
          {
            //time for new step
            testing.last = millis();
            for (int vecpos = 0; vecpos < AnimationFire.size() ; vecpos++)
            {
              AnimationFire[vecpos].getFrame();
              for (int i= AnimationFire[vecpos].led_offset ; i < AnimationFire[vecpos].led_offset + AnimationFire[vecpos].num_leds ; i++)
              {
                int ch_offset = i*3;
                pixels.setValue(ch_offset++, AnimationFire[vecpos].leds[i - AnimationFire[vecpos].led_offset].r);
                pixels.setValue(ch_offset++, AnimationFire[vecpos].leds[i - AnimationFire[vecpos].led_offset].g);
                pixels.setValue(ch_offset, AnimationFire[vecpos].leds[i - AnimationFire[vecpos].led_offset].b); 
              }
            }  
          }
          // Fire::
          if(millis() - testing.last2 > (1000 / config.sparkle_fps))
          {
            //time for new step
            testing.last2 = millis();
            for (int vecpos = 0; vecpos < AnimationSparkle.size() ; vecpos++)
            {
              AnimationSparkle[vecpos].getFrame();
              for (int i= AnimationSparkle[vecpos].led_offset ; i < AnimationSparkle[vecpos].led_offset + AnimationSparkle[vecpos].num_leds ; i++)
              {
                int ch_offset = i*3;
                pixels.setValue(ch_offset++, AnimationSparkle[vecpos].leds[i - AnimationSparkle[vecpos].led_offset].r);
                pixels.setValue(ch_offset++, AnimationSparkle[vecpos].leds[i - AnimationSparkle[vecpos].led_offset].g);
                pixels.setValue(ch_offset, AnimationSparkle[vecpos].leds[i - AnimationSparkle[vecpos].led_offset].b); 
              }
            }  
          }          

          
        break;  

       
        case TestMode::STATIC: {
          
          //continue to update color to whole string
          uint16_t i = 0;
          while (i <= config.channel_count - 3) {
  #if defined(ESPS_MODE_PIXEL)
            pixels.setValue(i++, testing.r);
            pixels.setValue(i++, testing.g);
            pixels.setValue(i++, testing.b);
  #elif defined(ESPS_MODE_SERIAL)
            serial.setValue(i++, testing.r);
            serial.setValue(i++, testing.g);
            serial.setValue(i++, testing.b);
  #endif
              }
         }
         break;
       
        
        case TestMode::CHASE:
          //run chase routine
          
          if(millis() - testing.last > 100){
            //time for new step
            testing.last = millis();
            
  #if defined(ESPS_MODE_PIXEL)
            //clear whole string
            for(int y =0; y < config.channel_count; y++) pixels.setValue(y, 0);
            //set pixel at step
            int ch_offset = testing.step*3;
            pixels.setValue(ch_offset++, testing.r);
            pixels.setValue(ch_offset++, testing.g);
            pixels.setValue(ch_offset, testing.b);
            testing.step++;
            if(testing.step >= (config.channel_count/3)) testing.step = 0;
          
  #elif defined(ESPS_MODE_SERIAL)
            for(int y =0; y < config.channel_count; y++) serial.setValue(y, 0);
            //set pixel at step
            serial.setValue(testing.step++, 0xFF);
            if(testing.step >= config.channel_count) testing.step = 0;
  #endif   
          }
       
        break;
        
        
        case TestMode::RAINBOW:
                   
        break;
      }      
    }

/* Streaming refresh */
#if defined(ESPS_MODE_PIXEL)
    if (pixels.canRefresh())
        pixels.show();
#elif defined(ESPS_MODE_SERIAL)
    if (serial.canRefresh())
        serial.show();
#endif
}
