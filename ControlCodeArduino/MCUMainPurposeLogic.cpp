/* ***********************************************
 *This code contains the core logic of the MCU for which its being used.
 *In this case, that purpose is to cure the epoxy on a CCD Gluing chuck.
 *The cure time is 4 hours.
 * *********************************************** */

/*Std headers*/
#include "Arduino.h"
#include "HeatControlSM.hpp"
#include "Parameters.hpp"
#include "MCUMainPurposeLogic.hpp"


void LCDOutputStatus(HeatControlSM& DAMICM_HCSM, Adafruit_ST7735& tft)
{


    tft.setCursor(0, 0);
    tft.fillScreen(ST77XX_BLACK);

    /*Tempertaure*/
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(1);
    tft.print("Temp (C): ");
    tft.println(DAMICM_HCSM.getCurrentTemperature());
    tft.print("dT/dt: ");
    tft.print(DAMICM_HCSM.getTemperatureRate()*60);
    tft.println(" C/min");
    tft.println("-----------------");

    /*State*/
    tft.print("State: ");
    tft.setTextColor(ST77XX_RED);

    int cState = DAMICM_HCSM.getCurrentState();
    switch(cState) {
    case DAMICM_HCSM.ST_Idle         :
        tft.println("Idle");
        break;
    case DAMICM_HCSM.ST_CoolDown     :
        tft.println("Cooldown");
        break;
    case DAMICM_HCSM.ST_Warmup       :
        tft.println("Warmup");
        break;
    case DAMICM_HCSM.ST_MaintainWarm :
        tft.println("MaintainWarm");
        break;

    }
    /*PID Value*/
    tft.setTextColor(ST77XX_WHITE);
    tft.print("Power: ");
    tft.setTextColor(ST77XX_MAGENTA);
    tft.print(DAMICM_HCSM.getCurrentPIDValue());
    tft.println("%");
    tft.println("-----------------");
    tft.setTextColor(ST77XX_WHITE);

    /*Present state target*/

    if (cState == DAMICM_HCSM.ST_MaintainWarm) {
        tft.print("Target: ");
        tft.setTextColor(ST77XX_GREEN);
        tft.print(SecondsToMaintainWarm/60);
        tft.println("min");
    } else if (cState == DAMICM_HCSM.ST_Idle) {
        if (millis()/1000 < SecondsBeforeHeatingStarts-1) {
            tft.print("Hello! ");
            tft.print("You have ");
            tft.setTextColor(ST77XX_YELLOW);
            tft.print(SecondsBeforeHeatingStarts - millis()/1000);
            tft.print("s ");
            tft.setTextColor(ST77XX_WHITE);
            tft.println("before the epoxy curing routine starts!");
        } else {
            tft.setTextColor(ST77XX_GREEN);
            tft.print("Operation complete, thank you.");
        }

    } else {
        tft.println("Target temp / rate: ");
        tft.setTextColor(ST77XX_GREEN);
        tft.print(DAMICM_HCSM.getTargetTemperature());
        tft.print("C / ");
        tft.print(DAMICM_HCSM.getTRateSP()*60);
        tft.println("C/min");
    }

    /*State time*/
    tft.println("-----------------");
    tft.setTextColor(ST77XX_WHITE);
    tft.print("State time: ");
    tft.setTextColor(ST77XX_ORANGE);
    tft.println((millis()-DAMICM_HCSM.StateTime) / 1000);
    /*Uptime*/
    tft.setTextColor(ST77XX_WHITE);
    tft.print("Uptime (s): ");
    tft.setTextColor(ST77XX_ORANGE);
    tft.println(millis() / 1000);
    /*Closing*/
    tft.println("-----------------");
    tft.println("Send feedback to: ");
    tft.println("Pitam Mitra");
    tft.println("pitamm@uw.edu");

}

void CCDEpoxyCureLogic(HeatControlSM& DAMICM_HCSM)
{

    /*Give it 2 minutes to start heating*/
    if (millis() / 1000 > SecondsBeforeHeatingStarts &&
            millis() / 1000 < (SecondsBeforeHeatingStarts+5) )
        DAMICM_HCSM.SetTemperature=90;

    /*Once Maintainwarm is reached, wait 4 hours before resetting temperature*/
    if (DAMICM_HCSM.getCurrentState() == DAMICM_HCSM.ST_MaintainWarm &&
            (millis()-DAMICM_HCSM.StateTime) > SecondsToMaintainWarm*1000  )
        DAMICM_HCSM.SetTemperature=20;


}

