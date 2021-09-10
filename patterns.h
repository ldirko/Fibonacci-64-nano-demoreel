//system procedures_____________________________________

byte XY_fibon_PLANAR(byte x, byte y) { 
  byte ledsindex = pgm_read_byte (FibonPlanarTable+y*NUM_COLS_PLANAR+x);
  return (ledsindex);
}

byte XY_CILINDR (byte x, byte y) {
  byte ledsindex = pgm_read_byte (FibonCilindrTable+y*NUM_COLS_CILINDR+x);
  return (ledsindex);
}

void GammaCorrection(){   //gamma correction function 
 byte r,g,b;
  for (uint16_t i=0; i<NUM_LEDS; i++){
    r=leds[i].r;
    g=leds[i].g;
    b=leds[i].b;
    leds[i].r = pgm_read_byte(exp_gamma + r);
    leds[i].g = pgm_read_byte(exp_gamma + g);
    leds[i].b = pgm_read_byte(exp_gamma + b);
  }
}


//_________________________ planar map patterns


//Start_____________________________________

void DrawOneFrameSpritePlanar(uint16_t xspeed, uint16_t yspeed, byte fract, byte *sprite, byte spr_cols, byte spr_rows) {
  CRGB color;
  
  uint16_t indextable = 0;
  uint16_t colsX3 = (spr_cols<<1)+spr_cols; //*3

  for (byte j = 0; j < NUM_ROWS_PLANAR; j++) {
   uint16_t yoffs = ((j + yspeed) % spr_rows)*colsX3;

    for (byte i = 0; i < NUM_COLS_PLANAR; i++) {
      uint16_t index = XY_fibon_PLANAR (i,j);
         
      if (index==lastSafeIndex) continue;
          
      uint16_t xoffs = (i + xspeed) % spr_cols;
      xoffs = (xoffs<<1)+xoffs; //*3

      byte* SpriteAdr = sprite + yoffs + xoffs;

      color.r = pgm_read_byte(SpriteAdr);
      color.g = pgm_read_byte(++SpriteAdr);
      color.b = pgm_read_byte(++SpriteAdr);

      nblend(leds[index], color, fract);
            
    }
  }
}

void StartFibo (){
  static uint16_t xspeed = 0;
  static uint16_t yspeed = 0;
  
  byte fract = 128;
  DrawOneFrameSpritePlanar (xspeed/8, yspeed/26, fract, (byte*)fibostart, 149, 9);
  xspeed++;
  // yspeed++;
  // delay(4);

};


//DigitalRain_____________________________________

void changepattern () {
  int rand1 = random16 (NUM_LEDS_PLANAR);
  int rand2 = random16 (NUM_LEDS_PLANAR);
  if (rain[rand1] && !rain[rand2]) {rain[rand1] = 0; rain[rand2] = 1;}
} //changepattern

void raininit() {     //init array of dots. run once
  for (int i = 0; i < NUM_LEDS_PLANAR; i++) rain [i] = 0;  
  byte rainNumb = random8(5,8);  //5..8 how many dots with some random variation
  for (int i = 0; i < rainNumb; i++) rain [random8(NUM_LEDS_PLANAR)] = 1;  
} //raininit

void updaterain() {
  static int speed = 1;

  for (byte j = 0; j < NUM_ROWS_PLANAR; j++) {
   int yindex=(j + speed) % NUM_ROWS_PLANAR*NUM_COLS_PLANAR;
    for (byte i = 0; i < NUM_COLS_PLANAR; i++) {
      byte layer = rain[yindex+i];   
      if (layer) leds[XY_fibon_PLANAR((NUM_COLS_PLANAR - 1) - i, (NUM_ROWS_PLANAR - 1) - j)].setHue (100);
    }
  }

  fadeToBlackBy(leds, NUM_LEDS, 80);
  speed ++;
}//updaterain

void DigitalRain() {
  if (InitNeeded) {raininit(); InitNeeded=0; FastLED.clear();}
  EVERY_N_MILLISECONDS(80) {updaterain();}
  EVERY_N_MILLISECONDS(15) {changepattern();}
} 

