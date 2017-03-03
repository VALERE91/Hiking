#include <Wire.h>
#include <SD.h>

#include <SparkFunLSM9DS1.h>

#include "Command.h"
#include "FileLogger.h"

#include <CurieBLE.h>

#define DATA_NUMBER 13
#define FREQUENCY 60
#define BUFFER_SIZE 10 * DATA_NUMBER

#define LSM9DS1_M	0x1E // Would be 0x1C if SDO_M is LOW
#define LSM9DS1_AG	0x6B // Would be 0x6A if SDO_AG is LOW

#define DECLINATION -8.58

BLEPeripheral blePeripheral; // create peripheral instance
BLEService ledService("00000d00-0000-1000-8000-00805f9b34fb"); // create service
															   // create switch characteristic and allow remote device to read and write
BLECharacteristic ledCharacteristic("00000d02-0000-1000-8000-00805f9b34fb", BLERead | BLEWrite | BLENotify, 1);
BLECharacteristic floorCharacteristic("00000d01-0000-1000-8000-00805f9b34fb", BLERead | BLEWrite | BLENotify, 20); // allows remote device to get notifications
BLECharacteristic locationCharacteristic("00000d03-0000-1000-8000-00805f9b34fb", BLERead | BLEWrite | BLENotify, 20); // allows remote device to get notifications
BLECharacteristic userCharacteristic("00000d04-0000-1000-8000-00805f9b34fb", BLERead | BLEWrite | BLENotify, 20); // allows remote device to get notifications

String locationName, userName, floorName;
bool save = false;
long lastPrint = 0;
long lastFreq = 0;

float ax, ay, az;
float gx, gy, gz;
float mx, my, mz;
float temp, pressure, altitude;

float buffer[BUFFER_SIZE];
int currentValue = 0;

char bufferTemp[1024];
char* b = bufferTemp;
int counter = 0;

LSM9DS1 imu;

void setup() {
	Serial.begin(115200);

	Serial.println("Starting...");
	// set the local name peripheral advertises
	blePeripheral.setLocalName("Hiking");
	// set the UUID for the service this peripheral advertises:
	blePeripheral.setAdvertisedServiceUuid(ledService.uuid());

	// add service and characteristics
	blePeripheral.addAttribute(ledService);
	blePeripheral.addAttribute(ledCharacteristic);
	blePeripheral.addAttribute(floorCharacteristic);
	blePeripheral.addAttribute(locationCharacteristic);
	blePeripheral.addAttribute(userCharacteristic);
	
	unsigned char stop = 0x00;
	ledCharacteristic.setValue(&stop,1);

	// Try to initialise and warn if we couldn't detect the chip
	Serial.println("Starting LSM9DS1");

	imu.settings.device.commInterface = IMU_MODE_I2C;
	imu.settings.device.mAddress = LSM9DS1_M;
	imu.settings.device.agAddress = LSM9DS1_AG;

	imu.settings.accel.enabled = true;
	imu.settings.accel.scale = A_SCALE_16G;
	imu.settings.accel.sampleRate = XL_ODR_119;

	imu.settings.mag.enabled = true;
	imu.settings.mag.scale = M_SCALE_16GS;
	imu.settings.mag.sampleRate = M_ODR_80;

	imu.settings.gyro.enabled = true;
	imu.settings.gyro.scale = G_SCALE_2000DPS;
	imu.settings.gyro.sampleRate = G_ODR_119;
	imu.settings.gyro.flipX = false; // Don't flip X
	imu.settings.gyro.flipY = false; // Don't flip Y
	imu.settings.gyro.flipZ = false; // Don't flip Z

	if (!imu.begin())
	{
		Serial.println("Failed to communicate with LSM9DS1.");
		Serial.println("Double-check wiring.");
		Serial.println("Default settings in this sketch will " \
			"work for an out of the box LSM9DS1 " \
			"Breakout, but may need to be modified " \
			"if the board jumpers are.");
		while (1)
			;
	}

	Serial.println("Found and configured LSM9DS1 9DOF");

	Serial.println("");
	Serial.println("");

	// advertise the service
	blePeripheral.begin();

	Serial.println("Bluetooth device active, waiting for connections...");
	FileLogger.init(8, "metadata.dat", "imu.csv", "gps.csv");
}

