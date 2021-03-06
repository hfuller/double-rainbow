#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>

#define PIN 10
//It's pin 10 and 11 on the power regulator backpack unit.
//(So set it to 10 for that.)
#define STRIPSIZE 20
int state = 0;
byte eep;

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRIPSIZE, PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(STRIPSIZE, PIN + 1, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(38400);
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  strip.begin(); strip2.begin();
  strip.setBrightness(100); strip2.setBrightness(100); // Lower brightness and save eyeballs!
  strip.show(); strip2.show(); // Initialize all pixels to 'off'
  byte eep = EEPROM.read(0);
  if ( eep == 1 ) {
    strip.setPixelColor(0, strip.Color(255, 0, 0));
    state = 1;
  } else if ( eep == 2 ) {
    strip.setPixelColor(0, strip.Color(0, 255, 0));
    state = 2;
  } else {
    strip.setPixelColor(0, strip.Color(0, 0, 255));
    state = 0;
  }
  strip.show();
  EEPROM.write(0, state + 1);
  delay(2000);
  digitalWrite(13, LOW);
  //state = 2; //for debug
}

void loop() {
  if ( state == 1 ) {
    //colorWave(75);
    runS_RAIN(75);
  } else if ( state == 2 ) {
    //rainbow(15); //BROKEN
    runS_PAPARAZZI(100);
  } else {
    //rainbowCycle(15); //BROKEN
    colorWave(75);
  }
  strip.show(); strip2.show();
}

//Custom effects begin
//Stolen from hackhuntsville on github (also partially my project tho)
void runS_RAIN(byte lowValue) {

  //fade out existing pixels
  for ( int i = 0; i < STRIPSIZE; i++ ) {
    byte x = (strip.getPixelColor(i));
    if ( x > lowValue ) {
      x -= 10;
      strip.setPixelColor(i, strip.Color(x, x, x));
    } else {
      Serial.print(x); Serial.print("<"); Serial.println(lowValue);
    }
  }
  for ( int i = 0; i < STRIPSIZE; i++ ) {
    byte x = (strip2.getPixelColor(i));
    if ( x > lowValue ) {
      x -= 10;
      strip2.setPixelColor(i, strip2.Color(x, x, x));
    } else {
      //Serial.print(x); Serial.print("<"); Serial.println(lowValue);
    }
  }
  
  //decide if we want to add a new raindrop
  if ( random(0, 3) > 1 ) {
    //we do
    strip.setPixelColor(random(0, STRIPSIZE), strip.Color(255, 255, 255));
  }
  if ( random(0, 3) > 1 ) {
    //we do
    strip2.setPixelColor(random(0, STRIPSIZE), strip2.Color(255, 255, 255));
  }

  delay(40);
}
void runS_PAPARAZZI(byte blankVal) {
  runS_BLANK(blankVal);
  //decide if we want to add a new raindrop
  if ( random(0, 100) < 99 ) {
    //we do
    int pos = random(0, STRIPSIZE);
    for ( int i=0; i<3; i++ ) {
      strip.setPixelColor(pos+i, strip.Color(255, 255, 255));
      strip2.setPixelColor(pos+i, strip2.Color(255, 255, 255));
    }
  }
  delay(random(75, 100));
}
void runS_BLANK(byte val) {
  for ( int i = 0; i < STRIPSIZE; i++ ) {
    strip.setPixelColor(i, val); strip2.setPixelColor(i, val);
  }
}
//Custom effects end

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256; j++) {
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i + j) & 255));
    }
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256 * 5; j++) { // 5 cycles of all colors on wheel
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if (WheelPos < 85) {
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
    WheelPos -= 170;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

/**
        ^   ^   ^
   ~~~~~ ColorWave ~~~~~
          V   V   V
*/
void colorWave(uint8_t wait) {
  int i, j, stripsize, cycle;
  float ang, rsin, gsin, bsin, offset;

  static int tick = 0;

  stripsize = strip.numPixels();
  cycle = stripsize * 25; // times around the circle...

  while (++tick % cycle) {
    offset = map2PI(tick);

    for (i = 0; i < stripsize; i++) {
      ang = map2PI(i) - offset;
      rsin = sin(ang);
      gsin = sin(2.0 * ang / 3.0 + map2PI(int(stripsize / 6)));
      bsin = sin(4.0 * ang / 5.0 + map2PI(int(stripsize / 3)));
      strip.setPixelColor(i, strip.Color(trigScale(rsin), trigScale(gsin), trigScale(bsin)));
      strip2.setPixelColor(i, strip.Color(trigScale(rsin), trigScale(gsin), trigScale(bsin)));
    }

    strip.show(); strip2.show();
    delay(wait);
  }

}

/**
   Scale a value returned from a trig function to a byte value.
   [-1, +1] -> [0, 254]
   Note that we ignore the possible value of 255, for efficiency,
   and because nobody will be able to differentiate between the
   brightness levels of 254 and 255.
*/
byte trigScale(float val) {
  val += 1.0; // move range to [0.0, 2.0]
  val *= 127.0; // move range to [0.0, 254.0]

  return int(val) & 255;
}

/**
   Map an integer so that [0, striplength] -> [0, 2PI]
*/
float map2PI(int i) {
  return PI * 2.0 * float(i) / float(strip.numPixels());
}
