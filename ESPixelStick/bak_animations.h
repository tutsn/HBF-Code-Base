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




