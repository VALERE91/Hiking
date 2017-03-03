#include "Command.h"

#include "FileLogger.h"

#define DEBUG

void FileLoggerClass::init(uint8_t chipSelect,const char* metadatFile, const char* imuFile, const char* gpsFile)
{
	Serial.begin(115200);
	Serial.println("Initializing SD card...");

	initialized = SD.begin(chipSelect);
	if (!initialized) {
		Serial.println("Card failed, or not present");
	}
	else {
		Serial.println("Card initialized.");
		if (!SD.exists(metadatFile)) {
			dataFile = SD.open(metadatFile, FILE_WRITE);
			dataFile.close();
		}
		if (!SD.exists(imuFile)) {
			dataFile = SD.open(imuFile, FILE_WRITE);
			dataFile.println("time,acc_x,acc_y,acc_z,gyr_x,gyr_y,gyr_z,mag_x,mag_y,mag_z,heading,pitch,roll,floor,people,sensor_location");
			dataFile.flush();
			dataFile.close();
		}
		if (!SD.exists(gpsFile)) {
			dataFile = SD.open(gpsFile, FILE_WRITE);
			dataFile.println("time,latitude,longitude,latitude_degree,longitude_degree,speed,angle,altitude,satellites");
			dataFile.flush();
			dataFile.close();
		}
		initialized = true;
		this->metadatFile = metadatFile;
		this->gpsFile = gpsFile;
		this->imuFile = imuFile;
	}
}

bool FileLoggerClass::write(const char * file, char * data)
{
	if (initialized && CommandClass::Instance()->Write()) {
		dataFile = SD.open(file, FILE_WRITE);
		dataFile.print(data);
		dataFile.flush();
		dataFile.close();
		return true;
	}
	return false;
}

bool FileLoggerClass::writeGPS(char * data)
{
	return write(gpsFile, data);
}

bool FileLoggerClass::writeIMU(char * data)
{
	return write(imuFile, data);
}

bool FileLoggerClass::ready()
{
	return initialized;
}


FileLoggerClass FileLogger;

