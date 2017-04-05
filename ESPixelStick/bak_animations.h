// #################### General and FastLed Configuration

#include <FastLED.h>
#include "bak_animations_palettes.h"
#define LOG_PORT        Serial  /* Serial port for console logging */
#define LED_PIN     5
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB

// #################### Palette Configuration

const TProgmemRGBGradientPalettePtr gGradientPalettes[] = 
{
  purplefly_gp,
  Fire_1_gp,
  bhw4_011_gp,
  Wild_Orange_gp,
  es_vintage_02_gp,
  ha_03_gp,
  bhw4_066_gp,
  bhw2_57_gp,
  PuOr_04_gp,
  Trouble_Ahead_gp,    
};

const uint8_t gGradientPaletteCount =  sizeof( gGradientPalettes) / sizeof( TProgmemRGBGradientPalettePtr );
CRGBPalette16 currentPalette( HeatColors_p );
CRGBPalette16 targetPalette( gGradientPalettes[0] );  


// #################### Matrix Configuration

int ledMode = 1;                  // this is the starting palette
const uint8_t kMatrixWidth  = 50;
const uint8_t kMatrixHeight = 6;
const bool    kMatrixSerpentineLayout = true;
#define NUM_LEDS (kMatrixWidth * kMatrixHeight)
#define MAX_DIMENSION ((kMatrixWidth>kMatrixHeight) ? kMatrixWidth : kMatrixHeight)
static uint16_t x;
static uint16_t y;
static uint16_t z;
uint16_t speed = 1; // speed is set dynamically once we've started up
uint16_t scale = 20; // scale is set dynamically once we've started up

// This is the array that we keep our computed noise values in
uint8_t noise[MAX_DIMENSION][MAX_DIMENSION];

uint8_t gCurrentPaletteNumber = 0;
uint8_t colorLoop = 1;


// #################### Fire Configuration

bool gReverseDirection = false;


// #################### Sparkle Configuration

#define FRAMES_PER_SECOND  30
#define COOLING            5         // controls how quickly LEDs dim
#define TWINKLING          150       // controls how many new LEDs twinkle
#define FLICKER            100        // controls how "flickery" each individual LED is


static int beatInterval =  3000;     // the interval at which you want the strip to "sparkle"
long nextBeat =            0;
long nextTwinkle =         3000;     // twinkling doesn't start until after the sanity check delay
unsigned int seeds =       0;
long loops =               0;
long deltaTimeTwinkle =    0;
long deltaTimeSparkle =    0;
boolean beatStarted =      false;

static int heat[NUM_LEDS];




// The leds

CRGB leds[kMatrixWidth * kMatrixHeight];

// functions
void fastled_setup();
void fire_setup();
void stairs_matrix_setup();
void stairs_matrix(uint16_t SecondsPerPalette);
void fillnoise8();
void mapNoiseToLEDsUsingPalette();
uint16_t XY( uint8_t x, uint8_t y);
void Fire2012WithPalette(uint8_t cooling, uint8_t sparking);

// ###########################################################################

void fastled_setup(uint16_t num_leds)
{
  LEDS.addLeds<LED_TYPE,LED_PIN,COLOR_ORDER>(leds, num_leds); 
}

// ###########################################################################



// ###########################################################################

void stairs_matrix_setup() {

  // Initialize our coordinates to some random values
  x = random16();
  y = random16();
  z = random16();

}

// ###########################################################################

void stairs_matrix(uint16_t SecondsPerPalette) {

  // Periodically choose a new palette
  
  EVERY_N_SECONDS( SecondsPerPalette ) {
    gCurrentPaletteNumber = addmod8( gCurrentPaletteNumber, 1, gGradientPaletteCount);
    targetPalette = gGradientPalettes[ gCurrentPaletteNumber ];
    LOG_PORT.println(gCurrentPaletteNumber);
  }
    nblendPaletteTowardPalette( currentPalette, targetPalette, 10);

  // generate noise data
  fillnoise8();
  
  // convert the noise data to colors in the LED array
  // using the current palette
  mapNoiseToLEDsUsingPalette();
}

// ###########################################################################

void fillnoise8() {
  // Fill the x/y array of 8-bit noise values using the inoise8 function.
 
  uint8_t dataSmoothing = 0;
  if( speed < 50) {
    dataSmoothing = 240 - (speed * 4);
  }
  
  for(int i = 0; i < MAX_DIMENSION; i++) {
    int ioffset = scale * i;
    for(int j = 0; j < MAX_DIMENSION; j++) {
      int joffset = scale * j;
      
      uint8_t data = inoise8(x + ioffset,y + joffset,z);

      // The range of the inoise8 function is roughly 16-238.
      // These two operations expand those values out to roughly 0..255
      // You can comment them out if you want the raw noise data.
      data = qsub8(data,16);
      data = qadd8(data,scale8(data,39));

      if( dataSmoothing ) {
        uint8_t olddata = noise[i][j];
        uint8_t newdata = scale8( olddata, dataSmoothing) + scale8( data, 256 - dataSmoothing);
        data = newdata;
      }
      noise[i][j] = data;
    }
  }
  z += speed;
  
  // apply slow drift to X and Y, just for visual variation.
  x += speed / 8;
  y -= speed / 16;
}

// ###########################################################################

