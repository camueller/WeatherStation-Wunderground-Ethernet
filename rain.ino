/*
 This Arduino sketch controls a personal weather station and use an ethernet shield to upload data to wunderground.com.
 Details about the weather station can be found at http://www.avanux.de/space/Arduino/Wetterstation.
 */

//////////////
// Variables
//////////////

const unsigned long RAIN_INTERVAL_SECONDS = 300;
const          int  RAIN_INTERVALS = 12; // 12 * 5min = 60 min
const          float MM_PER_SIGNAL = 0.2794;
unsigned       int  rainSignalCountInterval[RAIN_INTERVALS];
unsigned       int  rainSignalCountIntervalIndex;
volatile       int  rainSignalCount;
unsigned       long rainIntervalBegin;
               float rainInterval;

//////////////
// Set up
//////////////

void setupRain(unsigned long now) {
  attachInterrupt(rainInterrupt, rainSignal, RISING);
  rainSignalCount = 0;
  rainSignalCountIntervalIndex = 0;
  memset(rainSignalCountInterval, 0, sizeof(rainSignalCountInterval));
  rainIntervalBegin = now;
  rainInterval = 0;
  rainToday = 0;
}

//////////////
// Loop
//////////////

void loopRain(unsigned long now) {
#ifdef DEBUG_WS  
    Serial.print(F("rainSignalCount="));
    Serial.print(rainSignalCount);
    Serial.print(F(" rainSignalCountIntervalIndex="));
    Serial.println(rainSignalCountIntervalIndex);
#endif
  // calculate rain in interval
  boolean intervalIndexReset = false;
  if(now - rainIntervalBegin > 1000 * RAIN_INTERVAL_SECONDS || rainIntervalBegin > now) {
    rainSignalCountInterval[rainSignalCountIntervalIndex] = rainSignalCount;
    rainSignalCount = 0;
    rainIntervalBegin = now;
    
#ifdef DEBUG_WS  
    for(int i=0;i<RAIN_INTERVALS;i++) {
      Serial.print(F("rainSignalCountInterval["));
      Serial.print(i);
      Serial.print(F("]="));
      Serial.println(rainSignalCountInterval[i]);
    }
#endif

    rainInterval = rainSignalCountInterval[rainSignalCountIntervalIndex] * MM_PER_SIGNAL;
    rainToday += rainInterval;
    rainSignalCountIntervalIndex += 1;
    if(rainSignalCountIntervalIndex >= RAIN_INTERVALS) {
      rainSignalCountIntervalIndex = 0;
      intervalIndexReset = true;
    }
  }  

  // calculate rain total of all intervals
  rainLastHour = 0;
  for(int i=0;i<RAIN_INTERVALS;i++) {
    rainLastHour += rainSignalCountInterval[i] * MM_PER_SIGNAL;
  }

  // reset rainfall today  
  if(hourOfDay == 0) {
    // during summer time this is 1AM rather than midnight since time is GMT
    rainToday = 0; 
  }

#ifdef INFO_WS
  Serial.print(F("Rainfall ("));
  Serial.print(RAIN_INTERVAL_SECONDS);
  Serial.print(F("s): "));
  Serial.print(rainInterval);
  Serial.println(F(" mm"));

  Serial.print(F("Rainfall ("));
  Serial.print(RAIN_INTERVAL_SECONDS * RAIN_INTERVALS);
  Serial.print(F("s): "));
  Serial.print(rainLastHour);
  Serial.println(F(" mm"));

  Serial.print(F("Rainfall today: "));
  Serial.print(rainToday);
  Serial.println(F(" mm"));
#endif
}

//////////////
// Functions
//////////////

void rainSignal() {
  rainSignalCount++;
}
