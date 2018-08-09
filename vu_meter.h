#pragma once

/*
https://fittingmedia.wordpress.com/2013/12/18/arduino-vu-meter/
*/

#include "Arduino.h"

#include "FastLED/FastLED.h"
//#include "FFT/arduinoFFT.h"
#include "fix_fft.h"

#define PIN_VL PIN_VOLUME_LEFT
#define PIN_VR PIN_VOLUME_RIGHT
#define PIN_EL PIN_EQ_LEFT
#define PIN_ER PIN_EQ_RIGHT

#define NUM_OF_EQ_LEDS	8
#define NUM_OF_VOL_LEDS	37

#define LEFT_VOL_MID_VALUE	0 //407
#define RIGHT_VOL_MID_VALUE	0 //407

#define NUM_OF_SAMPLES 64

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
	//double samples[2][NUM_OF_SAMPLES] = { { 0 } };
	uint8_t sidx = 0;

	
	
	//arduinoFFT FFT = arduinoFFT();
	uint8_t vReal[2][NUM_OF_SAMPLES];
	uint8_t vImag[2][NUM_OF_SAMPLES];
	uint8_t vMagnitude[2][NUM_OF_SAMPLES];

	PROGMEM const uint8_t frequencyIdx[8] = { 5, 9, 14, 21, 29, 38, 48, 63 };

public:
	int vol[2] = { 0,0 };
	int peak[2] = { 0,0 };
	byte ev[2][NUM_OF_EQ_LEDS] = { { 0 } };

	SignalProcessor() {
		//ADCSRA = (1 << ADEN) | (1 << ADATE) | (1 << ADPS2) | (1 << ADPS0); // 0xA5;
	}

	// read audio input and process peak and frequency data.
	// return true if the new data ready.
	bool update() {
		// read audio signal
		cli();
		vReal[LEFT][sidx] = (analogRead(AUDIO_INPUT_LEFT) - LEFT_VOL_MID_VALUE);
		vReal[RIGHT][sidx] = (analogRead(AUDIO_INPUT_RIGHT) - RIGHT_VOL_MID_VALUE);
		
		/*ADMUX = AUDIO_INPUT_LEFT - A0;
		ADCSRA |= 1 << ADSC;
		while (!(ADCSRA & 0x10));
		vReal[LEFT][sidx] = ADCH << 6 | ADCL >> 2;
		ADMUX = AUDIO_INPUT_RIGHT - A0;
		ADCSRA |= 1 << ADSC;
		while (!(ADCSRA & 0x10));
		vReal[RIGHT][sidx] = ADCH << 6 | ADCL >> 2;*/

		vImag[LEFT][sidx] = 0;
		vImag[RIGHT][sidx] = 0;

		//for (int i = 0; i < 2; i++) {
			//if (samples[i][sidx] < 0) samples[i][sidx] = 0; // -samples[i][sidx];
			//samples[i][sidx] = (log(samples[i][sidx]) - 1) * 32;
			//if (peak[i] < samples[i][sidx]) peak[i] = samples[i][sidx];
			//peak[i] -= 1;
			//vol[i] = map(peak[i], 0, 127, 0, NUM_OF_VOL_LEDS);
		//}

		if (++sidx >= NUM_OF_SAMPLES) {
			sidx = 0;
			
			// find peak value
			peak[LEFT] -= 20; peak[RIGHT] -= 20;
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
			}
			sei();
			/*for (int i = 0; i < NUM_OF_EQ_LEDS; i++) {
				Serial.write('\t');
				Serial.print(ev[LEFT][i]);
				Serial.write(' ');
			}
			Serial.println();*/

			return true;
		}
		
		return false;
	}

	void push() {

	}
} sp;

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

	void push() {
		for (int j = 0; j < 2; j++) {
			for (int i = 0; i < NUM_OF_VOL_LEDS; i++) {
				if (i < map(sp.peak[j], 0, 255, 0, NUM_OF_VOL_LEDS))
					ch[j][i].setRGB(i >= 20 ? 255 : 0, i <= 30 ? 255 : 0, 0);
				else ch[j][i].setRGB(0, 0, 0);
			}
		}
	}
} vol;

class VU_EQ {
private:
	CRGB ch[2][NUM_OF_EQ_LEDS];
	PROGMEM const uint8_t colorSpace[NUM_OF_EQ_LEDS] = { 0, 28, 56, 84, 112, 140, 168, 196 };

public:
	VU_EQ() {
		FastLED.addLeds<WS2812B, PIN_EL>(ch[0], NUM_OF_EQ_LEDS);
		FastLED.addLeds<WS2812B, PIN_ER>(ch[1], NUM_OF_EQ_LEDS);
	}

	void push() {
		for (int j = 0; j < 2; j++) {
			for (int i = 0; i < NUM_OF_EQ_LEDS; i++) {
				ch[j][i].setHSV(colorSpace[i], 255, sp.ev[j][i] * 6);
			}
		}
	}
} eq;