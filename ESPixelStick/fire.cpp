#include <Arduino.h>
#include <utility>
#include "fire.h"
#include <FastLED.h> 

#define LOG_PORT Serial 


void Fire::setupAnimation(uint8_t conf_cooling, uint8_t conf_sparking)
{
  this->cooling = conf_cooling;
  this->sparking = conf_sparking;
  this->currentPalette = HeatColors_p; 
}

void Fire::setEnvironment(uint16_t conf_numberof_leds, uint16_t conf_led_offset, bool conf_led_reverse)
{
  this->num_leds = conf_numberof_leds;
  this->led_offset = conf_led_offset;
  this->led_reverse = conf_led_reverse; 
  this->heat = (byte *) malloc(this->num_leds * sizeof(byte));
  this->leds = (CRGB *) malloc(this->num_leds * sizeof(CRGB));
  
  LEDS.addLeds<WS2811,5,GRB>(this->leds, this->num_leds); 
}

void Fire::updateConfig(uint8_t conf_cooling, uint8_t conf_sparking, bool conf_led_reverse)
{
  this->cooling = conf_cooling;
  this->sparking = conf_sparking;
  // this->num_leds = conf_numberof_leds; // updating this without restart means freeing memory and malloc again...
  this->led_reverse = conf_led_reverse;  
}


void Fire::getFrame()
{
  // static byte heat[NUM_LEDS];
  CRGB color;
  
  // Step 1.  Cool down every cell a little
    for( int i = 0; i < this->num_leds; i++) {
      this->heat[i] = qsub8( this->heat[i],  random8(0, ((this->cooling * 10) / this->num_leds) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= this->num_leds - 1; k >= 2; k--) {
      this->heat[k] = (this->heat[k - 1] + this->heat[k - 2] + this->heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < this->sparking ) {
      int y = random8(7);
      this->heat[y] = qadd8( this->heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < this->num_leds; j++) {
      // Scale the heat value from 0-255 down to 0-240
      // for best results with color palettes.
      
      byte colorindex = scale8( this->heat[j], 240);
      CRGB color = ColorFromPalette( this->currentPalette, colorindex);
      int pixelnumber;
      if( this->led_reverse ) {
        pixelnumber = (this->num_leds-1) - j;
      } else {
        pixelnumber = j;
      }
      this->leds[pixelnumber] = color;
    }
}

