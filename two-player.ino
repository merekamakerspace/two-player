#include "FastLED.h"
#include "digits.h"
#include "track.h"

// How many leds in your strip?
#define NUM_LEDS_1 150
#define NUM_LEDS_2 98

#define NUM_LEDS NUM_LEDS_1 + NUM_LEDS_2

#define LEDS_PER_ROW 8
#define NUM_ROWS 31


// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
#define DATA_PIN_1 7
#define DATA_PIN_2 8



#define MAX_ENERGY 500

enum {RED_PLAYER, BLUE_PLAYER};



CRGB leds[NUM_LEDS];

CRGB player_colour[] = {CRGB::Red, CRGB::Blue};

int player_start[] = {3, 40};
int player_end[] = {0, 37};


unsigned long last_run = millis();
unsigned char hue = 0;
unsigned int delay_time = 100;
unsigned char fade_rate = 240;

void fadeAll()  {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].nscale8(fade_rate);

  }
  FastLED.show();
}

void light_row(int row, CRGB colour) {

  if (row > NUM_ROWS) {
    return;
  }

  for (int i = 0; i <  LEDS_PER_ROW; i++) {
    int pixel = (LEDS_PER_ROW * row) + i;
    leds[NUM_LEDS - pixel - 1] = colour;
  }

}


void show_digit(int digit, int start_row) {
  //FastLED.clear();

  for (int row = 0; row < 8; row++) {
    if (start_row + row >= 0 ) {
      byte line = DIGITS[digit][row];

      //Serial.println(line);
      int col = 0;
      for (byte mask = 00000001; mask > 0; mask <<= 1) { //iterate through bit mask
        //int pixel = ((start_row + row) * 8) +  col;
        int pixel  = ((start_row + row) * 8) + (7 - col);

        if ((start_row + row) % 2) {
          //pixel  = ((start_row + row) * 8) + (7 - col);
          pixel = ((start_row + row) * 8) +  col;//pixel += 1;
        }

        if (line & mask) {
          leds[pixel] = CRGB::Red;
          //Serial.print("*");
        } else {
          //Serial.print(" ");
          leds[pixel] = CRGB::Black;
        }
        //Serial.println(pixel);

        col++;
      }
    }
    //Serial.println();
  }
  FastLED.show();
}


void processSerial() {
  while (Serial.available()) {
    char inChar = (char) Serial.read();
    unsigned int val = (unsigned int) Serial.parseInt();

    switch (inChar) {
    case 's':
      Serial.print("s");
      if (val > 1) {
        delay_time = val;
        Serial.print("delay: ");
        Serial.println(val);
      }

      // get the new byte:
      break;
    case 'f':
      Serial.print("f");
      if (val < 256 && val > 1) {
        fade_rate = val;
        Serial.print("fade: ");
        Serial.println(val);
      }

      break;
    case 'd':
      FastLED.clear();

      if (val == 10) {
        Serial.println(0);
        FastLED.clear();
        show_digit(0, 8);
      }

      if (val > 0 && val < 10) {
        Serial.println(val);
        show_digit(val, 8);
      }

      break;
    }



  }

}


void serialEvent() {
  processSerial();

}

void testScreen() {
  for (int i = 0; i < NUM_ROWS; i++) {
    hue = map(i, 0, NUM_ROWS, 0, 160);
    light_row(i, CHSV(hue, 255, 255));

  }
  FastLED.show();

  //unsigned long start_time = millis();
  while (leds[NUM_LEDS - 3].r > 0 ) {
    fadeAll();
    delay(30);
  }

}


void drawPixel(int x, int y, CRGB color) {

  int pixel = ((LEDS_PER_ROW) * (y) ) + x;

  if (y % 2) {
    pixel  = ((LEDS_PER_ROW) * (y) ) + (LEDS_PER_ROW - 1 - x);
    //pixel += 1;
  }

  leds[pixel] = color;

}



void movePlayer(int player_id) {
  //Increment player token
  player_start[player_id]++;
  player_end[player_id]++;

  if (player_start[player_id] >= TRACK_LEN) {
    player_start[player_id] = 0;
  }

  if (player_end[player_id] >= TRACK_LEN) {
    player_end[player_id] = 0;
  }

}

// Add bit to end of players chain
void growPlayer(int player_id) {
  player_end[player_id]++;

  if (player_end[player_id] >= TRACK_LEN) {
    player_end[player_id] = 0;
  }

}



/* Checks for collision between players
 returns 1 if player 1 wins
 returns 1 if player 2 wins
 returns 0 for no collision
*/
int collision () {
  if (player_start[0] == player_end[1] ) {
    return 1;
  }
  if (player_start[1] == player_end[0] ) {
    return 2;
  }
  return 0;
}

void drawPlayer(int pid) {
  for (int i = player_start[pid]; i < player_end[pid]; i--) {
    //set led to track
    leds[TRACK[i]] = player_colour[pid];

  }

  //check that doesn't wrap around 0
  if (player_end[pid] > player_start[pid]) {
    for (int i = TRACK_LEN - 1; i < player_end[pid]; i--) {
      leds[TRACK[i]] = player_colour[pid];
      
    }

  }

}


void setup() {


  Serial.begin(115200);
  Serial.println("ON");

  //FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.addLeds<WS2812, DATA_PIN_1, RGB>(leds, 0, NUM_LEDS_1);
  FastLED.addLeds<WS2812, DATA_PIN_2, RGB>(leds, NUM_LEDS_1, NUM_LEDS_2);

  //testScreen();
  FastLED.clear();
  FastLED.show();

}


void colours() {
  hue += 1;
  CRGB col = CHSV(hue, 255, 255);
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = col;
  }
  FastLED.show();
}

void wait(unsigned int dt) {
  unsigned long start_time = millis();
  while (millis() - start_time <  dt) {
    processSerial();
    delay(1);
  }
}




void loop() {
  drawPlayer(RED_PLAYER);
  drawPlayer(BLUE_PLAYER);
  wait(100);
  fadeAll();
  movePlayer(RED_PLAYER);
  movePlayer(BLUE_PLAYER);


}


