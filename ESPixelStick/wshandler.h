/*
* ESPixelStick.h
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

#ifndef WSHANDLER_H_
#define WSHANDLER_H_

/* 
  Packet Commands
    E1 - Get Elements

    G1 - Get Config
    G2 - Get Config Status
    
    T0 - Disable Testing
    T1 - Static Testing

    S1 - Set Network Config
    S2 - Set Device Config

    X1 - Get RSSI
    X2 - Get E131 Status
    X6 - Reboot
*/

EFUpdate efupdate;

void procX(uint8_t *data, AsyncWebSocketClient *client) {
    switch (data[1]) {
        case '1':
            client->text("X1" + (String)WiFi.RSSI());
            break;
        case '2': {
            uint32_t seqErrors = 0;
            for (int i = 0; i < ((uniLast + 1) - config.universe); i++)
                seqErrors =+ seqError[i];

            client->text("X2" + (String)config.universe + ":" +
                    (String)uniLast + ":" +
                    (String)e131.stats.num_packets + ":" +
                    (String)seqErrors + ":" +
                    (String)e131.stats.packet_errors);
            break;
        }
        case '6':  // Init 6 baby, reboot!
            reboot = true;
    }
}

void procE(uint8_t *data, AsyncWebSocketClient *client) {
    switch (data[1]) {
        case '1':
            // Create buffer and root object
            DynamicJsonBuffer jsonBuffer;
            JsonObject &json = jsonBuffer.createObject();

#if defined (ESPS_MODE_PIXEL)
            // Pixel Types
            JsonObject &p_type = json.createNestedObject("p_type");
            p_type["WS2811 800kHz"] = static_cast<uint8_t>(PixelType::WS2811);
            p_type["GE Color Effects"] = static_cast<uint8_t>(PixelType::GECE);

            // Pixel Colors
            JsonObject &p_color = json.createNestedObject("p_color");
            p_color["RGB"] = static_cast<uint8_t>(PixelColor::RGB);
            p_color["GRB"] = static_cast<uint8_t>(PixelColor::GRB);
            p_color["BRG"] = static_cast<uint8_t>(PixelColor::BRG);
            p_color["RBG"] = static_cast<uint8_t>(PixelColor::RBG);
            p_color["GBR"] = static_cast<uint8_t>(PixelColor::GBR);
            p_color["BGR"] = static_cast<uint8_t>(PixelColor::BGR);

#elif defined (ESPS_MODE_SERIAL)
            // Serial Protocols
            JsonObject &s_proto = json.createNestedObject("s_proto");
            s_proto["DMX512"] = static_cast<uint8_t>(SerialType::DMX512);
            s_proto["Renard"] = static_cast<uint8_t>(SerialType::RENARD);

            // Serial Baudrates
            JsonObject &s_baud = json.createNestedObject("s_baud");
            s_baud["38400"] = static_cast<uint32_t>(BaudRate::BR_38400);
            s_baud["57600"] = static_cast<uint32_t>(BaudRate::BR_57600);
            s_baud["115200"] = static_cast<uint32_t>(BaudRate::BR_115200);
            s_baud["230400"] = static_cast<uint32_t>(BaudRate::BR_230400);
            s_baud["250000"] = static_cast<uint32_t>(BaudRate::BR_250000);
            s_baud["460800"] = static_cast<uint32_t>(BaudRate::BR_460800);
#endif
            // Fallback Modes
            JsonObject &hbf_fb_mode_1 = json.createNestedObject("hbf_fb_mode_1");
            hbf_fb_mode_1["disabled"] = static_cast<uint8_t>(NeopixelMode::disabled);
            hbf_fb_mode_1["Fire"] = static_cast<uint8_t>(NeopixelMode::Fire);
            hbf_fb_mode_1["Sparkle"] = static_cast<uint8_t>(NeopixelMode::Sparkle);
            hbf_fb_mode_1["Other"] = static_cast<uint8_t>(NeopixelMode::Other);

            JsonObject &hbf_fb_mode_2 = json.createNestedObject("hbf_fb_mode_2");
            hbf_fb_mode_2["disabled"] = static_cast<uint8_t>(NeopixelMode::disabled);
            hbf_fb_mode_2["Fire"] = static_cast<uint8_t>(NeopixelMode::Fire);
            hbf_fb_mode_2["Sparkle"] = static_cast<uint8_t>(NeopixelMode::Sparkle);
            hbf_fb_mode_2["Other"] = static_cast<uint8_t>(NeopixelMode::Other);

            JsonObject &hbf_fb_mode_3 = json.createNestedObject("hbf_fb_mode_3");
            hbf_fb_mode_3["disabled"] = static_cast<uint8_t>(NeopixelMode::disabled);
            hbf_fb_mode_3["Fire"] = static_cast<uint8_t>(NeopixelMode::Fire);
            hbf_fb_mode_3["Sparkle"] = static_cast<uint8_t>(NeopixelMode::Sparkle);
            hbf_fb_mode_3["Other"] = static_cast<uint8_t>(NeopixelMode::Other);

            JsonObject &hbf_fb_mode_4 = json.createNestedObject("hbf_fb_mode_4");
            hbf_fb_mode_4["disabled"] = static_cast<uint8_t>(NeopixelMode::disabled);
            hbf_fb_mode_4["Fire"] = static_cast<uint8_t>(NeopixelMode::Fire);
            hbf_fb_mode_4["Sparkle"] = static_cast<uint8_t>(NeopixelMode::Sparkle);
            hbf_fb_mode_4["Other"] = static_cast<uint8_t>(NeopixelMode::Other);   

            JsonObject &hbf_fb_mode_5 = json.createNestedObject("hbf_fb_mode_5");
            hbf_fb_mode_5["disabled"] = static_cast<uint8_t>(NeopixelMode::disabled);
            hbf_fb_mode_5["Fire"] = static_cast<uint8_t>(NeopixelMode::Fire);
            hbf_fb_mode_5["Sparkle"] = static_cast<uint8_t>(NeopixelMode::Sparkle);
            hbf_fb_mode_5["Other"] = static_cast<uint8_t>(NeopixelMode::Other);   

            JsonObject &hbf_fb_mode_6 = json.createNestedObject("hbf_fb_mode_6");
            hbf_fb_mode_6["disabled"] = static_cast<uint8_t>(NeopixelMode::disabled);
            hbf_fb_mode_6["Fire"] = static_cast<uint8_t>(NeopixelMode::Fire);
            hbf_fb_mode_6["Sparkle"] = static_cast<uint8_t>(NeopixelMode::Sparkle);
            hbf_fb_mode_6["Other"] = static_cast<uint8_t>(NeopixelMode::Other);   

            JsonObject &hbf_fb_mode_7 = json.createNestedObject("hbf_fb_mode_7");
            hbf_fb_mode_7["disabled"] = static_cast<uint8_t>(NeopixelMode::disabled);
            hbf_fb_mode_7["Fire"] = static_cast<uint8_t>(NeopixelMode::Fire);
            hbf_fb_mode_7["Sparkle"] = static_cast<uint8_t>(NeopixelMode::Sparkle);
            hbf_fb_mode_7["Other"] = static_cast<uint8_t>(NeopixelMode::Other);   

            JsonObject &hbf_fb_mode_8 = json.createNestedObject("hbf_fb_mode_8");
            hbf_fb_mode_8["disabled"] = static_cast<uint8_t>(NeopixelMode::disabled);
            hbf_fb_mode_8["Fire"] = static_cast<uint8_t>(NeopixelMode::Fire);
            hbf_fb_mode_8["Sparkle"] = static_cast<uint8_t>(NeopixelMode::Sparkle);
            hbf_fb_mode_8["Other"] = static_cast<uint8_t>(NeopixelMode::Other);   

            JsonObject &hbf_fb_mode_9 = json.createNestedObject("hbf_fb_mode_9");
            hbf_fb_mode_9["disabled"] = static_cast<uint8_t>(NeopixelMode::disabled);
            hbf_fb_mode_9["Fire"] = static_cast<uint8_t>(NeopixelMode::Fire);
            hbf_fb_mode_9["Sparkle"] = static_cast<uint8_t>(NeopixelMode::Sparkle);
            hbf_fb_mode_9["Other"] = static_cast<uint8_t>(NeopixelMode::Other);   

            JsonObject &hbf_fb_mode_10 = json.createNestedObject("hbf_fb_mode_10");
            hbf_fb_mode_10["disabled"] = static_cast<uint8_t>(NeopixelMode::disabled);
            hbf_fb_mode_10["Fire"] = static_cast<uint8_t>(NeopixelMode::Fire);
            hbf_fb_mode_10["Sparkle"] = static_cast<uint8_t>(NeopixelMode::Sparkle);
            hbf_fb_mode_10["Other"] = static_cast<uint8_t>(NeopixelMode::Other);   

            JsonObject &hbf_fb_mode_11 = json.createNestedObject("hbf_fb_mode_11");
            hbf_fb_mode_11["disabled"] = static_cast<uint8_t>(NeopixelMode::disabled);
            hbf_fb_mode_11["Fire"] = static_cast<uint8_t>(NeopixelMode::Fire);
            hbf_fb_mode_11["Sparkle"] = static_cast<uint8_t>(NeopixelMode::Sparkle);
            hbf_fb_mode_11["Other"] = static_cast<uint8_t>(NeopixelMode::Other);   

            JsonObject &hbf_fb_mode_12 = json.createNestedObject("hbf_fb_mode_12");
            hbf_fb_mode_12["disabled"] = static_cast<uint8_t>(NeopixelMode::disabled);
            hbf_fb_mode_12["Fire"] = static_cast<uint8_t>(NeopixelMode::Fire);
            hbf_fb_mode_12["Sparkle"] = static_cast<uint8_t>(NeopixelMode::Sparkle);
            hbf_fb_mode_12["Other"] = static_cast<uint8_t>(NeopixelMode::Other);   

            JsonObject &hbf_fb_mode_13 = json.createNestedObject("hbf_fb_mode_13");
            hbf_fb_mode_13["disabled"] = static_cast<uint8_t>(NeopixelMode::disabled);
            hbf_fb_mode_13["Fire"] = static_cast<uint8_t>(NeopixelMode::Fire);
            hbf_fb_mode_13["Sparkle"] = static_cast<uint8_t>(NeopixelMode::Sparkle);
            hbf_fb_mode_13["Other"] = static_cast<uint8_t>(NeopixelMode::Other);   

            JsonObject &hbf_fb_mode_14 = json.createNestedObject("hbf_fb_mode_14");
            hbf_fb_mode_14["disabled"] = static_cast<uint8_t>(NeopixelMode::disabled);
            hbf_fb_mode_14["Fire"] = static_cast<uint8_t>(NeopixelMode::Fire);
            hbf_fb_mode_14["Sparkle"] = static_cast<uint8_t>(NeopixelMode::Sparkle);
            hbf_fb_mode_14["Other"] = static_cast<uint8_t>(NeopixelMode::Other);   

            JsonObject &hbf_fb_mode_15 = json.createNestedObject("hbf_fb_mode_15");
            hbf_fb_mode_15["disabled"] = static_cast<uint8_t>(NeopixelMode::disabled);
            hbf_fb_mode_15["Fire"] = static_cast<uint8_t>(NeopixelMode::Fire);
            hbf_fb_mode_15["Sparkle"] = static_cast<uint8_t>(NeopixelMode::Sparkle);
            hbf_fb_mode_15["Other"] = static_cast<uint8_t>(NeopixelMode::Other);   

            JsonObject &hbf_fb_mode_16 = json.createNestedObject("hbf_fb_mode_16");
            hbf_fb_mode_16["disabled"] = static_cast<uint8_t>(NeopixelMode::disabled);
            hbf_fb_mode_16["Fire"] = static_cast<uint8_t>(NeopixelMode::Fire);
            hbf_fb_mode_16["Sparkle"] = static_cast<uint8_t>(NeopixelMode::Sparkle);
            hbf_fb_mode_16["Other"] = static_cast<uint8_t>(NeopixelMode::Other);                                                                                                                                                                                    

            String response;
            json.printTo(response);
            client->text("E1" + response);
            break;
    }
}

