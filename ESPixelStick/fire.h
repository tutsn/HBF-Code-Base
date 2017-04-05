#include <FastLED.h> 

class Fire{

  public:
    void setupFire(uint16_t conf_numberof_leds, uint8_t conf_cooling, uint8_t conf_sparking, bool conf_gReverseDirection);
    void goFire();
    CRGB *leds;
    uint16_t num_leds;
    
  private:
    bool gReverseDirection;
    byte *heat;
    CRGBPalette16 currentPalette;
    uint8_t cooling;
    uint8_t sparking;

};