//DiagonalPattern_____________________________________

void DiagonalPattern() {
 uint16_t ms = millis()/8;
 int index=0;

  for (byte j = 0; j < NUM_ROWS_PLANAR; j++) { 
    for (byte i = 0; i < NUM_COLS_PLANAR; i++) {
      byte ledsindex = XY_fibon_PLANAR(i,j);
      if (ledsindex!=lastSafeIndex) leds[ledsindex].setHue((i<<3)+(j<<3)+ms); 
    }
  }
}


//fire2021_____________________________________

void fire2021 (){
  int  a = millis();
  int  a1 = a/3;
  for (byte j = 0; j < NUM_ROWS_PLANAR; j++) { 
    for (byte i = 0; i < NUM_COLS_PLANAR; i++) {
      byte ledsindex = XY_fibon_PLANAR(i,j);
      if (ledsindex!=lastSafeIndex) 
       leds[ledsindex] = HeatColor(qsub8 (inoise8 (i * 90, j * 90+a, a1), 
       abs8(j - (NUM_ROWS_PLANAR-1)) * 255 / (NUM_ROWS_PLANAR+4)));
    }
  }
}


//Distortion_Waves_plan_____________________________________

void Distortion_Waves_plan() {
  byte speed = 5; 
  uint8_t w = 2;
  uint8_t scale = 2;

  uint16_t a=millis()/24;
  uint16_t a2=a/2;
  uint16_t a3=a/3;

  uint16_t cx =  beatsin8 (10-speed,0,NUM_COLS_PLANAR)*scale;
  uint16_t cy =  beatsin8 (12-speed,0,NUM_ROWS_PLANAR)*scale;
  uint16_t cx1 = beatsin8 (13-speed,0,NUM_COLS_PLANAR)*scale;
  uint16_t cy1 = beatsin8 (15-speed,0,NUM_ROWS_PLANAR)*scale;
  uint16_t cx2 = beatsin8 (17-speed,0,NUM_COLS_PLANAR)*scale;
  uint16_t cy2 = beatsin8 (14-speed,0,NUM_ROWS_PLANAR)*scale;
  
  uint16_t xoffs=0;

  for (int x = 0; x < NUM_COLS_PLANAR; x++) {

    xoffs += scale;
    uint16_t yoffs = 0;

    for (int y = 0; y < NUM_ROWS_PLANAR; y++) {
      uint16_t index = XY_fibon_PLANAR (x, y);

      yoffs += scale;

      // byte rdistort = cos_wave [((x+y)*8+a2)&255]>>1; 
      // byte gdistort = cos_wave [((x+y)*8+a3+32)&255]>>1;
      // byte bdistort = cos_wave [((x+y)*8+a+64)&255]>>1;

      byte rdistort = cos_wave [(cos_wave[((x<<3)+a )&255]+cos_wave[((y<<3)-a2)&255]+a3   )&255]>>1; 
      byte gdistort = cos_wave [(cos_wave[((x<<3)-a2)&255]+cos_wave[((y<<3)+a3)&255]+a+32 )&255]>>1; 
      byte bdistort = cos_wave [(cos_wave[((x<<3)+a3)&255]+cos_wave[((y<<3)-a) &255]+a2+64)&255]>>1; 

      byte valueR = rdistort+ w*  (a- ( ((xoffs - cx) *  (xoffs - cx) +  (yoffs - cy) *   (yoffs - cy))>>7  ));
      byte valueG = gdistort+ w*  (a2-( ((xoffs - cx1) * (xoffs - cx1) + (yoffs - cy1) *  (yoffs - cy1))>>7 ));
      byte valueB = bdistort+ w*  (a3-( ((xoffs - cx2) * (xoffs - cx2) + (yoffs - cy2) *  (yoffs - cy2))>>7 ));

      valueR = cos_wave [valueR];
      valueG = cos_wave [valueG];
      valueB = cos_wave [valueB];
     
      leds[index].setRGB (valueR,valueG,valueB); 
    }
  }
 GammaCorrection();
}


//RGB_hiphotic_____________________________________