void procG(uint8_t *data, AsyncWebSocketClient *client) {
    switch (data[1]) {
        case '1': {
            String response;
            serializeConfig(response, false, true);
            client->text("G1" + response);
            break;
        }
        case '2':
            // Create buffer and root object
            DynamicJsonBuffer jsonBuffer;
            JsonObject &json = jsonBuffer.createObject();

            json["ssid"] = (String)WiFi.SSID();
            json["ip"] = WiFi.localIP().toString();
            json["mac"] = getMacAddress();
            json["version"] = (String)VERSION;
            json["testing"] = static_cast<uint8_t>(config.testmode);

            String response;
            json.printTo(response);
            client->text("G2" + response);
            break;
    }
}

void procS(uint8_t *data, AsyncWebSocketClient *client) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.parseObject(reinterpret_cast<char*>(data + 2));
    if (!json.success()) {
        LOG_PORT.println(F("*** procS(): Parse Error ***"));
        LOG_PORT.println(reinterpret_cast<char*>(data));
        return;
    }

    switch (data[1]) {
        case '1':   // Set Network Config
            dsNetworkConfig(json);
            saveConfig();
            client->text("S1");
            break;
        case '2':   // Set Device Config
            dsDeviceConfig(json);
            saveConfig();
            client->text("S2");
            break;
    }
}

