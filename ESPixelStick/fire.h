#include <FastLED.h> 

class Fire{

  public:
    void setupAnimation(uint8_t conf_cooling, uint8_t conf_sparking);
    void updateConfig(uint8_t conf_cooling, uint8_t conf_sparking, bool conf_led_reverse);
    void setEnvironment(uint16_t conf_numberof_leds, uint16_t conf_led_offset, bool conf_led_reverse);
    void getFrame();
    CRGB *leds;
    
    uint8_t animation_mode = 0;
    
    uint16_t num_leds;
    uint16_t led_offset;
    bool led_reverse;
    
  private:
    byte *heat;
    CRGBPalette16 currentPalette;
    uint8_t cooling;
    uint8_t sparking;

};

