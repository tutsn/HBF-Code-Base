#include <Arduino.h>
#include <utility>
#include "fire.h"
#include <FastLED.h> 

#define LOG_PORT        Serial 

void Fire::setupFire(uint16_t conf_numberof_leds, uint8_t conf_cooling, uint8_t conf_sparking, bool conf_gReverseDirection)
{
  this->cooling = conf_cooling;
  this->sparking = conf_sparking;
  this->num_leds = conf_numberof_leds;
  this->gReverseDirection = conf_gReverseDirection;
  this->heat = (byte *) malloc(conf_numberof_leds * sizeof(byte));
  this->leds = (CRGB *) malloc(conf_numberof_leds * sizeof(CRGB));

  LEDS.addLeds<WS2811,5,GRB>(this->leds, this->num_leds); 
  this->currentPalette = HeatColors_p; 
}

void Fire::updateConfig(uint8_t conf_cooling, uint8_t conf_sparking, bool conf_gReverseDirection)
{
  this->cooling = conf_cooling;
  this->sparking = conf_sparking;
  // this->num_leds = conf_numberof_leds; // updating this without restart means freeing memory and malloc again...
  this->gReverseDirection = conf_gReverseDirection;  
}


void Fire::goFire()
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
      if( this->gReverseDirection ) {
        pixelnumber = (this->num_leds-1) - j;
      } else {
        pixelnumber = j;
      }
      this->leds[pixelnumber] = color;
    }
}