/*
enum class TestMode : uint8_t {
    DISABLED,
    STATIC,
    CHASE,
    RAINBOW,
    VIEW_STREAM
};
*/

void procT(uint8_t *data, AsyncWebSocketClient *client) {
    
    switch(data[1]) {
        case '0':
            config.testmode = TestMode::DISABLED;
            //clear whole string
#if defined(ESPS_MODE_PIXEL)
          for(int y =0; y < config.channel_count; y++) pixels.setValue(y, 0);
#elif defined(ESPS_MODE_SERIAL)
          for(int y =0; y < config.channel_count; y++) serial.setValue(y, 0);
#endif
            break;

        case '1': {//static color
            config.testmode = TestMode::STATIC;
            testing.step = 0;
            DynamicJsonBuffer jsonBuffer;
            JsonObject &json = jsonBuffer.parseObject(reinterpret_cast<char*>(data + 2));

            testing.r = json["r"];
            testing.g = json["g"];
            testing.b = json["b"];
            break;
        }
        case '2': {//chase
            config.testmode = TestMode::CHASE;
            testing.step = 0;
            DynamicJsonBuffer jsonBuffer;
            JsonObject &json = jsonBuffer.parseObject(reinterpret_cast<char*>(data + 2));

            testing.r = json["r"];
            testing.g = json["g"];
            testing.b = json["b"];
        
          break;
        }
        case '3': //rainbow
            config.testmode = TestMode::RAINBOW;
            testing.step = 0;
          break;
        case '4': {//view stream
            config.testmode = TestMode::VIEW_STREAM;
#if defined(ESPS_MODE_PIXEL)
            client->binary(pixels.getData(),config.channel_count);
#elif defined(ESPS_MODE_SERIAL)
            if( config.serial_type == SerialType::DMX512)
                client->binary(&serial.getData()[1],config.channel_count);
            else 
                client->binary(&serial.getData()[2],config.channel_count);        
              
#endif
            break;
        }
        case '5': // NOISEMATRIX
            config.testmode = TestMode::NOISEMATRIX;
            LOG_PORT.println("TESTMODE NOISEMATRIX");
        break;

        case '6': // FIRE
            config.testmode = TestMode::FIRE;
            LOG_PORT.println("TESTMODE FIRE");
        break;
        case '7': // SPARKLE
            config.testmode = TestMode::SPARKLE;
            LOG_PORT.println("TESTMODE SPARKLE");                        
        break;

        case '8': // FALLBACK
            config.testmode = TestMode::FALLBACK;
            LOG_PORT.println("FALLBACK ANIMATIONS");                        
        break;        
    }
}

