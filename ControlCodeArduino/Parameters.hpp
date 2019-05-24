//
//  CryoControlSM.hpp
//  CC_R
//
//  Created by Pitam Mitra on 1/6/19.
//  Copyright © 2019 Pitam Mitra. All rights reserved.
//

#ifndef Parameters_hpp
#define Parameters_hpp


#define RateMovingAvgN 20
#define MovingAvgN 5
#define DeltaTRatePerMin 3.0
#define TargetTemperature 80.0

/*SM variables and memory*/
#define KpA 0.05
#define KiA 20
#define KdA 1

#define KpR 0.2
#define KiR 5
#define KdR 1

/*Pinouts*/
// which analog pin to connect
#define THERMISTORPIN A0
// resistance at 25 degrees C
#define THERMISTORNOMINAL 10000
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 5
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3984
// the value of the divider resistor
#define SERIESRESISTOR 10000
// pinout of the relay
#define RELAYPIN 8

/*Heating / cooling cycle times*/
#define SecondsBeforeHeatingStarts 120
#define SecondsToMaintainWarm 4*60*60


#endif
