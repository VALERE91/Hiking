// 
// 
// 

#include "GPSEngine.h"

void tick()
{
	GPSEngine.read();
}

void GPSEngineClass::init(SoftwareSerial* gpsSerial)
{
	GPS = new Adafruit_GPS(gpsSerial);

	GPS->begin(9600);
	GPS->sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
	GPS->sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);  
	GPS->sendCommand(PGCMD_ANTENNA);

	delay(1000);
	gpsSerial->println(PMTK_Q_RELEASE);
	timer = millis();
	CurieTimerOne.start(1000, &tick);
}

void GPSEngineClass::read()
{
	GPS->read();
}

void GPSEngineClass::loop()
{
	if (timer > millis())  timer = millis();

	if (GPS->newNMEAreceived()) {
		if (!GPS->parse(GPS->lastNMEA()))
			return;
	}

	if (millis() - timer > 1000) {
		timer = millis(); // reset the timer
		if (GPS->fix) {
			char bufferTemp[128];
			int charWritten = sprintf(time, "20%02d-%02d-%02dT%02d:%02d:%02d.%03d", GPS->year,
				GPS->month,
				GPS->day,
				GPS->hour,
				GPS->minute,
				GPS->seconds,
				GPS->milliseconds);
			time[24] = '\0';

			charWritten = sprintf(bufferTemp, "20%02d-%02d-%02dT%02d:%02d:%02d.%03d;%.4f%c;%.4f%c;%.6f;%.6f;%.2f;%.2f;%.2f;%d",
				GPS->year,
				GPS->month,
				GPS->day,
				GPS->hour,
				GPS->minute,
				GPS->seconds,
				GPS->milliseconds,
				GPS->latitude,
				GPS->lat,
				GPS->longitude,
				GPS->lon,
				GPS->latitudeDegrees,
				GPS->longitudeDegrees,
				GPS->speed,
				GPS->angle,
				GPS->altitude,
				(int)GPS->satellites);

			char subbuff[charWritten + 1];
			memcpy(subbuff, bufferTemp, charWritten);
			subbuff[charWritten] = '\0';
#ifdef DEBUG
			Serial.print("Received : ");Serial.println(subbuff);
#endif
			if (FileLogger.ready()) {
				FileLogger.writeGPS(subbuff);
#ifdef DEBUG
				Serial.print("Wrote : ");Serial.println(subbuff);
#endif
			}
		}
	}
}

const char* GPSEngineClass::getTime()
{
	return time;
}

GPSEngineClass::~GPSEngineClass()
{
	if (GPS != NULL) {
		delete(GPS);
	}
}

GPSEngineClass GPSEngine;
