/*
* PixelDriver.cpp - Pixel driver code for ESPixelStick
*
* Project: ESPixelStick - An ESP8266 and E1.31 based pixel driver
* Copyright (c) 2015 Shelby Merrick
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

#include <Arduino.h>
#include <utility>
#include <algorithm>
#include "PixelDriver.h"
#include "bitbang.h"
#include <Wire.h>

extern "C" {
#include <eagle_soc.h>
#include <ets_sys.h>
#include <uart.h>
#include <uart_register.h>
}

static const uint8_t    *uart_buffer;       // Buffer tracker
static const uint8_t    *uart_buffer_tail;  // Buffer tracker
static bool             ws2811gamma;        // Gamma flag

uint8_t PixelDriver::rOffset = 0;
uint8_t PixelDriver::gOffset = 1;
uint8_t PixelDriver::bOffset = 2;

int PixelDriver::begin() {
    return begin(PixelType::WS2811, PixelColor::RGB, 170);
}

int PixelDriver::begin(PixelType type) {
    return begin(type, PixelColor::RGB, 170);
}

int PixelDriver::begin(PixelType type, PixelColor color, uint16_t length) {
    int retval = true;

    this->type = type;
    this->color = color;

    updateOrder(color);

    if (pixdata) free(pixdata);
    szBuffer = length * 3;
    if (pixdata = static_cast<uint8_t *>(malloc(szBuffer))) {
        memset(pixdata, 0, szBuffer);
        numPixels = length;
    } else {
        numPixels = 0;
        szBuffer = 0;
        retval = false;
    }

    uint16_t szAsync = szBuffer;
    if (type == PixelType::GECE) {
        if (pbuff) free(pbuff);
        if (pbuff = static_cast<uint8_t *>(malloc(GECE_PSIZE))) {
            memset(pbuff, 0, GECE_PSIZE);
        } else {
            numPixels = 0;
            szBuffer = 0;
            retval = false;
        }
        szAsync = GECE_PSIZE;
    }

    if (asyncdata) free(asyncdata);
    if (asyncdata = static_cast<uint8_t *>(malloc(szAsync))) {
        memset(asyncdata, 0, szAsync);
    } else {
        numPixels = 0;
        szBuffer = 0;
        retval = false;
    }

    if (type == PixelType::WS2811) {
        refreshTime = WS2811_TFRAME * length + WS2811_TIDLE;
        ws2811_init();
    } else if (type == PixelType::GECE) {
        refreshTime = (GECE_TFRAME + GECE_TIDLE) * length;
        gece_init();
    } else {
        retval = false;
    }

    return retval;
}

void PixelDriver::setPin(uint8_t pin) {
    if (this->pin >= 0)
        this->pin = pin;
}

void PixelDriver::setGamma(bool gamma) {
    ws2811gamma = gamma;
}

void PixelDriver::ws2811_init() {
    /* Serial rate is 4x 800KHz for WS2811 */
    Serial1.begin(3200000, SERIAL_6N1, SERIAL_TX_ONLY);
    CLEAR_PERI_REG_MASK(UART_CONF0(UART), UART_INV_MASK);
    SET_PERI_REG_MASK(UART_CONF0(UART), (BIT(22)));

    /* Clear FIFOs */
    SET_PERI_REG_MASK(UART_CONF0(UART), UART_RXFIFO_RST | UART_TXFIFO_RST);
    CLEAR_PERI_REG_MASK(UART_CONF0(UART), UART_RXFIFO_RST | UART_TXFIFO_RST);

    /* Disable all interrupts */
    ETS_UART_INTR_DISABLE();

    /* Atttach interrupt handler */
    ETS_UART_INTR_ATTACH(handleWS2811, NULL);

    /* Set TX FIFO trigger. 80 bytes gives 200 microsecs to refill the FIFO */
    WRITE_PERI_REG(UART_CONF1(UART), 80 << UART_TXFIFO_EMPTY_THRHD_S);

    /* Disable RX & TX interrupts. It is enabled by uart.c in the SDK */
    CLEAR_PERI_REG_MASK(UART_INT_ENA(UART), UART_RXFIFO_FULL_INT_ENA | UART_TXFIFO_EMPTY_INT_ENA);

    /* Clear all pending interrupts in UART1 */
    WRITE_PERI_REG(UART_INT_CLR(UART), 0xffff);

    /* Reenable interrupts */
    ETS_UART_INTR_ENABLE();
}

void PixelDriver::gece_init() {
    /* Setup for bit-banging */
    Serial1.end();
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);

    /* Setup serial log port */
    LOG_PORT.begin(115200);
}

