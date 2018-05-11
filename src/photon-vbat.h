#ifndef __PHOTON_VBAT_H
#define __PHOTON_VBAT_H

#include "Particle.h"

/**
 * Class the read the value of the VBAT and the temperature sensor on the Photon.
 *
 * Also works on the Electron, however VBAT is generally connected to 3V3 so it's always 3.3V.
 *
 * This only works on the STM32F2xx devices including the Photon, Electron, and E Series.
 * It does not work on the Core or Mesh devices as it directly accesses the hardware.
 */
class PhotonVBAT {
public:
	/**
	 * Create a PhotonVBAT object. Normally you create one as a global variable.
	 *
	 * @param vbatCal is a calibration constant. 6.63 generates the correct values for me, but you may need
	 * to set it to something else.
	 *
	 * @param tempCal is the calibration constant for the ADC reading at 25.0 deg. C. That should be 943, however
	 * it can vary quite a bit from device to device.
	 *
	 * @param sparePin is a pin that you're not using that's safe to do an analogRead on. It should not be a pin
	 * that you are actually reading an analog value from, it should be some other pin.
	 */
	PhotonVBAT(int sparePin, float vbatCal = 6.63, int tempCal = 943);
	virtual ~PhotonVBAT();

	/**
	 * @brief Reads the voltage of the VBAT line, in volts.
	 */
	float readVBAT();

	/**
	 * @brief Reads the temperature of the CPU in degrees C.
	 */
	float readTempC();

	/**
	 * @brief Used internally to read the ADC value for a special channel usually ADC_Channel_16 or ADC_Channel_18.
	 */
	uint16_t readChannel(uint8_t channel);

	/**
	 * @brief Icky function to reset the ADCs back into the same state the system firmware expects them to be in.
	 */
	void restoreSystemSettings();

private:
	float vbatCal = 6.63;
	int tempCal;
	int sparePin;
};

#endif // __PHOTON_VBAT_H
