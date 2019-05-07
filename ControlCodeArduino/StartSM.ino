/* ***********************************************
 * DAMICM CCD Cryo Controller
 *
 * *********************************************** */

/*Std headers*/
#include "Arduino.h"
#include "HeatControlSM.hpp"
#include "Parameters.hpp"


#include "MCUMainPurposeLogic.hpp"

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

#define TFT_CS        10
#define TFT_RST       13 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC        12



HeatControlSM DAMICM_HCSM;
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

unsigned long previousMillis = 0;
const long runInterval = 1000;
const long check2Interval = 10000;

void setup()
{

    pinMode(THERMISTORPIN, INPUT);
    //pinMode(RELAYPIN, OUTPUT);
    tft.initR(INITR_BLACKTAB);
    tft.fillScreen(ST77XX_BLACK);

    /*PWM Frequency - lowering*/
    // PWM Set-up on pin: DAC1
    REG_PMC_PCER1 |= PMC_PCER1_PID36;                     // First enable the peripheral's clock using the Peripheral Clock Enable Register (PCER), in the case of the PWM controller, peripheral ID 36
    REG_PIOC_ABSR |= PIO_ABSR_P22;                        // Set PWM pin perhipheral type A or B, in this case B. PC22 (REG_PIOC_ABSR) Peripheral B is PWML5
    REG_PIOC_PDR |= PIO_PDR_P22;                          // Set Channel C Pin 22 as output by writing to PDR register.
    /*If DIVA or DIVB are 0 CLKA and CLKB are turned off respectively.
     *If DIVA or DIVB are 1 the respective clocks are prescaled by PREA or PREB.
     *If DIVA or DIVB are between 2-255 the CLKA and CLKB are divided by PREA or PREB then divided by DIVA or DIVB.
     *
     *The prescalers PREA and PREB are 4-bit values that range from 0 to 15 and correspond to a prescaler of 1 through to 1024. For example PWM_CLK_PREA(5) will divide the master clock (84MHz) by 32.
     */
    REG_PWM_CLK = PWM_CLK_PREA(10) | PWM_CLK_DIVA(255);      // Set the PWM clock rate to (84MHz/(1024*255) = 322 Hz)
    REG_PWM_CMR5 = PWM_CMR_CPRE_CLKA;                     // Enable single slope PWM and set the clock source as CLKA on channel 5 by selecting it on Channel Mode Register CMR5
    REG_PWM_CPRD5 = 322;                                  // Set the PWM frequency 322Hz/1Hz = 322
    REG_PWM_CDTY5 = 161;                                  // Set the PWM duty cycle 50% (322/2=161)
    REG_PWM_ENA = PWM_ENA_CHID5;                          // Enable the PWM channel



    Serial.begin(9600);
}


void loop()
{

    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= runInterval) {
        previousMillis = currentMillis;
        DAMICM_HCSM.SMEngine();
        LCDOutputStatus(DAMICM_HCSM, tft);
        CCDEpoxyCureLogic(DAMICM_HCSM);
        float currentPID = DAMICM_HCSM.getCurrentPIDValue();

        /*PWM Output
         *Out PWM limits are 0-100 but the
         *pin can take 0-322. We also have a constratint that the
         *part we are using has a response time on 30ms. This corresponds
         *to a duty cycle of 10. Which means, our controller will not work below
         *a duty cycle of 10 and above 312. I think this is acceptable.
         */
        int _apin_dutycycle = (int)currentPID * 322/100;
        REG_PWM_CDTY5 = _apin_dutycycle;

        /*DEBUG*/
        Serial.println(DAMICM_HCSM.getCurrentTemperature());
        Serial.println(DAMICM_HCSM.getCurrentPIDValue());
        Serial.println(DAMICM_HCSM.getCurrentState());
        Serial.println(_apin_dutycycle);
        Serial.print("test");

    }










}