void RGB_hiphotic() {

 int a = millis()/6;
 for (int x = 0; x < NUM_COLS_PLANAR; x++) {
   for (int y = 0; y < NUM_ROWS_PLANAR; y++) {
     byte index = XY_fibon_PLANAR (x, y);
     leds[index].b=sin8((x-8)*cos8((y+20)*4)/4+a);
     leds[index].g=(sin8(x*16+a/3)+cos8(y*8+a/2))/2;
     leds[index].r=sin8(cos8(x*8+a/3)+sin8(y*8+a/4)+a);
    }
  }
 GammaCorrection();
}


//PlasmaBall_____________________________________

void mydrawLine(byte x1, byte y1) { // draw line frim center
  CRGB color;
  byte xsteps = abs8(NUM_COLS_PLANAR/2 - x1) + 1;
  byte ysteps = abs8(NUM_ROWS_PLANAR/2 - y1) + 1;
  byte steps = xsteps >= ysteps ? xsteps : ysteps;
  for (uint16_t i = 1; i <= steps; i++) {
    byte dx = lerp8by8(NUM_COLS_PLANAR/2, x1, i * 255 / steps);
    byte dy = lerp8by8(NUM_ROWS_PLANAR/2, y1, i * 255 / steps);
    byte index = XY_fibon_PLANAR(dx, dy);
    if(index!=lastSafeIndex) {
      color = CHSV (240-i*8, 255,255);
      // leds[index].setHue (240-i*8);
      nblend (leds[index], color, 32);
      leds[index] %= (i * 255 / steps); //for draw gradient line
    } 
  }
}

void PlasmaBall() {
  static byte hue =0;
  int8_t speed = 0;

  byte x1 = beatsin8 (18 + speed, 0, (NUM_COLS_PLANAR-1));
  byte x2 = beatsin8 (23+ speed, 0, (NUM_COLS_PLANAR-1)); 
  byte x3 = beatsin8 (27+ speed, 0, (NUM_COLS_PLANAR-1)); 
  byte x4 = beatsin8 (31+ speed, 0, (NUM_COLS_PLANAR-1)); 
  byte x5 = beatsin8 (33+ speed, 0, (NUM_COLS_PLANAR-1)); 
  byte x6 = beatsin8 (25+ speed, 0, (NUM_COLS_PLANAR-1)); 

  byte y1 = beatsin8 (20+ speed, 0, (NUM_ROWS_PLANAR-1)); 
  byte y2 = beatsin8 (26+ speed, 0, (NUM_ROWS_PLANAR-1));
  byte y3 = beatsin8 (15+ speed, 0, (NUM_ROWS_PLANAR-1));
  byte y4 = beatsin8 (27+ speed, 0, (NUM_ROWS_PLANAR-1));
  byte y5 = beatsin8 (30+ speed, 0, (NUM_ROWS_PLANAR-1));
  byte y6 = beatsin8 (19+ speed, 0, (NUM_ROWS_PLANAR-1));

  fadeToBlackBy (leds,NUM_LEDS, 15 );

  mydrawLine( x1, y1);
  mydrawLine( x2, y2);
  mydrawLine( x3, y3);
  mydrawLine( x4, y4);
  mydrawLine( x5, y5);
  mydrawLine( x6, y6);
} 


//Fire comets_____________________________________

DEFINE_GRADIENT_PALETTE(firepal) { // define fire palette
  0,  0,  0,  0, //black
  50, 0,  0,  0,
  64,  255,  0,  0, // red
  100,  255,  255,  0, //yellow
  255,  255,  255,  255, // white
};

uint16_t buffXY (byte x, byte y){
  return (y*(NUM_COLS_PLANAR+2)+x);
}

void fadecenter(){
 uint16_t sum;
  for (byte x = 1; x < NUM_COLS_PLANAR+1; x++) {
    for (byte y = 1; y < NUM_ROWS_PLANAR+1; y++) {
      sum =  rain [buffXY(x,y)]; 
      sum += rain [buffXY(x+1,y)]; 
      sum += rain [buffXY(x-1,y)];
      sum += rain [buffXY(x,y+1)]; 
      sum += rain [buffXY(x,y-1)];
      sum /=5;
      rain [buffXY(x,y)] = sum;
    }
  }
}

