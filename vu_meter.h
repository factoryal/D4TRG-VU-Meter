#pragma once

/*
https://fittingmedia.wordpress.com/2013/12/18/arduino-vu-meter/
*/

#include "Arduino.h"

#include "FastLED/FastLED.h"

#define PIN_VL PIN_VOLUME_LEFT
#define PIN_VR PIN_VOLUME_RIGHT
#define PIN_EL PIN_EQ_LEFT
#define PIN_ER PIN_EQ_RIGHT

#define NUM_OF_EQ_LEDS	8
#define NUM_OF_VOL_LEDS	37

enum {
	LEFT = 0, RIGHT
};

enum {
	VOL = 0, EQ
};

class SignalProcessor {
private:

};

class VU_VOL {
private:
	CRGB ch[2][NUM_OF_VOL_LEDS];

public:
	VU_VOL() {
		FastLED.addLeds<WS2812B, PIN_VL>(ch[0], NUM_OF_VOL_LEDS);
		FastLED.addLeds<WS2812B, PIN_VR>(ch[1], NUM_OF_VOL_LEDS);
	}

	void update() {
		
	}

	void debug() {
		static int oldRead = 0;
		static const int theta = 0.8;
		static int peak = 0;
		int nowRead = analogRead(A0)-407;
		//Serial.println(nowRead);
		if (nowRead < 0) nowRead = -nowRead;
		nowRead = log((nowRead)) * 32-32;
		if (peak < nowRead) peak = nowRead;
		peak -= 2;
		//peak = nowRead;
		int v = map(peak, 0, 127, 0, NUM_OF_VOL_LEDS-1);
		for (int i = 0; i < NUM_OF_VOL_LEDS; i++) {
			if(i<v) ch[LEFT][i].setRGB(i >= 20 ? 255 : 0, i <= 30 ? 255 : 0, 0);
			else ch[LEFT][i].setRGB(0, 0, 0);
		}
		FastLED.show();
		oldRead = nowRead;
		
	}
};