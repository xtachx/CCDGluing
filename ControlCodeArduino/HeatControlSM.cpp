//
//  CryoControlSM.cpp
//  CC_R
//
//  Created by Pitam Mitra on 1/6/19.
//  Copyright © 2019 Pitam Mitra. All rights reserved.
//
#include "Arduino.h"
#include <vector>
//#include <iostream>
//#include <iomanip>
//#include <unistd.h>
#include <cmath>

#include "HeatControlSM.hpp"
#include "PID_v1.h"
#include "Parameters.hpp"


HeatControlSM::HeatControlSM(void){



    /*Current starting state for FSM is idle. Should be state is also idle*/
    CurrentFSMState = ST_Idle;
    ShouldBeFSMState = ST_Idle;


    CurrentTemperature=0;
    LastTemperature=0;
    //nowtime=millis();

    /*Kp Ki Kd changes */
    this->AbsPID->SetTunings(KpA, KiA, KdA);
    this->RatePID->SetTunings(KpR, KiR, KdR);


    /*The two PID implementations*/
    this->AbsPID = new PID(&CurrentTemperature, &TOutput, &SetTemperature, KpA, KiA, KdA, P_ON_M, DIRECT);
    this->RatePID = new PID(&TempratureRateMovingAvg, &ROutput, &RSetpoint, KpR, KiR, KdR, P_ON_M, DIRECT);
    this->AbsPID->SetOutputLimits(0,100);
    this->RatePID->SetOutputLimits(0,100);

}

HeatControlSM::~HeatControlSM(void){
    delete this->AbsPID;
    delete this->RatePID;
}

void HeatControlSM::SMEngine(void ){

    /*First, run the interaction with the SQL server with updates
     *and fresh changes*/
    this->UpdateVars();

    /*There is no chance of a Kp, Ki Kd or temperature changes.
     *Temperature set point - handle this change via the loop function*/

    /*Now update the current and the last temperature. Also update the rate of change of temperature with the new information.*/
    this->LastTemperature = this->CurrentTemperature;

    /*If you have multiple sources of temperature, use this line to decide which one (or comb.) to use.*/
    this->CurrentTemperature += (this->_thisRunCurTemp-this->CurrentTemperature)/MovingAvgN;

    /*Calculate the rate moving avg*/
    if (this->LastTemperature !=0 ) this->TempratureRateMovingAvg += (this->CurrentTemperature-this->LastTemperature)/RateMovingAvgN - this->TempratureRateMovingAvg/RateMovingAvgN;


    /*Decide what state the system should be in. Then run the function to switch state if needed.*/
    this->StateDecision();
    if (this->CurrentFSMState != this->ShouldBeFSMState) this->StateSwitch();

    /*Finally, run the state function. Note: This probably should be run between decision and switch if one wants to use exit guards.
     *A hash table is not available for the arduino. Hence we have to use a jump table manually defined here.*/

    switch(this->ShouldBeFSMState){
        case ST_Idle         : this->Idle();
                               break;
        case ST_CoolDown     : this->CoolDown();
                               break;
        case ST_Warmup       : this->Warmup();
                               break;
        case ST_MaintainWarm : this->MaintainWarm();
                               break;
    }

}


void HeatControlSM::StateDecision(void ){

    /*If the SM is turned off (manual mode), then the state should be idle and no output is produced.*/
    if (this->FSMMode == MANUAL) {
        this->ShouldBeFSMState=ST_Idle;
        return;
    }

    /*Should never happen conditions*/
    if (this->CurrentTemperature > 90 && this->SetTemperature <90)
        this->ShouldBeFSMState=ST_Idle; //this should never happen in practice. If it does, then idle.

    /*Warmup State conditions - while not in MaintainWarm*/
    if (this->SetTemperature > this->CurrentTemperature + 2 &&
        this->CurrentTemperature < 90 && this->CurrentFSMState != ST_MaintainWarm) this->ShouldBeFSMState=ST_Warmup;
    /*Warmup while in maintainwarm*/
    if (this->SetTemperature > this->CurrentTemperature + 10 &&
        this->CurrentTemperature < 90 && this->CurrentFSMState == ST_MaintainWarm) this->ShouldBeFSMState=ST_Warmup;


    /*Cooldown while the current temperature is low - i.e. <220 K*/
    if (this->SetTemperature < this->CurrentTemperature - 2 &&
        this->CurrentTemperature >= 30) this->ShouldBeFSMState=ST_CoolDown;

    /*Maintain a warm state once the temperature is within 2 K of set point*/
    if (this->SetTemperature > 50  &&
        fabs(this->SetTemperature - this->CurrentTemperature) <= 2 && this->CurrentFSMState != ST_MaintainWarm)
        this->ShouldBeFSMState=ST_MaintainWarm;


}



