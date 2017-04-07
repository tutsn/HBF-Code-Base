#include <FastLED.h> 

class Sparkle{

  public:
    void setupSparkle(uint16_t conf_numberof_leds, uint16_t conf_sparkle_fps, uint8_t conf_cooling, uint8_t conf_twinkling, uint8_t conf_flicker, uint16_t conf_sparkle_bpm, uint16_t conf_sparkle_hue);
    void updateConfig(uint16_t conf_sparkle_fps, uint8_t conf_cooling, uint8_t conf_twinkling, uint8_t conf_flicker, uint16_t conf_sparkle_bpm, uint16_t conf_sparkle_hue);
    void goSparkle();

    CRGB *leds;
    uint16_t num_leds;

  private:
    CHSV TwinkleColor(int temperature);
    CRGB HeatColor_(uint8_t temperature);
    void Twinkle();
    void Sparkling();

    int *heat;
    uint16_t sparkle_bpm;
    uint16_t sparkle_fps;
    uint16_t sparkle_hue;
    uint8_t cooling;
    uint8_t twinkling;
    uint8_t flicker;
    
    long nextBeat;
    long nextTwinkle;
    long loops;
    long deltaTimeTwinkle;
    long deltaTimeSparkle;
    boolean beatStarted;    
    
};

