// GPSEngine.h

#ifndef _GPSENGINE_h
#define _GPSENGINE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#include "CurieTimerOne.h"
#include "FileLogger.h"

void tick();

class GPSEngineClass
{
 protected:
	Adafruit_GPS *GPS;
	uint32_t timer;
	char time[25];

 public:
	void init(SoftwareSerial* gpsSerial);
	void read();
	void loop();
	const char* getTime();
	~GPSEngineClass();
};

extern GPSEngineClass GPSEngine;

#endif

