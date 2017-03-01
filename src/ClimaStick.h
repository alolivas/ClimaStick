// The MIT License (MIT)
//
// Copyright (c) 2017 THINK BIG LABS SL
// Author: jorge.trincadoc@gmail.com (Jorge Trincado Castán)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.



//*****************************ClimaStick.h******************************/
/*Contains a redefinition of ESP-12 pins, the includes of all libraries */
/*needed to use builted in sensors and connect to Thinger.io with 		*/
/*Clima&Move v1.0 developement board.								    */

 

#ifndef ClimaStick_h
#define ClimaStick_h

	#include <ESP8266WiFi.h>		//GENERIC ESP8266  
	#include <ThingerESP8266.h>		//THINGER.IO IOT PLATFORM  
  
	#include <Wire.h>				//I2C BUS

	#include "Adafruit_Sensor.hpp"

	#include "HMC5883L.hpp"			//MAGNETOMETER
	#include "MPU6050.hpp" 			//ACELEROMETER & GYRO
	#include "Adafruit_BME280.hpp"	//ENVIRONEMENT SENSOR
    #include "TSL2561.h"			//LUMINOSITY SENSOR

	#include "NTPClient.hpp"
	#include <WiFiUdp.h>

	/***PIN REDEFINITION***/

	/*rgb led pin*/
	static const uint8_t RED = 13;
	static const uint8_t GREEN = 12;
	static const uint8_t BLUE = 14;
	static const uint8_t R = 13;
	static const uint8_t G = 12;
	static const uint8_t B = 14;

	/*battery voltaje to ADC pin, 400K pullup 100K pulldown voltaje divider*/
	static const uint8_t BAT   = 0;

	/*user button to GPIO_0*/
	static const uint8_t USR   = 0;
	static const uint8_t BUTTON = 0;

	/*GPIO_16 is connected to RESET circuitery*/
	static const uint8_t WUP   = 16;
	static const uint8_t WAKEUP   = 16;
	

	/*struct variables*/

	struct accelerometer {int16_t ax, ay, az, gx, gy, gz;};
	//struct gyroscope {int16_t x, y, z;};
	struct compass {float heading,headingDegrees;};
	struct magnetometer {float x, y, z, nx, ny, nz;};
	struct environmental {float temperature, humidity, pressure, altitude, luminosity, lux;};
	struct realTime{int hour, minute, second;};

	uint16_t visibleSpectum,fullSpectum,IRSpectum;
   
	Vector raw, norm;
	sensors_event_t event;
 		
	#define SEALEVELPRESSURE_HPA (1013.25)  //PRESSURE REFERENCE
   
	/*Sensor instances*/ 
 	HMC5883L Compass;
  	MPU6050 mpu;
  	Adafruit_BME280 bme; 
  	TSL2561 tsl(TSL2561_ADDR_FLOAT); 

 	/*Other instances*/
 	WiFiUDP ntpUDP;
	NTPClient timeClient(ntpUDP);
 

	/*INICIALIZING ALL SENSORS: I2C HANDSHAKE*/
	void builtinSensors_begin(){
		#ifdef _DEBUG_
			Serial.begin(115200);
		#endif
	    Wire.begin();
	 
		/*Inicialicing sensors*/
 			int begin_ok = Compass.begin();
			#ifdef _DEBUG_
			Serial.println(begin_ok ? "HMC5883L connection successful" : "HMC5883L connection failed");
		    #endif
		    Compass.setRange(HMC5883L_RANGE_1_3GA);         // Set measurement range
	 		Compass.setMeasurementMode(HMC5883L_CONTINOUS); // Set measurement mode
			Compass.setDataRate(HMC5883L_DATARATE_30HZ);    // Set data rate
			Compass.setSamples(HMC5883L_SAMPLES_8);         // Set number of samples averaged
			Compass.setOffset(0, 0);                        // Set calibration offset. See HMC5883L_calibration.ino
	   
	 		begin_ok = mpu.testConnection();
	 		#ifdef _DEBUG_
	 		Serial.println(begin_ok ? "MPU6050 connection successful" : "MPU6050 connection failed");	
		 	#endif
		 	mpu.initialize();
	 		mpu.setI2CMasterModeEnabled(false);
	 		mpu.setI2CBypassEnabled(true) ;
	  		mpu.setSleepEnabled(false);
			bme.begin();

			begin_ok = bme.begin();
			#ifdef _DEBUG_
	    	Serial.println(begin_ok ? "BME280 connection successful" : "BME280 connection failed");
	  		#endif

			begin_ok = tsl.begin();
	  		#ifdef _DEBUG_
			Serial.println(begin_ok ? "TSL2561 connection successful" : "TSL connection failed");
	  	  	#endif
	  	  	tsl.setGain(TSL2561_GAIN_16X);      // set 16x gain (for dim situations)
  		   	tsl.setTiming(TSL2561_INTEGRATIONTIME_13MS);  // shortest integration time (bright light)
			
	};

 
	 /*ACCELEROMETER AND GYROSCOPE*/
	accelerometer get_acceleration(){
	  	
	  	struct accelerometer accelgyro;
	  	
	  	mpu.getMotion6(&accelgyro.ax, &accelgyro.ay, &accelgyro.az, &accelgyro.gx, &accelgyro.gy, &accelgyro.gz);
		return accelgyro;
	};
 
	/*MAGNETOMETER COMPASS REFRESH*/
	compass get_compass(){
	  	
	  	struct compass myCompass;

	  	Vector norm = Compass.readNormalize();
		myCompass.heading = atan2(norm.YAxis, norm.XAxis); // Calculate heading

		// Set declination angle on your location and fix heading
		// You can find  your declination on: http://magnetic-declination.com/
		float declinationAngle = (4.0 + (26.0 / 60.0)) / (180 / M_PI); // Formula: (deg + (min / 60.0)) / (180 / M_PI);
		myCompass.heading += declinationAngle;

		// Correct for heading < 0deg and heading > 360deg
		if (myCompass.heading < 0)   myCompass.heading += 2 * PI;
		if (myCompass.heading > 2 * PI)  myCompass.heading -= 2 * PI;
		myCompass.headingDegrees = myCompass.heading * 180/M_PI; // Convert to degrees		
	
		return myCompass;
	};
	 
	 /*MAGNETOMETER AXIS RAW AND NORM*/
	magnetometer get_magnet(){
		struct magnetometer magnet;

	  	raw = Compass.readRaw();
		norm = Compass.readNormalize();

		magnet.x = raw.XAxis; 
		magnet.y = raw.YAxis; 
		magnet.x = raw.ZAxis; 
		magnet.nx = norm.XAxis; 
		magnet.ny = norm.YAxis; 
		magnet.nz = norm.ZAxis; 

		return magnet;
	};
	 
	 /*ENVIRONMENTAL VARIABLES*/
	environmental get_clima(){
 		struct environmental clima;

 		clima.temperature = bme.readTemperature();
		clima.humidity = bme.readHumidity();
		clima.pressure = bme.readPressure() / 100.0F;
		clima.altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
		 
 		visibleSpectum = tsl.getLuminosity(TSL2561_VISIBLE);     
 		fullSpectum = tsl.getLuminosity(TSL2561_FULLSPECTRUM);
 		IRSpectum = tsl.getLuminosity(TSL2561_INFRARED);
   		
   		clima.luminosity = tsl.getFullLuminosity();
   		clima.lux = visibleSpectum;
   		 

 		uint32_t lum = tsl.getFullLuminosity();
   		uint16_t ir, full;
 		ir = lum >> 16;
  		full = lum & 0xFFFF;
   
  		return clima;
	};
	    
	/*
	*	 BATTERY STATUS FUNCTIONS
	*/
	
	float batteryVoltaje(){ // read the ADC value and calculate the real time voltaje
		return (float)analogRead(BAT)/205;
	};

	
	float batteryLoad(){ // obtains the remain battery load
		#ifdef _batterySwProtection_
			if(batteryVoltaje()<=3.65) {
				for(int t=0;t<3;t++){
				digitalWrite(13,1);
				delay(70);
				digitalWrite(13,0);
				delay(70);
				}

				ESP.deepSleep(0); // forced deepsleep if battery breacks the voltage range

			}
		#endif
			float volt=batteryVoltaje();
			if(volt<4.2){
			volt-=3.7;
			volt*=200; 
			}else volt=100;	
			return volt;
	};

	/*
	*	NTP client refresh: fill Hour, minute, second and date variables.
	*/

	realTime get_time(){
		struct realTime Time;
		 
		 if(WiFi.status() == WL_CONNECTED) timeClient.update(); 
		 Time.hour = timeClient.getHours();
		 Time.minute = timeClient.getMinutes();
		 Time.second = timeClient.getSeconds();

		 return Time;
	};
 
	/*
	*	BUILTIN RGB LED FUNCTIONS
	*/
	  
	void rgb(int r, int g, int b){  
		analogWrite(R,r);
		analogWrite(G,g);
		analogWrite(B,b);
	}; 

 
	void rgb(String colorName){

		if(colorName.equals("BLUE"))		rgb(0,0,200);
		else if(colorName.equals("GREEN"))  rgb(0,150,0);
		else if(colorName.equals("YELLOW")) rgb(50,150,0);
		else if(colorName.equals("ORANGE")) rgb(100,150,0);
		else if(colorName.equals("RED"))  	rgb(150,30,0);
		else if(colorName.equals("PURPLE")) rgb(200,0,50);
		else if(colorName.equals("WHITE"))  rgb(50,200,200);
	  
	};  


	#endif  //from ifndef Clima_Move_h
