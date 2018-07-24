#include "FastLED/FastLED.h"

CRGB leds[8];

enum {
	LEFT = 0, RIGHT
};

class VU_LR {
private:
	uint8_t pin;

public:
	VU_LR(uint8_t ledPin) {

	}

};

class VU_EQ {
private:
	uint8_t pin;

public:
	VU_EQ(uint8_t ledPin) {

	}


	void update() {

	}

};

VU_EQ vu_eq[2] = { VU_EQ(A0), VU_EQ(A1)};

void setup() {
	FastLED.addLeds<WS2812B, A0>(leds, 8);
}

void loop() {
	for (int i = 0; i < 8; i++) {
		leds[i].setRGB(0, 0, 255);
		FastLED.show();
		delay(100);
		leds[i].setRGB(1, 1, 1);
	}
}