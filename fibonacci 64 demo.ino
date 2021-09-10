//Fibonacci 64 demo effects compilation
//Yaroslaw Turbin 
//https://twitter.com/ldir_ko
//https://www.reddit.com/user/ldirko/
//https://vk.com/ldirko


#include <FastLED.h>
#include "Adafruit_FreeTouch.h" //https://github.com/adafruit/Adafruit_FreeTouch

#define DATA_PIN    A10              //set your datapin
#define LED_TYPE    WS2812B          //leds type
#define COLOR_ORDER GRB              //color order of leds

#define MAX_POWER_MILLIAMPS 450      //write here your power in milliamps. default i set 800 mA for safet

#define NUM_COLS_PLANAR 9            // resolution of planar lookup table
#define NUM_ROWS_PLANAR 9            // resolution of planar lookup table
#define NUM_LEDS_PLANAR NUM_COLS_PLANAR*NUM_ROWS_PLANAR

#define NUM_COLS_CILINDR 28           // resolution of cilindrical lookup table
#define NUM_ROWS_CILINDR 7            // resolution of cinindrical lookup table
#define NUM_LEDS_CILINDR NUM_COLS_CILINDR*NUM_ROWS_CILINDR

#define NUM_LEDS 64                   
byte lastSafeIndex = 64; 

byte start = 1;                       // play start animation 
byte InitNeeded = 1;
byte automodeOn = 255;                //state of autochange pattern
uint16_t automodeOndelay = 15000;     //this delay for automode on if left\right buttons not pressed 
byte brigtness = 64;

byte button0press = 0;                //state of touch0
byte button1press = 0;                //state of touch1
byte button2press = 0;                //state of touch2

uint8_t gCurrentPatternNumber = 0;    // Index number of which pattern is current

CRGB leds [NUM_LEDS_PLANAR+1];
byte rain [(NUM_COLS_PLANAR+2)*(NUM_ROWS_PLANAR+2)];           //need for Digital Rain and Fire Comets effects

Adafruit_FreeTouch touch0 = Adafruit_FreeTouch(A0, OVERSAMPLE_4, RESISTOR_0, FREQ_MODE_NONE);
Adafruit_FreeTouch touch1 = Adafruit_FreeTouch(A1, OVERSAMPLE_4, RESISTOR_0, FREQ_MODE_NONE);
Adafruit_FreeTouch touch2 = Adafruit_FreeTouch(A2, OVERSAMPLE_4, RESISTOR_0, FREQ_MODE_NONE);

#include "palletes.h"
#include "tables.h"
#include "patterns.h"

void setup() {

  Serial.begin(115200);

  if (!touch0.begin())
    Serial.println("Failed to begin qt on pin A0");
  if (!touch1.begin())
    Serial.println("Failed to begin qt on pin A1");
  if (!touch2.begin())
    Serial.println("Failed to begin qt on pin A2");

  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, 64)
  .setCorrection( TypicalLEDStrip );
  FastLED.setMaxPowerInVoltsAndMilliamps( 5, MAX_POWER_MILLIAMPS);   
  FastLED.setBrightness(brigtness);
  FastLED.clear();
  
  random16_set_seed((touch0.measure()+touch1.measure()+touch1.measure())/3);
  gTargetPalette = ( gGradientPalettes[random8(gGradientPaletteCount)]);       // shoose random pallete on start
} 

