#include "pin_definition.h"
#include "vu_meter.h"


VU_VOL vol;


void setup() {
	cli();
	TCCR1B |= 1 << 1; // clk/8
	TIMSK1 |= 1 << TOIE1;
	sei();
	Serial.begin(115200);
	
}

void loop() {
	vol.debug();
	vol.update();
}

ISR(TIMER1_OVF_vect) {
	
}