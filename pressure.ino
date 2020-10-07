/*
 This Arduino sketch controls a personal weather station and use an ethernet shield to upload data to wunderground.com.
 Details about the weather station can be found at http://www.avanux.de/space/Arduino/Wetterstation.
 */

//////////////
// Variables
//////////////


//////////////
// Set up
//////////////

void setupPressure() {
}

//////////////
// Loop
//////////////

void loopPressure() {
  int pressureValue = analogReadSmoothed(pressurePin);
  pressure=((pressureValue/1024.0)+0.095)/0.000009;

#ifdef INFO_WS
  Serial.print(F("Pressure="));
  Serial.print(pressure);
  Serial.println(F(" pa"));
#endif
}