void balls (){
  byte speed = 15;
  byte bright = 170;  

  byte x  = beatsin8 (10+speed,2,NUM_COLS_PLANAR-1);
  byte y  = beatsin8 (15+speed,2,NUM_ROWS_PLANAR-1);
  byte x1 = beatsin8 (12+speed,2,NUM_COLS_PLANAR-1);
  byte y1 = beatsin8 (9+speed,2,NUM_ROWS_PLANAR-1);
  byte x2 = beatsin8 (15+speed,2,NUM_COLS_PLANAR-1);
  byte y2 = beatsin8 (11+speed,2,NUM_ROWS_PLANAR-1);

  rain [buffXY(x,y)]   = bright;
  rain [buffXY(x1,y1)] = bright;
  rain [buffXY(x2,y2)] = bright;
}

void toLeds (){
  CRGBPalette256 myPal = firepal;

  uint16_t buffIndex = (NUM_COLS_PLANAR+3);
  for (int y = 0; y < NUM_ROWS_PLANAR; y++) {
   for (int x = 0; x < NUM_COLS_PLANAR; x++) {
      int index = XY_fibon_PLANAR (x, (NUM_ROWS_PLANAR-1)-y);
      CRGB color = ColorFromPalette (myPal, rain [buffIndex],255);
      nblend (leds[index], color, 16);
      // leds[index] = color;
      buffIndex++;
    }
   buffIndex+=2; 
  } 
}

void FireComets() {
  if (InitNeeded) {FastLED.clear(); InitNeeded=0;}
  balls ();
  fadecenter(); 
  toLeds();
}


//_________________________ cilindrical map patterns


//Spiral_____________________________________

void Spiral () {
 uint16_t a=millis()/7;
 float scale = sin(a/32*PI/180)*27-9;  //-36..18

 float scale1 =0;
  for (int i = 0; i < NUM_COLS_CILINDR; i++) {
    for (int j = 0; j < NUM_ROWS_CILINDR; j++) {
      uint16_t index = XY_CILINDR (i,j);  
      if (index == lastSafeIndex) continue;
     
      CRGB newcolor = ColorFromPalette( gCurrentPalette, (i*255/(NUM_COLS_CILINDR-1)+j*255/(NUM_ROWS_CILINDR-1))+(int)scale1+a+sin16(a)/16384, 255);
      nblend( leds[index], newcolor, 16);
    }
    scale1 += scale;
  }

}


//RGB_Caleidoscope1_____________________________________

void RGB_Caleidoscope1() {
 uint16_t  a =millis()/8;

  for (int j = 0; j < NUM_ROWS_CILINDR; j++) {
    for (int i = 0; i < NUM_COLS_CILINDR; i++) {
      uint16_t index = XY_CILINDR(i,j);
      if (index == lastSafeIndex)  continue;

      byte valueR = (sin8(i*26+a)+cos8(j*26+a/2))/2;
      byte valueG = sin8(j*26+a/2+sin8(leds[index].r+a)/16);
      byte valueB = cos8(i*26+j*26-a/2+leds[index].g);

      leds[index].setRGB (valueR,valueG,valueB);
    }
  }
 GammaCorrection();
}


//RGB_Caleidoscope2_____________________________________

void RGB_Caleidoscope2() {
uint16_t  a = millis()/8;

for (int j = 0; j < NUM_ROWS_CILINDR; j++) {
    for (int i = 0; i < NUM_COLS_CILINDR; i++) {
      uint16_t index = XY_CILINDR(i,j);
      if (index == lastSafeIndex)  continue;

      leds[index].setRGB((sin8(i*28+a)+cos8(j*28+a))>>1, (sin8(i*28-a)+cos8(j*28+a>>1))>>1, sin8(j*26+a));
    }
  }
GammaCorrection();
}


//Distortion_Waves_cilindr_____________________________________

