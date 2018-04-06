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

enum {  WAITING, COUNT_DOWN, START_GAME, PLAYING, WIN, LOSE, END_GAME };

int state = WAITING;


CRGB leds[NUM_LEDS];

CRGB player_colour[] = {CRGB::Red, CRGB::Blue};

int player_start[] = {2, 39};
int player_end[] = {0, 37};


unsigned long last_twinkle = millis();
unsigned char hue = 0;
unsigned int delay_time = 100;
unsigned char fade_rate = 100;
unsigned long last_grow = millis();

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

void movePlayer(int player_id) {
	//Increment player token
	player_start[player_id]++;
	player_end[player_id]++;

	if (player_start[player_id] == TRACK_LEN) {
		player_start[player_id] = 0;
	}

	if (player_end[player_id] == TRACK_LEN) {
		player_end[player_id] = 0;
	}

}

void drawPlayer(int pid) {
	// Serial.print(player_start[pid]);
	// Serial.print(" ");
	// Serial.print(player_end[pid]);

	for (int i = player_start[pid]; i >= player_end[pid]; i--) {
		// Serial.print("\tnorm ");
		// Serial.print(i);
		// Serial.print(" ");
		// Serial.print(TRACK[i]);
		leds[TRACK[i]] = player_colour[pid];



	}

	//check that doesn't wrap around 0
	if (player_end[pid] > player_start[pid]) {

		for (int i = TRACK_LEN - 1; i >= player_end[pid]; i--) {
			leds[TRACK[i]] = player_colour[pid];

			// Serial.print("\twrap ");
			// Serial.print(i);
			// Serial.print(" ");
			// Serial.print(TRACK[i]);

		}

		for (int i = 0; i <= player_start[pid]; i++ ) {
			leds[TRACK[i]] = player_colour[pid];
			// Serial.print("\twrap ");
			// Serial.print(i);
			// Serial.print(" ");
			// Serial.print(TRACK[i]);

		}

	}
	//Serial.println();
}


void processSerial() {
	while (Serial.available()) {
		char inChar = (char) Serial.read();
		unsigned int val;
		switch (inChar) {
		case 's':
			val = (unsigned int) Serial.parseInt();

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
			val = (unsigned int) Serial.parseInt();

			if (val < 256 && val > 1) {
				fade_rate = val;
				Serial.print("fade: ");
				Serial.println(val);
			}

			break;
		case 'd':
			FastLED.clear();
			val = (unsigned int) Serial.parseInt();

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

		case 'x':
			state = COUNT_DOWN;
			break;
		case 'q':
			movePlayer(RED_PLAYER);
			drawPlayer(RED_PLAYER);
			FastLED.show();

			break;
		case 'p':
			movePlayer(BLUE_PLAYER);
			drawPlayer(BLUE_PLAYER);
			FastLED.show();

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





// Add bit to end of players chain
void growPlayer(int player_id) {
	player_end[player_id]--;

	if (player_end[player_id] < 0) {
		player_end[player_id] = TRACK_LEN - 1;
	}

	if (player_end[player_id] == TRACK_LEN) {
		player_end[player_id] = 0;
	}


}



void resetGame() {
	//Reset player positions
	player_start[0] = 2;
	player_end[0] = 0;

	player_start[1] = 39;
	player_end[1] = 37;
	FastLED.clear();
	FastLED.show();
	fade_rate = 100;
	last_grow = millis();

}

int collision () {
	if (player_start[0] == player_end[1] ) {
		Serial.println("Red Wins");
		return 1;
	}
	if (player_start[1] == player_end[0] ) {
		Serial.println("Blue Wins");
		return 2;
	}
	return 0;
}


int matrix_row = NUM_ROWS;
int matrix_col = random(LEDS_PER_ROW);

void matrix() {

	for (int i = 0; i < NUM_LEDS; i++) {
		leds[i].nscale8(240);
	}

	if (matrix_row <= 0) {
		matrix_row = NUM_ROWS;
		matrix_col = random(LEDS_PER_ROW);
		hue += 42;
	}

	if (millis() - last_twinkle > 100) {
		last_twinkle = millis();
		matrix_row--;
		int pixel = ((LEDS_PER_ROW) * (matrix_row) ) + matrix_col;

		if (matrix_row % 2) {
			pixel  = ((LEDS_PER_ROW) * (matrix_row) ) + (LEDS_PER_ROW - 1 - matrix_col);
			//pixel += 1;
		}
		leds[NUM_LEDS - pixel - 1] = CHSV(hue, 255, 255);

		FastLED.show();

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

//Show 3 to 1 count_down
void count_down() {
	FastLED.clear();
	// FastLED.show();
	show_digit(3, 8);
	delay(1000);
	show_digit(2, 8);
	delay(1000);
	show_digit(1, 8);
	delay(1000);
	FastLED.clear();
	FastLED.show();


}





void loop() {

	switch (state) {

	case WAITING:
		fadeAll();
		matrix();
		wait(100);
		break;
	case COUNT_DOWN:
		count_down();
		state = START_GAME;
		break;

	case START_GAME:
		resetGame();
		state = PLAYING;
		break;

	case PLAYING:
		//Serial.println("here");
		wait(delay_time);
		//Serial.println("here again");

		//for(int i =0; i < 100; i++){
		drawPlayer(RED_PLAYER);
		drawPlayer(BLUE_PLAYER);
		FastLED.show();

		fadeAll();
		//FastLED.show();

		//}


		if (millis() - last_grow > 3000) {
			growPlayer(RED_PLAYER);
			growPlayer(BLUE_PLAYER);
			last_grow = millis();
			// Serial.print("grow\t");
			// Serial.print(player_start[RED_PLAYER]);
			// Serial.print(" ");
			// Serial.print(player_end[RED_PLAYER]);
			// Serial.print(" ");
			// Serial.print(player_start[RED_PLAYER] - player_end[RED_PLAYER]);
			// Serial.print("\t");
			// Serial.print(player_start[BLUE_PLAYER]);
			// Serial.print(" ");
			// Serial.print(player_end[BLUE_PLAYER]);
			// Serial.print(" ");
			// Serial.println(player_start[BLUE_PLAYER] - player_end[BLUE_PLAYER]);

			drawPlayer(RED_PLAYER);
			drawPlayer(BLUE_PLAYER);
			//Serial.println("now here");

			FastLED.show();

		}

		if (collision()) {
			state = WIN;
		}

		break;

	case WIN:
		for (int i = 0 ; i < 10; i++) {
			fadeAll();
			delay(10);
		}
		//RED Wins
		if (collision() ==  1) {
			for (int i = 0; i < NUM_LEDS; i++) {
				leds[i] = CRGB::Red;
			}

		}
		// Blue Wins
		if (collision() ==  2) {
			for (int i = 0; i < NUM_LEDS; i++) {
				leds[i] = CRGB::Blue;
			}

		}
		FastLED.show();

		for (int i = 0 ; i < 50; i++) {
			fade_rate = 240;
			fadeAll();
			delay(100);
		}

		//Show winner
		state = WAITING;
		fade_rate = 150;
		break;
	case END_GAME:

		break;
	}


}