void HeatControlSM::StateSwitch(void ){

    /*Activate entry guard*/
    this->EntryGuardActive=true;
    this->StateTime = millis();

    /*Switch the state of the machine*/
    this->CurrentFSMState = this->ShouldBeFSMState;
}




void HeatControlSM::Warmup(void){

    /*Entry guard function: Activate rate PID. Set the rate target for RatePID.
     *Turn off cryocooler.
     */
    if (this->EntryGuardActive){

        /*Activate the correct PID*/
        this->AbsPID->SetMode(MANUAL);
        this->RatePID->SetMode(AUTOMATIC);

        /*Turn off the cryocooler power control feature and put cryocooler to min power*/
        this->RatePID->SetOutputLimits(0,255);

        /*Set the correct rate direction for the rate*/
        this->RSetpoint = DeltaTRatePerMin/60.0;

        /*Guard done*/
        this->EntryGuardActive = false;
    }

    /*Calculate Rate PID*/
    this->RatePID->Compute();
    this->ThisRunPIDValue = this->ROutput;

}


void HeatControlSM::Idle(void){


    /*Entry guard function: Deactivate all PIDs.
     *Turn off cryocooler.
     */
    if (this->EntryGuardActive){

        this->AbsPID->SetMode(MANUAL);
        this->RatePID->SetMode(MANUAL);

        /*Turn off the cryocooler power control feature and put cryocooler to min power*/
        this->RatePID->SetOutputLimits(0,255);


        this->EntryGuardActive = false;
    }

    /*System is idle - so heater should be OFF*/
    this->ThisRunPIDValue = 0.0;

}


/*Note: CooldownHot possibly requires
 *PID limits to be overriden
 *since 75% power seems to be too little
 *to get the rate at <5 / min
 */

void HeatControlSM::CoolDown(void ){


    /*Entry guard function: Activate rate PID. Set the rate target for RatePID.
     *Turn on cryocooler.
     */
    if (this->EntryGuardActive){

        /*Activate the correct PID*/
        this->AbsPID->SetMode(MANUAL);
        this->RatePID->SetMode(AUTOMATIC);

        /*Set the correct rate direction for the rate*/
        this->RSetpoint = -1.0*DeltaTRatePerMin/60.0; // degrees per sec


        this->EntryGuardActive = false;
    }

    /*Calculate Rate PID*/
    this->RatePID->Compute();
    this->ThisRunPIDValue = this->ROutput;


}


void HeatControlSM::MaintainWarm(void){


    /*Entry guard function: Activate AbsPID.
     *Turn off cryocooler.
     */
    if (this->EntryGuardActive){

        /*Activate the correct PID*/
        this->AbsPID->SetMode(AUTOMATIC);
        this->RatePID->SetMode(MANUAL);

        this->EntryGuardActive = false;
    }


    /*Calculate PID*/
    this->AbsPID->Compute();
    this->ThisRunPIDValue = this->TOutput;


}


void HeatControlSM::UpdateVars(void ){

    /*Measure the temperature*/
    int ThermResistance;
    int VoltageDropADC;
    float VoltageDrop;
    float R;

    analogReadResolution(12);
    VoltageDropADC = analogRead(THERMISTORPIN);
    VoltageDrop = (float)VoltageDropADC * 3.3 / 4095.0;


    R = 33000/VoltageDrop - 10000;

    this->_thisRunCurTemp = R / THERMISTORNOMINAL;     // (R/Ro)
    this->_thisRunCurTemp = log(this->_thisRunCurTemp);                  // ln(R/Ro)
    this->_thisRunCurTemp /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
    this->_thisRunCurTemp += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
    this->_thisRunCurTemp = 1.0 / this->_thisRunCurTemp;                 // Invert
    this->_thisRunCurTemp -= 273.15;                         // convert to C

}






/*The functions to access a copy of variables for viewing*/
double HeatControlSM::getCurrentTemperature(void) {return this->CurrentTemperature;}
double HeatControlSM::getTargetTemperature(void) {return this->SetTemperature;}
double HeatControlSM::getCurrentPIDValue(void) {return this->ThisRunPIDValue;}
double HeatControlSM::getTemperatureRate(void) {return this->TempratureRateMovingAvg;}
double HeatControlSM::getTemperatureSP(void) {return this->SetTemperature;}
double HeatControlSM::getTRateSP(void) {return this->RSetpoint;}
int HeatControlSM::getCurrentState(void) {return (int)this->CurrentFSMState;}
int HeatControlSM::getShouldBeState(void) {return (int)this->ShouldBeFSMState;}