void loop() {
	
	blePeripheral.poll();
	
	if (ledCharacteristic.written()) {
		save = ledCharacteristic.value()[0];
		CommandClass::Instance()->Write(save);
	}
	if (floorCharacteristic.written()) {
		String s((const char*)floorCharacteristic.value());
		CommandClass::Instance()->HikingName(s.substring(0, floorCharacteristic.valueLength()));
	}
	if (userCharacteristic.written()) {
		String s((const char*)userCharacteristic.value());
		CommandClass::Instance()->PeopleRecording(s.substring(0, userCharacteristic.valueLength()));
	}
	if (locationCharacteristic.written()) {
		String s((const char*)locationCharacteristic.value());
		CommandClass::Instance()->SensorLocation(s.substring(0, locationCharacteristic.valueLength()));
	}
	
	if (save && millis() - lastFreq > 1000) {
		lastFreq = millis();
		Serial.println(counter);
		counter = 0;
	}

	if (save && millis() - lastPrint > (1000/FREQUENCY)) {
		lastPrint = millis();

		// Update the sensor values whenever new data is available
		if (imu.gyroAvailable() && imu.accelAvailable() && imu.magAvailable())
		{
			imu.readGyro();
			imu.readAccel();
			imu.readMag();
		}else {
			return;
		}

		float roll = atan2(imu.ay, imu.az);
		float pitch = atan2(-imu.ax, sqrt(imu.ay * imu.ay + imu.az * imu.az));

		float heading;
		if (imu.my == 0)
			heading = (imu.mx < 0) ? PI : 0;
		else
			heading = atan2(imu.mx, my);

		heading -= DECLINATION * PI / 180;

		if (heading > PI) heading -= (2 * PI);
		else if (heading < -PI) heading += (2 * PI);
		else if (heading < 0) heading += 2 * PI;

		// Convert everything from radians to degrees:
		heading *= 180.0 / PI;
		pitch *= 180.0 / PI;
		roll *= 180.0 / PI;

		buffer[currentValue++] = millis();
		buffer[currentValue++] = imu.calcAccel(imu.ax);
		buffer[currentValue++] = imu.calcAccel(imu.ay);
		buffer[currentValue++] = imu.calcAccel(imu.az);
		buffer[currentValue++] = imu.calcGyro(imu.gx);
		buffer[currentValue++] = imu.calcGyro(imu.gy);
		buffer[currentValue++] = imu.calcGyro(imu.gz);
		buffer[currentValue++] = imu.calcMag(imu.mx);
		buffer[currentValue++] = imu.calcMag(imu.my);
		buffer[currentValue++] = imu.calcMag(imu.mz);
		buffer[currentValue++] = heading;
		buffer[currentValue++] = pitch;
		buffer[currentValue++] = roll;

		counter++;

		if (currentValue >= BUFFER_SIZE) {
			currentValue = 0;
			for (int i = 0; i < BUFFER_SIZE - DATA_NUMBER;i+= DATA_NUMBER) {
				b += sprintf(b, "%s,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%s,%s,%s\n", String(buffer[i]).c_str(), 
					buffer[i+1], buffer[i + 2], buffer[i + 3],
					buffer[i + 4], buffer[i + 5], buffer[i + 6],
					buffer[i + 7], buffer[i + 8], buffer[i + 9],
					buffer[i + 10], buffer[i + 11], buffer[i + 12],
					CommandClass::Instance()->HikingName().c_str(),
					CommandClass::Instance()->PeopleRecording().c_str(),
					CommandClass::Instance()->SensorLocation().c_str());
			}
			b = bufferTemp;
			FileLogger.writeIMU(bufferTemp);
		}
	}
}