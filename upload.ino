/*
 This Arduino sketch controls a personal weather station and use an ethernet shield to upload data to wunderground.com.
 Details about the weather station can be found at http://www.avanux.de/space/Arduino/Wetterstation.
 */

//////////////
// Libraries
//////////////

#include <SPI.h>
#include <Ethernet.h>

//////////////
// Variables
//////////////

const char SERVER[]     = "weatherstation.wunderground.com";
const char WEBPAGE[]    = "/weatherstation/updateweatherstation.php";
// defined in credentials.ino: const char ID[]       = "my pws id"; 
// defined in credentials.ino: const char PASSWORD[] = "my password";
const char HTTP_HOST[]  = " HTTP/1.0\r\nHost: ";
const char USER_AGENT[] = "\r\nUser-Agent: Arduino\r\n\r\n";
byte mac[]      = { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 };
const unsigned long IDLE_TIMEOUT_MS = 1000;
EthernetClient client;
const unsigned long UPLOAD_INTERVAL_SECONDS = 300;
const unsigned int  GMT_OFFSET = 1;
unsigned long upoadIntervalBegin;

//////////////
// Set up
//////////////

void setupUpload(unsigned long now) {
  if (Ethernet.begin(mac) == 0) {
#ifdef INFO_WS
    Serial.println(F("DHCP failed"));
#endif
  }
  else {
    printIPAddress();
  }
  upoadIntervalBegin = now;
}

//////////////
// Loop
//////////////