void Distortion_Waves_cilindr() {
  byte speed = 5; 
  uint8_t w = 1;
  uint8_t scale = 4;

  uint16_t a=millis()/24;
  uint16_t a2=a/2;
  uint16_t a3=a/3;

  uint16_t cx =  beatsin8 (10-speed,0,NUM_COLS_CILINDR)*scale;
  uint16_t cy =  beatsin8 (12-speed,0,NUM_ROWS_CILINDR)*scale;
  uint16_t cx1 = beatsin8 (13-speed,0,NUM_COLS_CILINDR)*scale;
  uint16_t cy1 = beatsin8 (15-speed,0,NUM_ROWS_CILINDR)*scale;
  uint16_t cx2 = beatsin8 (17-speed,0,NUM_COLS_CILINDR)*scale;
  uint16_t cy2 = beatsin8 (14-speed,0,NUM_ROWS_CILINDR)*scale;
  
  uint16_t xoffs=0;

  for (int x = 0; x < NUM_COLS_CILINDR; x++) {

    xoffs += scale;
    uint16_t yoffs = 0;

    for (int y = 0; y < NUM_ROWS_CILINDR; y++) {
      uint16_t index = XY_CILINDR (x, y);
      yoffs += scale;

      if (index == lastSafeIndex)  continue;

      // byte rdistort = cos_wave [((x+y)*8+a2)&255]>>1; 
      // byte gdistort = cos_wave [((x+y)*8+a3+32)&255]>>1;
      // byte bdistort = cos_wave [((x+y)*8+a+64)&255]>>1;

      byte rdistort = cos_wave [(cos_wave[((x<<3)+a )&255]+cos_wave[((y<<3)-a2)&255]+a3   )&255]>>1; 
      byte gdistort = cos_wave [(cos_wave[((x<<3)-a2)&255]+cos_wave[((y<<3)+a3)&255]+a+32 )&255]>>1; 
      byte bdistort = cos_wave [(cos_wave[((x<<3)+a3)&255]+cos_wave[((y<<3)-a) &255]+a2+64)&255]>>1; 

      byte valueR = rdistort+ w*  (a- ( ((xoffs - cx) *  (xoffs - cx) +  (yoffs - cy) *   (yoffs - cy))>>7  ));
      byte valueG = gdistort+ w*  (a2-( ((xoffs - cx1) * (xoffs - cx1) + (yoffs - cy1) *  (yoffs - cy1))>>7 ));
      byte valueB = bdistort+ w*  (a3-( ((xoffs - cx2) * (xoffs - cx2) + (yoffs - cy2) *  (yoffs - cy2))>>7 ));

      valueR = cos_wave [valueR];
      valueG = cos_wave [valueG];
      valueB = cos_wave [valueB];
     
      leds[index].setRGB (valueR,valueG,valueB); 
    }
  }

 GammaCorrection();
}


//FireButterfly_____________________________________

void FireButterfly() {
  uint16_t  a =millis()/3;
  
  for (int j = 0; j < NUM_ROWS_CILINDR; j++) {
   for (int i = 0; i < NUM_COLS_CILINDR; i++) {
     uint16_t index = XY_CILINDR(i,j);
     if (index == lastSafeIndex)  continue;

     leds[index]= HeatColor (qsub8 (inoise8 (i * 80+a , j * 5+ a , a /3), abs8(j - (NUM_ROWS_CILINDR-1)) * 255 / (NUM_ROWS_CILINDR+2)));
    }
  }
}

//Sprite_Scroll_____________________________________

void DrawOneFrameSprite(uint16_t xspeed, uint16_t yspeed, byte fract, byte *sprite, byte spr_cols, byte spr_rows) {
  CRGB color;
  
  uint16_t indextable = 0;
  uint16_t colsX3 = (spr_cols<<1)+spr_cols; //*3

  for (byte j = 0; j < NUM_ROWS_CILINDR; j++) {
   uint16_t yoffs = ((j + yspeed) % spr_rows)*colsX3;

    for (byte i = 0; i < NUM_COLS_CILINDR; i++) {
      uint16_t index = XY_CILINDR (i,j);
         
      if (index==lastSafeIndex) continue;
          
      uint16_t xoffs = (i + xspeed) % spr_cols;
      xoffs = (xoffs<<1)+xoffs; //*3

      byte* SpriteAdr = sprite + yoffs + xoffs;

      color.r = pgm_read_byte(SpriteAdr);
      color.g = pgm_read_byte(++SpriteAdr);
      color.b = pgm_read_byte(++SpriteAdr);

      nblend(leds[index], color, fract);
            
    }
  }
}

