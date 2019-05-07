//
//  CryoControlSM.hpp
//  CC_R
//
//  Created by Pitam Mitra on 1/6/19.
//  Copyright © 2019 Pitam Mitra. All rights reserved.
//

#ifndef CryoControlSM_hpp
#define CryoControlSM_hpp

#include <stdio.h>
#include "PID_v1.h"
#include <map>

#include <cmath>
#include <vector>
//#include <iomanip>

#include "Parameters.hpp"


class HeatControlSM {

private:


    double LastTemperature;
    unsigned long nowtime;

    double TInput, TOutput;
    double RInput, ROutput;

    bool OutputPower=0;

    /*SM variables and memory*/
    //double KpA=2, KiA=5, KdA=1;
    //double KpR=2, KiR=5, KdR=1;

    /*Process related variables*/
    double ThisRunPIDValue=0.0;
    double ThisRunCCPower=0.0;
    double SentCCPower=0.0;
    double CurrentTemperature=0.0;

    double TempratureRateMovingAvg=0.0;
    double TempratureMovingAvg=0.0;
    double RSetpoint=0.0;
    double _thisRunCurTemp=0.0;



    /* ***********************************************************
     *We have two different PIDs.
     *
     *1. For controlling the absolute value of the temperature
     *   and is used around setpoints when stability is desired.
     *
     *2. For controlling the rate of ascent or descent of temperature
     * ***********************************************************/

    PID* AbsPID;
    PID* RatePID;

    /*SM Functions and states*/
    void UpdateVars();

    void Idle(void);
    void CoolDown(void );
    void Warmup(void);
    void MaintainWarm(void);


    void StateDecision( void);
    void StateSwitch (void );

    bool EntryGuardActive=false;
    bool ExitGuardActive=false;
    bool FSMMode=AUTOMATIC;


public:

    HeatControlSM();
    ~HeatControlSM(void );

    /*The SM Engine to be run at every time interval*/
    void SMEngine(void);

    /*Functions to access a copy of variables for viewing*/
    double getCurrentTemperature(void);
    double getTargetTemperature(void);
    double getCurrentPIDValue(void);
    double getTemperatureRate(void);
    double getTemperatureSP(void);
    double getTRateSP(void);
    int getCurrentState(void);
    int getShouldBeState(void);
    double getSentCCPower(void);

    /*process variable*/
    double SetTemperature=0.0;
    unsigned long StateTime=0.0;

    /*SM States - need to be read by other functions elsewhere*/
    /*Enum values of all the states that the FSM can be in*/
    enum FSMStates {
        ST_Idle,
        ST_CoolDown,
        ST_Warmup,
        ST_MaintainWarm,
    };

    FSMStates CurrentFSMState;
    FSMStates ShouldBeFSMState;



};

#endif /* CryoControlSM_hpp */