void handle_fw_upload(AsyncWebServerRequest *request, String filename,
        size_t index, uint8_t *data, size_t len, bool final) {
    if (!index) {
        WiFiUDP::stopAll();
        LOG_PORT.print(F("* Upload Started: "));
        LOG_PORT.println(filename.c_str());
        efupdate.begin();
    }

    if (!efupdate.process(data, len)) {
        LOG_PORT.print(F("*** UPDATE ERROR: "));
        LOG_PORT.println(String(efupdate.getError()));
    }

    if (efupdate.hasError())
        request->send(200, "text/plain", "Update Error: " +
                String(efupdate.getError()));

    if (final) {
        LOG_PORT.println(F("* Upload Finished."));
        efupdate.end();
        SPIFFS.begin();
        saveConfig();
        reboot = true;
    }
}

void wsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
        AwsEventType type, void * arg, uint8_t *data, size_t len) {
    switch (type) {
        case WS_EVT_DATA: {
            AwsFrameInfo *info = static_cast<AwsFrameInfo*>(arg);
            if (info->opcode == WS_TEXT) {
                switch (data[0]) {
                    case 'X':
                        procX(data, client);
                        break;
                    case 'E':
                        procE(data, client);
                        break;
                    case 'G':
                        procG(data, client);
                        break;
                    case 'S':
                        procS(data, client);
                        break;
                    case 'T':
                        procT(data, client);
                        break;
                }
            } else {
                LOG_PORT.println(F("-- binary message --"));
            }
            break;
        }
        case WS_EVT_CONNECT:
            LOG_PORT.print(F("* WS Connect - "));
            LOG_PORT.println(client->id());
            break;
        case WS_EVT_DISCONNECT:
            LOG_PORT.print(F("* WS Disconnect - "));
            LOG_PORT.println(client->id());
            break;
        case WS_EVT_ERROR:
            LOG_PORT.println(F("** WS ERROR **"));
            break;
    }
}

#endif /* ESPIXELSTICK_H_ */
