/*
 This Arduino sketch controls a personal weather station and use an ethernet shield to upload data to wunderground.com.
 Details about the weather station can be found at http://www.avanux.de/space/Arduino/Wetterstation.
 */

//////////////
// Variables
//////////////

const long WIND_SPEED_AVG_INTERVAL_SECONDS = 120;
const long WIND_SPEED_GUST_INTERVAL_SECONDS = 600;
volatile uint16_t windSpeedSignalCount;
unsigned long  windCalcIntervalBegin;
unsigned long  windSpeedAvgIntervalBegin;
         float windSpeedAvgValues;
         float windSpeedAvgValueSum;
unsigned long  windSpeedGustIntervalBegin;

//////////////
// Set up
//////////////

void setupWindSpeed(unsigned long now)
{
  attachInterrupt(windSpeedInterrupt, windSignal, RISING);
  windSpeed = 0.0;
  windSpeedAvg = 0.0;
  windSpeedGust = 0.0;
  initWindSpeedSignalCount();
  windCalcIntervalBegin = now;
  windSpeedAvgIntervalBegin = now;
  windSpeedGustIntervalBegin = now;
}

void initWindSpeedSignalCount() {
  windSpeedSignalCount = 0;
}


//////////////
// Loop
//////////////

void loopWindSpeed(unsigned long now)
{
  // calculate wind speed  
  float intervalSeconds = (now - windCalcIntervalBegin) / 1000.0;
  windSpeed = windSpeedSignalCount * 2.4 / intervalSeconds;
#ifdef DEBUG_WS  
  Serial.print(F("windSpeedSignalCount="));
  Serial.print(windSpeedSignalCount);
  Serial.print(F(" intervalSeconds="));
  Serial.println(intervalSeconds);
#endif
#ifdef INFO_WS
  Serial.print(F("wind speed "));
  Serial.print(windSpeed);
  Serial.println(F(" kph"));
#endif  
  windSpeedSignalCount=0;  
  windCalcIntervalBegin = now;
    
  // calculate average
  windSpeedAvgValues++;
  windSpeedAvgValueSum += windSpeed;
  
  if(now - windSpeedAvgIntervalBegin > WIND_SPEED_AVG_INTERVAL_SECONDS * 1000 || windCalcIntervalBegin > now) {
    windSpeedAvg = windSpeedAvgValueSum / windSpeedAvgValues;

#ifdef DEBUG_WS  
    Serial.print(F("windSpeedAvgValueSum="));
    Serial.print(windSpeedAvgValueSum);
    Serial.print(F(" windSpeedAvgValues="));
    Serial.println(windSpeedAvgValues);
#endif
#ifdef INFO_WS
    Serial.print(F("Wind avg("));
    Serial.print(WIND_SPEED_AVG_INTERVAL_SECONDS);
    Serial.print(F("s): "));
    Serial.print(windSpeedAvg);
    Serial.println(F(" kph"));
#endif      
    windSpeedAvgIntervalBegin = now;
    windSpeedAvgValues = 0;
    windSpeedAvgValueSum = 0;
  }
    
  if(now - windSpeedGustIntervalBegin > WIND_SPEED_GUST_INTERVAL_SECONDS * 1000 || windSpeedGustIntervalBegin > now) {
#ifdef INFO_WS
    Serial.print(F("Wind gusts ("));
    Serial.print(WIND_SPEED_GUST_INTERVAL_SECONDS);
    Serial.print(F("s): "));
    Serial.print(windSpeedGust);
    Serial.println(F(" kph"));
#endif      
    windSpeedGustIntervalBegin = now;
    windSpeedGust = 0.0;
  }
  if(windSpeed > windSpeedGust) {
    windSpeedGust = windSpeed;
    windGustDirection = windDirection;
  }
}

//////////////
// Functions
//////////////

void windSignal()
{
  windSpeedSignalCount++;
}
