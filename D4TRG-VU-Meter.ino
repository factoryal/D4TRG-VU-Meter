#include "pin_definition.h"
#include "vu_meter.h"

#include <SoftwareSerial.h>

SoftwareSerial Serial1(PIN_RX, PIN_TX);


void setup() {
	cli();
	TCCR1B |= 1 << 1; // clk/8
	TIMSK1 |= 1 << TOIE1;
	sei();
	delay(1000);
	Serial1.begin(9600);
}

void loop() {
	if (sp.update()) {
		vu.push();
		fu.push();
		FastLED.show();
	}
	
}
ISR(TIMER1_OVF_vect) {
	if (Serial1.available()) {
		led_brightness_level = Serial.read();
	}
}