void loop() {
  
  if (start) {StartAnimation (); start = 0;}  
  
  random16_add_entropy(random());

  if (CheckTouch0()) leftButton();
  if (CheckTouch2()) rightButton();
  if (CheckTouch1()) centerButton();
  
  CheckAutomodeON();
 
  EVERY_N_SECONDS( SECONDS_PER_PALETTE ) {   //random change palettes
    gCurrentPaletteNumber = random8 (gGradientPaletteCount);
    gTargetPalette = gGradientPalettes[ gCurrentPaletteNumber ];
  }

  EVERY_N_MILLISECONDS(40) {   //blend current palette to next palette
    nblendPaletteTowardPalette( gCurrentPalette, gTargetPalette, 16);
  }

  EVERY_N_SECONDS( 15 ) {  // speed of change patterns periodically
    if (automodeOn) {
      FadeOut (150);        // fade out current effect
      gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE(gPatterns); //next effect
      InitNeeded=1; //flag if init something need
      FadeIn (200);        // fade in current effect
    }
  }
   
  gPatterns[gCurrentPatternNumber](); //play current pattern
  FastLED.show();
  delay(4);        //some time fast call rapidly FastLED.show() on esp32 causes flicker, this delay() is easy way to fix this 
}


void StartAnimation (){
  for (int i=0; i<1150; i++) {
    StartFibo();
    FastLED.show();
    delay(4);  
  }
}

void centerButton (){
  static byte brIndex = 3;
  static byte bright[] = {0,16,32,64,96,128,160,192}; //8 steps 
  brIndex = (brIndex+1)%7;
  brigtness = bright[brIndex];
  FastLED.setBrightness(brigtness);
}

void leftButton () {
  gCurrentPatternNumber = (gCurrentPatternNumber - 1 + ARRAY_SIZE(gPatterns)) % ARRAY_SIZE(gPatterns); //next effect
  InitNeeded=1; //flag if init something need
}

void rightButton () {
  gCurrentPatternNumber = (gCurrentPatternNumber + 1 + ARRAY_SIZE(gPatterns)) % ARRAY_SIZE(gPatterns); //next effect
  InitNeeded=1; //flag if init something need
}

boolean CheckTouch0 () {   //left button change to prev pattern
  static boolean state = 0;
  static unsigned long oldtime = 0;
  uint16_t touchRaw0;

  unsigned long time = millis(); 
  if ((time-oldtime)<250) {state = 0; return(state);}
      
  oldtime = time;
  touchRaw0 = touch0.measure();
  if (touchRaw0 > 700 and !state ) state = 255;

  button0press = state;  
  return (state); 
}  

boolean CheckTouch2 () {    //right button change to next pattern
  static boolean state = 0;
  static unsigned long oldtime = 0;
  uint16_t touchRaw0;

  unsigned long time = millis(); 

  if ((time-oldtime)<250) {state = 0; return(state);}
      
  oldtime = time;
  touchRaw0 = touch2.measure();
  if (touchRaw0 > 700 and !state ) state = 255;

  button2press = state; 
  return (state); 
}

boolean CheckTouch1 () {    //center button change bright on loop with 8 steps
  static boolean state = 0;
  static unsigned long oldtime = 0;
  uint16_t touchRaw0;

  unsigned long time = millis(); 

  if ((time-oldtime)<250) {state = 0; return(state);}
      
  oldtime = time;
  touchRaw0 = touch1.measure();
  if (touchRaw0 > 700 and !state ) state = 255;

  button1press = state; 
  return (state); 
}


void CheckAutomodeON (){  //check how long buttons not pressed and turn automode ON 
  static byte state=255;
  static uint16_t timeWithoutPressButton = 1;

  if (!button2press && !button0press && !state)  {timeWithoutPressButton++;} 
  if (timeWithoutPressButton > automodeOndelay) state = 255;     
  if (button2press || button0press) {timeWithoutPressButton=0; state=0;}

  automodeOn = state;
}

void FadeOut (byte steps){
  for (int i=0; i<=steps; i++) {
    gPatterns[gCurrentPatternNumber]();
    byte fadeOut = lerp8by8 (brigtness, 0, 255*i/steps);
    FastLED.setBrightness(fadeOut);
    FastLED.show(); 
    delay(10);
  
  }
}

void FadeIn (byte steps){
  for (int i=steps+1; i--; i>=0) {
    gPatterns[gCurrentPatternNumber]();
    byte fadeOut = lerp8by8 (brigtness, 0, 255*i/steps);
    FastLED.setBrightness(fadeOut);
    FastLED.show(); 
    delay(10);

  }
}
