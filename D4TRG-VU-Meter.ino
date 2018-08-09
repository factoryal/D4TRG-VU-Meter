#include "pin_definition.h"
#include "vu_meter.h"


void setup() {
	cli();
	TCCR1B |= 1 << 1; // clk/8
	TIMSK1 |= 1 << TOIE1;
	//ADCSRA |= 1 << ADPS2;
	//ADCSRA &= ~(1 << ADPS1 | 1 << ADPS2);
	sei();
	//Serial.begin(115200);
	//Serial.println("Serial ready.");
	delay(1000);
}

void loop() {
	if (sp.update()) {
		vol.push();
		eq.push();
		FastLED.show();
	}
}

ISR(TIMER1_OVF_vect) {
	
}