void Heart_Sprite_Scroll (){
  static uint16_t xspeed = 0;
  static uint16_t yspeed = 0;
  
  byte fract = 8;
  DrawOneFrameSprite (xspeed/26, yspeed/26, fract, (byte*)heartSprite, 28, 7);
  xspeed--;
  yspeed++;
  // delay(4);
}


//Sprite_Scrolls_____________________________________

void Sprite_Scroll_1 (){
  static uint16_t xspeed = 0;
  static uint16_t yspeed = 0;
  
  byte fract = 6;
  DrawOneFrameSprite (xspeed/26, yspeed/26, fract, (byte*)sprite1, 28, 7);
  xspeed++;
  // yspeed++;
  // delay(4);
}

void Sprite_Scroll_2 (){
  static uint16_t xspeed = 0;
  static uint16_t yspeed = 0;
  
  byte fract = 4;
  DrawOneFrameSprite (xspeed/26, yspeed/26, fract, (byte*)sprite2, 28, 7);
  xspeed++;
  // yspeed++;
  // delay(4);
}


//Swirl_____________________________________

void Swirl() {
  uint16_t  a = millis()/7;

  for (int j = 0; j < NUM_ROWS_CILINDR; j++) {
    for (int i = 0; i < NUM_COLS_CILINDR; i++) {
      uint16_t index = XY_CILINDR(i,j);
      if (index==lastSafeIndex) continue;
      // leds[index].setHue(i*54+(a>>2)+(sin8(j*16+a))>>1); 
      byte hue = i*54+(a>>2)+(sin8(j*16+a))>>1; 
      nblend (leds[index], ColorFromPalette( gCurrentPalette,hue,255),16);
    }
  } //end cycles
}


//Cilindrical_Pattern_____________________________________

void Cilindrical_Pattern() {
uint16_t  a = millis()/12;

for (int j = 0; j < NUM_ROWS_CILINDR; j++) {
   for (int i = 0; i < NUM_COLS_CILINDR; i++) {
      uint16_t index = XY_CILINDR(i,j);
      if (index==lastSafeIndex) continue;
      // leds[index].setHue ((sin8((i*28)+a)>>1)+(sin8((j*26)+a))>>1);
      byte hue = (sin8((i*28)+a)>>1)+(sin8((j*28)+a))>>1; 
      nblend (leds[index], ColorFromPalette( gCurrentPalette,hue,255),16);
    }
  }
// GammaCorrection();
}


//Spiral2_____________________________________

void Spiral2 () {
    if (InitNeeded) {raininit(); InitNeeded=0; FastLED.clear();}

 uint16_t a=millis()/6;
 float scale = sin(a/32*PI/180)*27-9;  //-36..18

 float scale1 =0;
  for (int i = 0; i < NUM_COLS_CILINDR; i++) {
    for (int j = 0; j < NUM_ROWS_CILINDR/2; j++) {
      uint16_t index = XY_CILINDR (i,j);  
      if (index == lastSafeIndex) continue;
     
      CRGB newcolor = ColorFromPalette( gCurrentPalette, (i*255/(NUM_COLS_CILINDR-1)*2+j*255/(NUM_ROWS_CILINDR-1)/8)+a+sin16(a)/16384, 255);
      nblend( leds[index], newcolor, 32);
    }
    scale1 += scale;
  }

for (int i = 0; i < NUM_COLS_CILINDR; i++) {
    for (int j = NUM_ROWS_CILINDR/2; j < NUM_ROWS_CILINDR; j++) {
      uint16_t index = XY_CILINDR (i,j);  
      if (index == lastSafeIndex) continue;
     
      CRGB newcolor = ColorFromPalette( gCurrentPalette, (i*255/(NUM_COLS_CILINDR-1)*2+j*255/(NUM_ROWS_CILINDR-1)/8)-a+sin16(a)/16384, 255);
      nblend( leds[index], newcolor, 32);
    }
    scale1 += scale;
  }
}


