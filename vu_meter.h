#pragma once

/*
https://fittingmedia.wordpress.com/2013/12/18/arduino-vu-meter/
*/

#include "Arduino.h"

#include "FastLED/FastLED.h"
#include "fix_fft.h"

#include "AnalogReadFast.h"

#define PIN_VL PIN_VU_LEFT
#define PIN_VR PIN_VU_RIGHT
#define PIN_EL PIN_FU_LEFT
#define PIN_ER PIN_FU_RIGHT

#define NUM_OF_EQ_LEDS	8
#define NUM_OF_VOL_LEDS	37

#define LEFT_VOL_MID_VALUE	0 //407
#define RIGHT_VOL_MID_VALUE	0 //407

#define NUM_OF_SAMPLES 128
#define PEAK_VOLUME_LINEAR_DECREASE_AMOUNT 5
#define PEAK_EQ_LINEAR_DECREASE_AMOUNT 1

static byte led_brightness_level = 4;

enum {
	LEFT = 0, RIGHT
};

enum {
	VOL = 0, EQ
};

class SignalProcessor {
private:
	int oldRead[2] = { 0,0 };
	int nowRead[2] = { 0,0 };
	uint8_t sidx = 0;
	
	uint8_t vReal[2][NUM_OF_SAMPLES];
	uint8_t vImag[2][NUM_OF_SAMPLES];
	uint8_t vMagnitude[2][NUM_OF_SAMPLES];

	PROGMEM const uint8_t frequencyIdx[8] = { 8, 16, 24, 32, 40, 48, 56, 64 };

public:
	int vol[2] = { 0, 0 };
	int peak[2] = { 0, 0 };
	int ev[2][NUM_OF_EQ_LEDS] = { { 0 } };
	int evpeak[2][NUM_OF_EQ_LEDS] = { { 0 } };

	SignalProcessor() {
		//ADCSRA = (1 << ADEN) | (1 << ADATE) | (1 << ADPS2) | (1 << ADPS0); // 0xA5;
		pinMode(PIN_DIAG_1, 1);
	}

	// read audio input and process peak and frequency data.
	// return true if the new data ready.
	bool update() {
		vReal[LEFT][sidx] = (analogReadFast(AUDIO_INPUT_LEFT) - LEFT_VOL_MID_VALUE);
		vReal[RIGHT][sidx] = (analogReadFast(AUDIO_INPUT_RIGHT) - RIGHT_VOL_MID_VALUE);
		

		vImag[LEFT][sidx] = 0;
		vImag[RIGHT][sidx] = 0;


		if (++sidx >= NUM_OF_SAMPLES) {
			digitalWrite(PIN_DIAG_1, 1);
			sidx = 0;

			// find peak value
			peak[LEFT] = 0;
			peak[RIGHT] = 0;
			/*peak[LEFT] -= PEAK_VOLUME_LINEAR_DECREASE_AMOUNT;
			peak[RIGHT] -= PEAK_VOLUME_LINEAR_DECREASE_AMOUNT;*/

			for (int i = 0; i < NUM_OF_SAMPLES; i++) {
				if (peak[LEFT] < vReal[LEFT][i]) peak[LEFT] = vReal[LEFT][i];
				if (peak[RIGHT] < vReal[RIGHT][i]) peak[RIGHT] = vReal[RIGHT][i];
			}


			// frequency analysis
			for (int i = 0; i < 2; i++) {
				fix_fft((char*)vReal[i], (char*)vImag[i], 6, 0);
			}

			for (int i = 0; i < 2; i++) {
				for (int j = 0; j < NUM_OF_SAMPLES; j++) {
					vMagnitude[i][j] = sqrt(vReal[i][j] * vReal[i][j] + vImag[i][j] * vImag[i][j]);
				}
			}


			memset(ev, 0, sizeof(ev));
			for (int i = 0, j = 0; i < NUM_OF_EQ_LEDS; i++) {
				do {
					ev[LEFT][i] += vMagnitude[LEFT][j];
					ev[RIGHT][i] += vMagnitude[RIGHT][j];
				} while (++j < frequencyIdx[i]);

				evpeak[LEFT][i] -= PEAK_EQ_LINEAR_DECREASE_AMOUNT;
				evpeak[RIGHT][i] -= PEAK_EQ_LINEAR_DECREASE_AMOUNT;
				if (evpeak[LEFT][i] < ev[LEFT][i]) evpeak[LEFT][i] = ev[LEFT][i];
				if (evpeak[RIGHT][i] < ev[RIGHT][i]) evpeak[RIGHT][i] = ev[RIGHT][i];
				
			}
			digitalWrite(PIN_DIAG_1, 0);

			return true;
		}
		
		return false;
	}
} sp;

class VU_VOL {
private:
	CRGB ch[2][NUM_OF_VOL_LEDS];
	const byte brightness[9] = { 0, 31, 63, 95, 127, 159, 191, 223, 255 };

public:
	VU_VOL() {
		FastLED.addLeds<WS2812B, PIN_VL>(ch[0], NUM_OF_VOL_LEDS);
		FastLED.addLeds<WS2812B, PIN_VR>(ch[1], NUM_OF_VOL_LEDS);
	}

	void push() {
		static int p[2];
		for (int j = 0; j < 2; j++) {
			int s = 30 * log(sp.peak[j]);
			if (p[j] < s) p[j] = s;
			else p[j] -= PEAK_VOLUME_LINEAR_DECREASE_AMOUNT;

			int v = map(p[j], 0, 127, 0, NUM_OF_VOL_LEDS);
			for (int i = 0; i < NUM_OF_VOL_LEDS; i++) {
				if (i < v)
					ch[j][i].setRGB(i >= 20 ? brightness[led_brightness_level] : 0, i <= 30 ? brightness[led_brightness_level] : 0, 0);
				else ch[j][i].setRGB(0, 0, 0);
			}
		}
	}
} vu;

class VU_FU {
private:
	CRGB ch[2][NUM_OF_EQ_LEDS];
	PROGMEM const uint8_t colorSpace[NUM_OF_EQ_LEDS] = { 0, 28, 56, 84, 112, 140, 168, 196 };

public:
	VU_FU() {
		FastLED.addLeds<WS2812B, PIN_EL>(ch[0], NUM_OF_EQ_LEDS);
		FastLED.addLeds<WS2812B, PIN_ER>(ch[1], NUM_OF_EQ_LEDS);
	}

	void push() {
		for (int j = 0; j < 2; j++) {
			for (int i = 0; i < NUM_OF_EQ_LEDS; i++) {
				ch[j][i].setHSV(colorSpace[i], 255, sp.evpeak[j][i] * 3 * led_brightness_level);
			}
		}
	}
} fu;