void PixelDriver::updateOrder(PixelColor color) {
    this->color = color;

    switch (color) {
        case PixelColor::GRB:
            rOffset = 1;
            gOffset = 0;
            bOffset = 2;
            break;
        case PixelColor::BRG:
            rOffset = 1;
            gOffset = 2;
            bOffset = 0;
            break;
        case PixelColor::RBG:
            rOffset = 0;
            gOffset = 2;
            bOffset = 1;
            break;
        case PixelColor::GBR:
            rOffset = 2;
            gOffset = 0;
            bOffset = 1;
            break;
        case PixelColor::BGR:
            rOffset = 2;
            gOffset = 1;
            bOffset = 0;
            break;
        default:
            rOffset = 0;
            gOffset = 1;
            bOffset = 2;
    }
}

void ICACHE_RAM_ATTR PixelDriver::handleWS2811(void *param) {
    /* Process if UART1 */
    if (READ_PERI_REG(UART_INT_ST(UART1))) {
        // Fill the FIFO with new data
        uart_buffer = fillWS2811(uart_buffer, uart_buffer_tail);

        // Disable TX interrupt when done
        if (uart_buffer == uart_buffer_tail)
            CLEAR_PERI_REG_MASK(UART_INT_ENA(UART1), UART_TXFIFO_EMPTY_INT_ENA);

        // Clear all interrupts flags (just in case)
        WRITE_PERI_REG(UART_INT_CLR(UART1), 0xffff);
    }

    /* Clear if UART0 */
    if (READ_PERI_REG(UART_INT_ST(UART0)))
        WRITE_PERI_REG(UART_INT_CLR(UART0), 0xffff);
}

const uint8_t* ICACHE_RAM_ATTR PixelDriver::fillWS2811(const uint8_t *buff,
        const uint8_t *tail) {
    uint8_t avail = (UART_TX_FIFO_SIZE - getFifoLength()) / 4;
    if (tail - buff > avail)
        tail = buff + avail;

    if (ws2811gamma) {
        while (buff + 2 < tail) {
            uint8_t subpix = buff[rOffset];
            enqueue(LOOKUP_2811[(GAMMA_2811[subpix] >> 6) & 0x3]);
            enqueue(LOOKUP_2811[(GAMMA_2811[subpix] >> 4) & 0x3]);
            enqueue(LOOKUP_2811[(GAMMA_2811[subpix] >> 2) & 0x3]);
            enqueue(LOOKUP_2811[GAMMA_2811[subpix] & 0x3]);

            subpix = buff[gOffset];
            enqueue(LOOKUP_2811[(GAMMA_2811[subpix] >> 6) & 0x3]);
            enqueue(LOOKUP_2811[(GAMMA_2811[subpix] >> 4) & 0x3]);
            enqueue(LOOKUP_2811[(GAMMA_2811[subpix] >> 2) & 0x3]);
            enqueue(LOOKUP_2811[GAMMA_2811[subpix] & 0x3]);

            subpix = buff[bOffset];
            enqueue(LOOKUP_2811[(GAMMA_2811[subpix] >> 6) & 0x3]);
            enqueue(LOOKUP_2811[(GAMMA_2811[subpix] >> 4) & 0x3]);
            enqueue(LOOKUP_2811[(GAMMA_2811[subpix] >> 2) & 0x3]);
            enqueue(LOOKUP_2811[GAMMA_2811[subpix] & 0x3]);

            buff += 3;
        }
    } else {
        while (buff + 2 < tail) {
            uint8_t subpix = buff[rOffset];
            enqueue(LOOKUP_2811[(subpix >> 6) & 0x3]);
            enqueue(LOOKUP_2811[(subpix >> 4) & 0x3]);
            enqueue(LOOKUP_2811[(subpix >> 2) & 0x3]);
            enqueue(LOOKUP_2811[subpix & 0x3]);

            subpix = buff[gOffset];
            enqueue(LOOKUP_2811[(subpix >> 6) & 0x3]);
            enqueue(LOOKUP_2811[(subpix >> 4) & 0x3]);
            enqueue(LOOKUP_2811[(subpix >> 2) & 0x3]);
            enqueue(LOOKUP_2811[subpix & 0x3]);

            subpix = buff[bOffset];
            enqueue(LOOKUP_2811[(subpix >> 6) & 0x3]);
            enqueue(LOOKUP_2811[(subpix >> 4) & 0x3]);
            enqueue(LOOKUP_2811[(subpix >> 2) & 0x3]);
            enqueue(LOOKUP_2811[subpix & 0x3]);

            buff += 3;
        }
    }
    return buff;
}

