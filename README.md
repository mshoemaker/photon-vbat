# photon-vbat

Class the read the value of the VBAT and the temperature sensor on the Photon or P1.

Also works on the Electron, however VBAT is generally connected to 3V3 so it's always 3.3V.

This only works on the STM32F2xx devices including the Photon, Electron, and E Series.

It does not work on the Spark Core or Mesh devices as it directly accesses the hardware.

## Usage

Include the photon-vbat library and include the header file (if not already included):

```
#include "photon-vbat.h"
```

Declare a global object:

```
PhotonVBAT photonVBAT(A7, 6.54, 943);
```

Parameters are:

- A7 = An unused pin that you are not normally analogReading from that's safe to pass to analogRead
- 6.54 = VBAT calibration constant (optional)
- 943 = temperature calibration constant (optional)

Use it. For example:

```
void loop() {
	// Read battery voltage (in volts)
	float vbat = photonVBAT.readVBAT();

	// Read processor temperature (in degrees C)
	float tempC = photonVBAT.readTempC();

	float tempF = tempC * 9.0 / 5.0 + 32.0;

	Serial.printlnf("vbat=%f tempC=%f tempF=%f A0=%d", vbat, tempC, tempF, analogRead(A0));
	delay(1000);
}
```

The unused pin must be a pin that you can pass to analogRead, and should not be a pin you're actually analogReading from. In the example above it tests reading A0, so you should not pass A0 as sparePin, and you'll get invalid readings.
