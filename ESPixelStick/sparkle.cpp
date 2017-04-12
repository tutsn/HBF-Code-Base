#include <Arduino.h>
#include <utility>
#include "sparkle.h"
#include <FastLED.h> 

#define LOG_PORT        Serial 

void Sparkle::setupAnimation(uint16_t conf_sparkle_fps, uint8_t conf_cooling, uint8_t conf_twinkling, uint8_t conf_flicker, uint16_t conf_sparkle_bpm, uint16_t conf_sparkle_hue)
{
  this->cooling = conf_cooling;
  this->twinkling = conf_twinkling;
  this->flicker = conf_flicker;
  this->sparkle_bpm = conf_sparkle_bpm;  
  this->sparkle_fps = conf_sparkle_fps;  
  this->sparkle_hue = conf_sparkle_hue;  

  this->nextBeat = 0;
  this->nextTwinkle = 0;     
  this->loops = 0;
  this->deltaTimeTwinkle = 0;
  this->deltaTimeSparkle = 0;
  this->beatStarted = true;  
}

void Sparkle::setEnvironment(uint16_t conf_numberof_leds, uint16_t conf_led_offset, bool conf_led_reverse)
{
  this->num_leds = conf_numberof_leds;
  this->led_offset = conf_led_offset;
  this->led_reverse = conf_led_reverse;  
  this->heat = (int *) malloc(conf_numberof_leds * sizeof(int));
  this->leds = (CRGB *) malloc(conf_numberof_leds * sizeof(CRGB));
  LEDS.addLeds<WS2811,5,GRB>(this->leds, this->num_leds);   
}

void Sparkle::updateConfig(uint16_t conf_sparkle_fps, uint8_t conf_cooling, uint8_t conf_twinkling, uint8_t conf_flicker, uint16_t conf_sparkle_bpm, uint16_t conf_sparkle_hue)
{
  this->cooling = conf_cooling;
  this->twinkling = conf_twinkling;
  this->flicker = conf_flicker;
  this->sparkle_bpm = conf_sparkle_bpm;  
  this->sparkle_fps = conf_sparkle_fps;  
  this->sparkle_hue = conf_sparkle_hue;    
}

 

// ###########################################################################

CHSV Sparkle::TwinkleColor( int temperature)
{
  CHSV heatcolor_twinkle;
  heatcolor_twinkle.hue = 120;
  heatcolor_twinkle.saturation = 255;
  heatcolor_twinkle.value = temperature;
  return heatcolor_twinkle;
}

// ###########################################################################

 CRGB Sparkle::HeatColor_( uint8_t temperature)
  {
    CRGB heatcolor;

  // Scale 'heat' down from 0-255 to 0-191,
  // which can then be easily divided into three
  // equal 'thirds' of 64 units each.
  uint8_t t192 = scale8_video( temperature, 192);

  // calculate a value that ramps up from
  // zero to 255 in each 'third' of the scale.
  uint8_t heatramp = t192 & 0x3F; // 0..63
  heatramp <<= 2; // scale up to 0..252

  // now figure out which third of the spectrum we're in:
  if( t192 & 0x80) {
  // we're in the hottest third
  heatcolor.r = 255; // full red
  heatcolor.g = 255; // full green
  heatcolor.b = heatramp; // ramp up blue

  } else if( t192 & 0x40 ) {
  // we're in the middle third
  heatcolor.r = 255; // full red
  heatcolor.g = heatramp; // ramp up green
  heatcolor.b = 0; // no blue

  } else {
  // we're in the coolest third
  heatcolor.r = heatramp; // ramp up red
  heatcolor.g = 0; // no green
  heatcolor.b = 0; // no blue
  }

  return heatcolor;
  }

// ###########################################################################

void Sparkle::Twinkle()
{
  // Step 1. Create a randome number of seeds
  // random16_add_entropy( random()); //random8() isn't very random, so this mixes things up a bit
  unsigned int seeds = random16(10, this->num_leds - 10);

  // Step 2. "Cool" down every location on the strip a little
  for ( int i = 0; i < this->num_leds; i++) {
    this->heat[i] = qsub8( this->heat[i], this->cooling);
  }

  // Step 3. Make the seeds into heat on the string
  for ( int j = 0 ; j < seeds ; j++) {
    if (random16() < this->twinkling) {
      //again, we have to mix things up so the same locations don't always light up
      // random16_add_entropy( random());
      this->heat[random16(this->num_leds)] = random16(50, 255);
    }
  }

  // Step 4. Add some "flicker" to LEDs that are already lit
  //         Note: this is most visible in dim LEDs
  for ( int k = 0 ; k < this->num_leds ; k++ ) {
    if (this->heat[k] > 0 && random8() < this->flicker) {
      this->heat[k] = qadd8(this->heat[k] , 10);
    }
  }

  // Step 5. Map from heat cells to LED colors
  for ( int j = 0; j < this->num_leds; j++)
  {
    leds[j] = this->HeatColor_( this->heat[j] );
  }
  this->nextTwinkle += 1000 / this->sparkle_fps ; // assign the next time Twinkle() should happen
}

// ###########################################################################

// Sparkle works very much like Twinkle, but with more LEDs lighting up at once
void Sparkle::Sparkling() {
  // Step 1. Make a random numnber of seeds
  unsigned int seeds = random16(this->num_leds - 20 , this->num_leds);

  // Step 2. Increase the heat at those locations
  for ( int i = 0 ; i < seeds ; i++) {
    {
      int pos = random16(this->num_leds);
      // random16_add_entropy( random());
      this->heat[pos] = random8(50, 255);
    }
  }
  this->nextBeat += (60000 / this->sparkle_bpm); // assign the next time Twinkle() should happen
  this->loops++ ;
}

// ###########################################################################

void Sparkle::getFrame()
{
    if (this->loops == 0 && this->beatStarted == false) {
      this->nextBeat = millis();
      this->beatStarted == true;
      this->Sparkling();
    }
    else {
      this->deltaTimeSparkle = millis() - this->nextBeat;
      if ( this->deltaTimeSparkle > 0 ) this->Sparkling(); // if more time than
    }
  

  this->deltaTimeTwinkle = millis() - this->nextTwinkle;
  if ( this->deltaTimeTwinkle > 0 ) {
    this->Twinkle();
  }

}
