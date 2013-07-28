## Arduino Libraries

A collection (well, one for now but eventually a collection) of libraries I've written for working with the Arduino. Mostly stuff I've written to interface with specific chips that I've used.

### ina/

A class for working with [Texas Instruments' INA3221](http://www.ti.com/product/ina3221) [(datasheet)](http://www.ti.com/lit/gpn/ina3221) current and voltage monitoring IC. It doesn't cover all the capabilities of the device, only what I need in the applications I have. An example:

	#include "ina.h"

	#define MON_ADDRESS 0x40
	ina monitor(MON_ADDRESS, AVG_16, CT_332, CT_332);
	
	void setup() {
	Serial.begin(57600);
	// Initializing the INA3221 to read current and voltage on channels 1 and 3
	int err = monitor.setMode(CHAN_1 | CHAN_3, SHUNT_MEAS | BUS_MEAS);
	if (err) {
		Serial.print("Init failed with error: ");
		Serial.println(err);
		// don't do anything else
		while () {}
	}

	int validReading(int val) {
		// return 1 if getting the value from the INA was successful, 0 otherwise
		if (val == 0 || val <= -4 || val >= 4) {
			return 1;
		}
		return 0;
	}

	void loop() {
		// have the INA sample its inputs
		monitor.trigger();
		int reading = monitor.getVal(CHAN_1, SHUNT_MEAS);
		if (validReading(reading)) {
			Serial.print("Chan 1 current: ");
			// Here you would also need to scale the reading to account for
			// the current sense resistor used
			Serial.println(reading);
		}

		reading = monitor.getVal(CHAN_2, SHUNT_MEAS);
		if (validReading(reading)) {
			Serial.print("Chan 2 current: ");
			// Again, would need to scale this
			Serial.println(reading);
		}

		delay(1000);
	}

There are still a few items worth doing here:
- Implement controlling the alert and warning outputs
- Implement the continuous monitoring mode
- Some convenience functions (`validReading`, returning actual voltage and current values instead of raw readings)
- extend this to cover others in the family (maybe 209, 219 220, and 226?)
