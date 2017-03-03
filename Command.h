// Command.h

#ifndef _COMMAND_h
#define _COMMAND_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

class CommandClass
{
 protected:
	 bool _write;
	 String _hikingName;
	 String _sensorLocation;
	 String _peopleRecording;

	 static CommandClass* _instance;
 public:
	void init();
	
	bool Write() const { return _write; }
	void Write(bool val) { _write = val; }
	static CommandClass* Instance();
	String HikingName() const { return _hikingName; }
	void HikingName(String val) { _hikingName = val; }
	String SensorLocation() const { return _sensorLocation; }
	void SensorLocation(String val) { _sensorLocation = val; }
	String PeopleRecording() const { return _peopleRecording; }
	void PeopleRecording(String val) { _peopleRecording = val; }
};

extern CommandClass Command;

#endif