void mapNoiseToLEDsUsingPalette()
{
  static uint8_t ihue=0;
  for(int i = 0; i < kMatrixWidth; i++) {
    for(int j = 0; j < kMatrixHeight; j++) {
      // We use the value at the (i,j) coordinate in the noise
      // array for our brightness, and the flipped value from (j,i)
      // for our pixel's index into the color palette.

      uint8_t index = noise[j][i];
      uint8_t bri =   noise[i][j];

      // if this palette is a 'loop', add a slowly-changing base value
      if( colorLoop) { 
        index += ihue;
      }

      // brighten up, as the color palette itself often contains the 
      // light/dark dynamic range desired
      if( bri > 127 ) {
        bri = 255;
      } else {
        bri = dim8_raw( bri * 2);
      }

      CRGB color = ColorFromPalette( currentPalette, index, bri);
      leds[XY(i,j)] = color;
    }
  }
  ihue+=1;
}

// ###########################################################################

uint16_t XY( uint8_t x, uint8_t y)
{
  uint16_t i;
  if( kMatrixSerpentineLayout == false) {
    i = (y * kMatrixWidth) + x;
  }
  if( kMatrixSerpentineLayout == true) {
    if( y & 0x01) {
      // Odd rows run backwards
      uint8_t reverseX = (kMatrixWidth - 1) - x;
      i = (y * kMatrixWidth) + reverseX;
    } else {
      // Even rows run forwards
      i = (y * kMatrixWidth) + x;
    }
  }
  return i;
}

// ###########################################################################

void Fire2012WithPalette(uint8_t cooling, uint8_t sparking)
{
// Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_LEDS; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((cooling * 10) / NUM_LEDS) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= NUM_LEDS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < sparking ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < NUM_LEDS; j++) {
      // Scale the heat value from 0-255 down to 0-240
      // for best results with color palettes.
      byte colorindex = scale8( heat[j], 240);
      CRGB color = ColorFromPalette( currentPalette, colorindex);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (NUM_LEDS-1) - j;
      } else {
        pixelnumber = j;
      }
      leds[pixelnumber] = color;
    }
}

// ###########################################################################

CHSV TwinkleColor( int temperature)
{
  CHSV heatcolor_twinkle;
  heatcolor_twinkle.hue = 120;
  heatcolor_twinkle.saturation = 255;
  heatcolor_twinkle.value = temperature;
  return heatcolor_twinkle;
}

// ###########################################################################

 CRGB HeatColor_( uint8_t temperature)
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

void Twinkle(uint16_t sparkle_fps, uint16_t sparkle_cooling, uint16_t sparkle_twinkle, uint16_t sparkle_flicker, uint16_t sparkle_bpm, uint16_t sparkle_hue)
{
  // Step 1. Create a randome number of seeds
  // random16_add_entropy( random()); //random8() isn't very random, so this mixes things up a bit
  seeds = random16(10, NUM_LEDS - 10);

  // Step 2. "Cool" down every location on the strip a little
  for ( int i = 0; i < NUM_LEDS; i++) {
    heat[i] = qsub8( heat[i], sparkle_cooling);
  }

  // Step 3. Make the seeds into heat on the string
  for ( int j = 0 ; j < seeds ; j++) {
    if (random16() < sparkle_twinkle) {
      //again, we have to mix things up so the same locations don't always light up
      // random16_add_entropy( random());
      heat[random16(NUM_LEDS)] = random16(50, 255);
    }
  }

  // Step 4. Add some "flicker" to LEDs that are already lit
  //         Note: this is most visible in dim LEDs
  for ( int k = 0 ; k < NUM_LEDS ; k++ ) {
    if (heat[k] > 0 && random8() < sparkle_flicker) {
      heat[k] = qadd8(heat[k] , 10);
    }
  }

  // Step 5. Map from heat cells to LED colors
  for ( int j = 0; j < NUM_LEDS; j++)
  {
    leds[j] = HeatColor_( heat[j] );
  }
  nextTwinkle += 1000 / sparkle_fps ; // assign the next time Twinkle() should happen
}

// ###########################################################################

// Sparkle works very much like Twinkle, but with more LEDs lighting up at once
void Sparkle(uint16_t sparkle_bpm) {
  // Step 1. Make a random numnber of seeds
  seeds = random16(NUM_LEDS - 20 , NUM_LEDS);

  // Step 2. Increase the heat at those locations
  for ( int i = 0 ; i < seeds ; i++) {
    {
      int pos = random16(NUM_LEDS);
      // random16_add_entropy( random());
      heat[pos] = random8(50, 255);
    }
  }
  nextBeat += (60000 / sparkle_bpm); // assign the next time Twinkle() should happen
  loops++ ;
}

// ###########################################################################

void getSparkle(uint16_t sparkle_fps, uint16_t sparkle_cooling, uint16_t sparkle_twinkle, uint16_t sparkle_flicker, uint16_t sparkle_bpm, uint16_t sparkle_hue)
{
    if (loops == 0 && beatStarted == false) {
      nextBeat = millis();
      beatStarted == true;
      Sparkle(sparkle_bpm);
    }
    else {
      long deltaTimeSparkle = millis() - nextBeat;
      if ( deltaTimeSparkle > 0 ) Sparkle(sparkle_bpm); // if more time than
    }
  

  deltaTimeTwinkle = millis() - nextTwinkle;
  if ( deltaTimeTwinkle > 0 ) {
    Twinkle(sparkle_fps, sparkle_cooling, sparkle_twinkle, sparkle_flicker, sparkle_bpm, sparkle_hue);
  }

}


