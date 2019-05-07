/* ***********************************************
 *This code contains the core logic of the MCU for which its being used.
 *In this case, that purpose is to cure the epoxy on a CCD Gluing chuck.
 *The cure time is 4 hours.
 * *********************************************** */

#ifndef _MCUMainPurposeLogic_
#define _MCUMainPurposeLogic_


#include "Arduino.h"
#include "HeatControlSM.hpp"
#include "Parameters.hpp"

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

void LCDOutputStatus(HeatControlSM&, Adafruit_ST7735& );
void CCDEpoxyCureLogic(HeatControlSM& );


#endif