void PixelDriver::show() {
    if (!pixdata) return;

    if (ultrasonic){
        readSonar();
        waterStairs();        
    }

    sendPeriDMX();

    if (type == PixelType::WS2811) {

      
        uart_buffer = pixdata;
        uart_buffer_tail = pixdata + szBuffer;
        SET_PERI_REG_MASK(UART_INT_ENA(1), UART_TXFIFO_EMPTY_INT_ENA);

        startTime = micros();

        // Copy the pixels to the idle buffer and swap them
        memcpy(asyncdata, pixdata, szBuffer);
        std::swap(asyncdata, pixdata);
    } else if (type == PixelType::GECE) {
         uint32_t packet = 0;

        /* Build a GECE packet */
        startTime = micros();
        for (uint8_t i = 0; i < numPixels; i++) {
            packet = (packet & ~GECE_ADDRESS_MASK) | (i << 20);
            packet = (packet & ~GECE_BRIGHTNESS_MASK) |
                    (GECE_DEFAULT_BRIGHTNESS << 12);
            packet = (packet & ~GECE_BLUE_MASK) | (pixdata[i*3+2] << 4);
            packet = (packet & ~GECE_GREEN_MASK) | pixdata[i*3+1];
            packet = (packet & ~GECE_RED_MASK) | (pixdata[i*3] >> 4);

            /* and send it */
            doGECE(pin, packet);
        }
    }
}

uint8_t* PixelDriver::getData() {
    return pixdata;
}

void PixelDriver::sendPeriDMX(){
    Wire.beginTransmission(8); // transmit to device #8
    Wire.write(peri_dimmer);              // sends one byte
    Wire.write(peri_dimmer);              // sends one byte
    // LOG_PORT.print("Sent Peri ");
    // LOG_PORT.println(peri_dimmer);
    Wire.endTransmission();    // stop transmitting
}

void PixelDriver::readSonar(){
    Wire.requestFrom(8, stairPixelData.numStairs);                 // request numStairs bytes from slave device #8
    std::vector<int> dists;
    while (Wire.available()) {                      // slave may send less than requested
        dists.push_back(Wire.read());               // receive a byte as character
        // LOG_PORT.print(dists.back());             // print the character
        // LOG_PORT.print(" "); 
    }
    // LOG_PORT.println(""); 

    if(stepData.size() == 0){   
        for(int stNr = 0; stNr < stairPixelData.numStairs; stNr++){
            stepData.push_back(stepData_t());
        }
    }

    if(stairPixelData.numStairs == dists.size()){
        for(int stNr = 0; stNr < stairPixelData.numStairs; stNr++){
            if ((dists[stNr] > 0) && (dists[stNr] < stairPixelData.triggerDist)){
                stepData[stNr].stairLinger++;  
            } else{
                stepData[stNr].stairLinger = 0;
            }

            if ((dists[stNr] > 0) && (dists[stNr] < stairPixelData.stepLength) && stepData[stNr].stairLinger == 1){
                stepData[stNr].footStepTime = millis();
                stepData[stNr].footStepDist = dists[stNr];
                LOG_PORT.print("Stair Nr: ");
                LOG_PORT.print(stNr);
                LOG_PORT.print(" trigger distance ");
                LOG_PORT.print(stepData[stNr].footStepDist);
                LOG_PORT.println(" cm");
            }
        } 
    } else {
        LOG_PORT.println("Missing sensor data..");
    }
}

void PixelDriver::setStairPixelGains(int stairNum, std::vector<float> gainArray){
    if(stairPixelData.numStairs <= stairNum)
        LOG_PORT.println("Stair request out of range");

    for(int pxNum = 0; pxNum < gainArray.size(); pxNum++){
        int ch_offset;
        if ( stairNum % 2 == 0)
            ch_offset = (stairNum * stairPixelData.numPixels + pxNum) * 3;
        else
            ch_offset = ((stairNum + 1) * stairPixelData.numPixels - pxNum) * 3;

        gainValue(ch_offset++, gainArray[pxNum]);
        gainValue(ch_offset++, gainArray[pxNum]);
        gainValue(ch_offset, gainArray[pxNum]);
    }
}

void PixelDriver::waterStairs(){
    waterAnimation.oscillationRate = 50 * (float)peri_dimmer/255;
    std::vector<float> gains;
    for(int stNr = 0; stNr < stairPixelData.numStairs; stNr++){
        if (millis() - stepData[stNr].footStepTime < waterAnimation.maxTime){
            float t = (millis() - stepData[stNr].footStepTime) / 1000.0;                                             // Time since footstep in seconds
            float p = ((float)stairPixelData.numPixels / (float)stairPixelData.stepLength) * (float)stepData[stNr].footStepDist;// foot step position in pixel
              
            for(int y = 0; y < stairPixelData.numPixels; y++) {               
                float x = (waterAnimation.dissapationRate * t * float(y - p));
                if (x == 0) x += 0.1;
                float gain = (sin(2 * M_PI * waterAnimation.oscillationRate * t) * (sin(x) / x) + 1) / 2.0;
      
                gains.push_back(gain);
            }
        }
        else {
            for (int y = 0 ; y < stairPixelData.numPixels; y++)
            {
                float gain = (sin(2 * M_PI * waterAnimation.oscillationRate * (millis() / 1000.0)) * sin(waterAnimation.waveDensity * y) + 1) / 2.0;

                gains.push_back(gain);
            }
        }
        setStairPixelGains(stNr, gains);
    }
}
