#define PIN_LATCH A4
#define PIN_CLOCK A5
#define PIN_DATA A3
#define PIN_L_IN A1

byte level[9] = { 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF };

void setup() {
	digitalWrite(PIN_LATCH, 1);
	pinMode(PIN_LATCH, 1);
	pinMode(PIN_CLOCK, 1);
	pinMode(PIN_DATA, 1);
}

void loop() {

	digitalWrite(PIN_LATCH, 0);
	shiftOut(PIN_DATA, PIN_CLOCK, MSBFIRST, level[map(analogRead(PIN_L_IN), 0, 511, 0, 8)]);
	digitalWrite(PIN_LATCH, 1);
}