void loopUpload(unsigned long now) {
  switch (Ethernet.maintain())
  {
    case 1:
      Serial.println(F("Error: renewed fail"));
      break;
    case 2:
      Serial.println(F("Renewed success"));
      printIPAddress();
      break;
    case 3:
      Serial.println(F("Error: rebind fail"));
      break;
    case 4:
      Serial.println(F("Rebind success"));
      printIPAddress();
      break;
    default:
      //nothing happened
      break;
  }

  if(now - upoadIntervalBegin > 1000 * UPLOAD_INTERVAL_SECONDS || upoadIntervalBegin > now) {
#ifdef INFO_WS
    Serial.print(F("Connecting to "));Serial.print(SERVER);Serial.println(" ...");
#endif
    if (client.connect(SERVER, 80)) {

#ifdef INFO_WS
      Serial.print(F("Connected to "));Serial.println(SERVER);
#endif
      // reset watchdog
      wdt_long_disable();

      // Make a HTTP request:
      String cmd = F("GET ");
      client.print(cmd);
#ifdef DEBUG_UPLOAD
      Serial.print(cmd);
#endif

      cmd = WEBPAGE; cmd+=F("?");
      client.print(cmd);
#ifdef DEBUG_UPLOAD
      Serial.print(cmd);
#endif

      cmd = F("ID="); cmd += ID;
      client.print(cmd);
#ifdef DEBUG_UPLOAD
      Serial.print(cmd);
#endif

      cmd = F("&PASSWORD="); cmd += PASSWORD;
      client.print(cmd);
#ifdef DEBUG_UPLOAD
      Serial.print(cmd);
#endif

      cmd = F("&dateutc=now");
      client.print(cmd);
#ifdef DEBUG_UPLOAD
      Serial.print(cmd);
#endif

#ifdef ENABLE_TEMPERATURE
      cmd = F("&tempf="); cmd += celsius2fahrenheit(temperature);
      client.print(cmd);
  #ifdef DEBUG_UPLOAD
      Serial.print(cmd);
  #endif
#endif

#ifdef ENABLE_HUMIDITY
      cmd = F("&humidity="); cmd += humidity;
      client.print(cmd);
  #ifdef DEBUG_UPLOAD
      Serial.print(cmd);
  #endif

      cmd = F("&dewptf="); cmd += celsius2fahrenheit(dewpoint);
      client.print(cmd);
  #ifdef DEBUG_UPLOAD
      Serial.print(cmd);
  #endif
#endif

#ifdef ENABLE_WIND_DIRECTION
      cmd = F("&winddir="); cmd += windDirectionAvg; //windDirection;
      client.print(cmd);
  #ifdef DEBUG_UPLOAD
      Serial.print(cmd);
  #endif

      cmd = F("&winddir_avg2m="); cmd += windDirectionAvg;
      client.print(cmd);
  #ifdef DEBUG_UPLOAD
      Serial.print(cmd);
  #endif

      cmd = F("&windgustdir="); cmd += windGustDirection;
      client.print(cmd);
  #ifdef DEBUG_UPLOAD
      Serial.print(cmd);
  #endif
#endif
#ifdef ENABLE_WIND_SPEED
      cmd = F("&windspeedmph="); cmd += kph2mph(windSpeed);
      client.print(cmd);
  #ifdef DEBUG_UPLOAD
      Serial.print(cmd);
  #endif

      cmd = F("&windspdmph_avg2m="); cmd += kph2mph(windSpeedAvg);
      client.print(cmd);
  #ifdef DEBUG_UPLOAD
      Serial.print(cmd);
  #endif

      cmd = F("&windgustmph="); cmd += kph2mph(windSpeedGust);
      client.print(cmd);
  #ifdef DEBUG_UPLOAD
      Serial.print(cmd);
  #endif

      cmd = F("&windgustmph_10m="); cmd += kph2mph(windSpeedGust);
      client.print(cmd);
  #ifdef DEBUG_UPLOAD
      Serial.print(cmd);
  #endif
#endif
#ifdef ENABLE_PRESSURE
      cmd = F("&baromin="); cmd += pa2inHg(pressure);
      client.print(cmd);
  #ifdef DEBUG_UPLOAD
      Serial.print(cmd);
  #endif
#endif
#ifdef ENABLE_RAIN
      cmd = F("&rainin="); cmd += mm2in(rainLastHour);
      client.print(cmd);
  #ifdef DEBUG_UPLOAD
      Serial.print(cmd);
  #endif

      cmd = F("&dailyrainin="); cmd += mm2in(rainToday);
      client.print(cmd);
  #ifdef DEBUG_UPLOAD
      Serial.print(cmd);
  #endif
#endif
#ifdef ENABLE_SOLAR_RADIATION
      cmd = F("&solarradiation="); cmd += solarRadiation;
      client.print(cmd);
  #ifdef DEBUG_UPLOAD
      Serial.print(cmd);
  #endif
#endif

      cmd = F("&action=updateraw");
      client.print(cmd);
#ifdef DEBUG_UPLOAD
      Serial.print(cmd);
#endif

      cmd = HTTP_HOST;
      client.print(cmd);
#ifdef DEBUG_UPLOAD
      Serial.print(cmd);
#endif

      cmd = SERVER;
      client.print(cmd);
#ifdef DEBUG_UPLOAD
      Serial.print(cmd);
#endif

      cmd = USER_AGENT;
      client.print(cmd);
#ifdef DEBUG_UPLOAD
      Serial.print(cmd);
#endif

      // HTTP/1.1 200 OK
      // Date: Tue, 06 Oct 2020 12:32:14 GMT
      // Content-Type: text/plain; charset=utf-8
      // Content-Length: 8
      // Connection: close
      // Cache-Control: max-age=0, no-cache, must-revalidate, proxy-revalidate
      // Expires: Tue, 06 Oct 2020 12:32:14 GMT
      // Last-Modified: {now} GMT
      //
      // success
      //

      const int responseLen = 400;
      char response[responseLen+1];
      memset(response, 0, sizeof(response));
      int read = 0;
      int tries = 0;
      char datain;
      while (client.connected() && tries<50000) {
        if(client.available()) {
          char datain = client.read();
          if(datain > 0) {
            response[read] = datain;
            read++;
          }
        }
        tries++;
      }
#ifdef DEBUG_UPLOAD
      Serial.print(F("Bytes read="));Serial.println(read);
      Serial.print(F("Tries="));Serial.println(tries);
      Serial.print(F("Response="));Serial.println(response);
#endif

      for(int i=0;i<responseLen;i++) {
        // search for "GMT"
        if(i>=32 && response[i-32]=='D' && response[i-31]=='a' && response[i-30]=='t' && response[i-29]=='e' && response[i]=='G' && response[i+1]=='M' && response[i+2]=='T') {
          char hourOfDayString[2+1];
          hourOfDayString[0]=response[i-9];
          hourOfDayString[1]=response[i-8];
          hourOfDayString[2] = '\0';
          hourOfDay = (atoi(hourOfDayString) + GMT_OFFSET) % 24; // +1 : we are GMT+1 in winter and ignore DST
        }
      }
#ifdef DEBUG_UPLOAD
      Serial.print(F("hourOfDay="));Serial.println(hourOfDay);
#endif
    }
    else {
      Serial.println(F("Connection failed to "));Serial.println(SERVER);
    }

    client.flush();
    delay(2);
    client.stop();
    delay (2);
    while (client.connected()) {
      client.stop();
      delay (2);
    }

    upoadIntervalBegin = now;
  }

  // uploading may take some seconds, therefore we have to ignore
  // the count increase during that time; otherwise we will see very
  // high wind speed because of that wrong count increase
  initWindSpeedSignalCount();
}

int kph2mph(float kph) {
  return kph * 0.621371192;
}

int celsius2fahrenheit(float celsius) {
  return celsius * 1.8 + 32;
}

float pa2inHg(float pa) {
  // http://www.engineeringtoolbox.com/pressure-units-converter-d_569.html
  return pa * 0.000296;
}

float mm2in(float mm) {
  return mm / 25.4;
}

void printIPAddress()
{
  Serial.print(F("My IP address: "));
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
  }

  Serial.println();
}