//Flower_____________________________________

void Flower() {
  uint16_t  a = millis()/8;

  for (int j = 0; j < NUM_ROWS_CILINDR; j++) {
    for (int i = 0; i < NUM_COLS_CILINDR; i++) {
      uint16_t index = XY_CILINDR(i,j);
      if (index==lastSafeIndex) continue;
      CRGB newcolor = ColorFromPalette( gCurrentPalette, (j*255/(NUM_ROWS_CILINDR-1)+sin8((i*8+a))+sin8(i*28-a)+a)/3, 255); //
      nblend (leds[index], newcolor, 32); 
    }
  }
}


//pride_____________________________________

void pride() 
// Pride2015
// Animated, ever-changing rainbows.
// by Mark Kriegsman
{
  static uint16_t sPseudotime = 0;
  static uint16_t sLastMillis = 0;
  static uint16_t sHue16 = 0;
 
  uint8_t sat8 = beatsin88( 87, 220, 250);
  uint8_t brightdepth = beatsin88( 341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16;//gHue * 256;
  uint16_t hueinc16 = beatsin88(113, 1, 3000);
  
  uint16_t ms = millis();
  uint16_t deltams = ms - sLastMillis ;
  sLastMillis  = ms;
  sPseudotime += deltams * msmultiplier;
  sHue16 += deltams * beatsin88( 400, 5,9);
  uint16_t brightnesstheta16 = sPseudotime;
  
  for( uint16_t i = 0 ; i < NUM_LEDS_CILINDR; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 / 256;

    brightnesstheta16  += brightnessthetainc16;
    uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);
    
    CRGB newcolor = CHSV( hue8, sat8, bri8);
    
    uint16_t pixelnumber = i;
    pixelnumber = (NUM_LEDS_CILINDR-1) - pixelnumber;
    // uint16_t ledindex = pgm_read_byte (FibonCilindrTable+pixelnumber);
    byte x = i%28; byte y = i/28;
    uint16_t ledindex = XY_CILINDR((x+y)%NUM_COLS_CILINDR, y);
    if (ledindex!=lastSafeIndex) nblend( leds[ledindex], newcolor, 64);
  }
}

void SoftTwinkles() {
  if (InitNeeded) {FastLED.clear(); InitNeeded=0;} 
// from Mark Kriegsman's great ODD/EVEN gist 
// kriegsman/SoftTwinkles.ino
// https://gist.github.com/kriegsman/99082f66a726bdff7776

  static const CRGB lightcolor(0,4,4);  
  static const CRGB darktcolor(0,2,2); 

  for( int i = 0; i < NUM_LEDS; i++) {
    if( !leds[i]) continue; // skip black pixels
    if( leds[i].b & 1) { // is red odd?
      leds[i] -= darktcolor; // darken if red is odd
    } else {
      leds[i] += lightcolor; // brighten if red is even
    }
  }
  
  int j = random16(NUM_LEDS);
  if( !leds[j] ) leds[j].b = 2;
  j = random16(NUM_LEDS);
  if (leds[j].r&1) leds[j].b-=1;
}

//array pointers to used patterns______________________

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns =     // this is list of patterns
{
 Spiral, FireComets, RGB_hiphotic, fire2021, Heart_Sprite_Scroll, 
 Flower, Swirl, Sprite_Scroll_2, pride, Spiral2, SoftTwinkles,
 Distortion_Waves_cilindr, RGB_Caleidoscope2, Sprite_Scroll_1,
 PlasmaBall, FireButterfly, Cilindrical_Pattern,
 Distortion_Waves_plan, RGB_Caleidoscope1,
 DiagonalPattern, DigitalRain
}; 
 