// FileLogger.h

#ifndef _FILELOGGER_h
#define _FILELOGGER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <SPI.h>
#include <SD.h>

class FileLoggerClass
{
 protected:
	 const char* metadatFile;
	 const char* imuFile;
	 const char* gpsFile;
	 bool initialized = false;
	 File dataFile;

 public:
	void init(uint8_t chipSelect, const char* metadatFile, const char* imuFile, const char* gpsFile);
	bool write(const char* file, char* data);
	bool writeGPS(char* data);
	bool writeIMU(char* data);
	bool ready();
};

extern FileLoggerClass FileLogger;

#endif

