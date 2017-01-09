/*
  This program reads the Temperature and Pressure from the BMP180
  on the BerryIMU and displays the current values in gauges and also shows historical data in a chart.
   
  http://ozzmaker.com/  
*/
// needed to avoid link error on ram check
extern "C" 
{
#include "user_interface.h"
}



#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Arduino.h>
#include "user_interface.h"
#include <WiFiUdp.h>
#include <TimeLib.h> 
#include "bmp180.h"



#define GAUGE_REFRESH 1000 // How oftern (in milliseconds) to refresh the gauge values

#define POLLPERIOD  3  // How often (in seconds) to poll for data which is used in the graph


#define ALLOCATED_RAM 33520







//NTP
const int timeZone = 11;               // Sydney, Australia
const char* timerServerDNSName = "0.europe.pool.ntp.org";
IPAddress timeServer;
WiFiUDP Udp;
const unsigned int localPort = 8888;  // local port to listen for UDP packets
const int NTP_PACKET_SIZE = 48;       // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE];   // buffer to hold incoming & outgoing packets


uint32_t freeRAM;
double T,F,P;

const char* ssid = "******************";
const char* password = "***********************";


int count = 0;
unsigned long numberOfRows=0; // size of array
unsigned long timeKeeper=0;
float *tempCdata;
float *pressData;
unsigned long *timeStamp;



//ESP8266WebServer server(80);
WiFiServer server(80);



void setup()
{
  timeKeeper = millis();
  
  //Initialise I2C communication
  Wire.begin(4,5);
  // Initialise serial communication, set baud rate = 115200
  Serial.begin(115200);

  // Connect to WiFi network
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);

  //Print IP to console
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  delay(3);
  
  // Start the server
  server.begin();
  Serial.println("HTTP server started");

  //Read callibration data from BMP180
  readCalBMP180();



  


  //Setup time using NTP
  Serial.print("Resolving NTP Server IP ");
  WiFi.hostByName(timerServerDNSName, timeServer);
  Serial.println(timeServer.toString());

  Serial.print("Starting UDP... ");
  Udp.begin(localPort);
  Serial.print("local port: ");
  Serial.println(Udp.localPort());

  Serial.println("Waiting for NTP sync");
  setSyncProvider(getNtpTime);


  timeKeeper = millis() + (POLLPERIOD * 1000);

  allocateRam();

}//End of setup



void loop()
{
  double p0,a;
  char status;
    
  delay(100);

  //Get free RAM
  freeRAM = system_get_free_heap_size();
 if (freeRAM < lowestRAM    )  lowestRAM = freeRAM;
  Serial.print("  freeRAM : ");
  Serial.print(freeRAM);
  Serial.println(" bytes ");

  
  
  WiFiClient client = server.available();


  /////////////////////////////////////
  // Read the first line of the request
  /////////////////////////////////////
  String sRequest = client.readStringUntil('\r');
  client.flush();


  
  // get path; end of path is either space or ?
  // Syntax is e.g. GET /?show=1234 HTTP/1.1
  String sPath="",sParam="", sCmd="";
  String sGetstart="GET ";
  int iStart,iEndSpace,iEndQuest;
  iStart = sRequest.indexOf(sGetstart);
  if (iStart>=0)
  {
    iStart+=+sGetstart.length();
    iEndSpace = sRequest.indexOf(" ",iStart);
    iEndQuest = sRequest.indexOf("?",iStart);
    
    // are there parameters?
    if(iEndSpace>0)
    {
      if(iEndQuest>0)
      {
        // there are parameters
        sPath  = sRequest.substring(iStart,iEndQuest);
        sParam = sRequest.substring(iEndQuest,iEndSpace);
      }
      else
      {
        // NO parameters
        sPath  = sRequest.substring(iStart,iEndSpace);
      }
    }
  }


 String htmlContent ;

  if(sPath=="/"){
    //Build web page to show gauges
    //Some points to note;
    //  -"\n" is not needed, however it makes the source for the html file more readable.
    //  -Quotes used within html have to be escaped out with backslash "\".
    //  -variables have to be converted to strings using String()
    //  -Dont make the the string htmlContent too large as it will crash the ESP.
    

    htmlContent = ("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n");
    htmlContent += ("<html><head><style type=\"text/css\">\n");
    htmlContent += ("<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\” \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n");   ////Need to be able to view on iOS devices
    htmlContent += ("</style></head><body>\n");
    htmlContent += ("<div style=\"float:left; width:300px; height: 300px;\">\n");
    htmlContent += ("<h3 style=text-align:center;font-size:200%;>BerryIMU and ESP8266</h3>\n");
    htmlContent += ("<h3 style=text-align:center;>Temperature and pressure monitoring</h3>\n");
    htmlContent += ("<h3 style=text-align:center;>Free RAM = "+ String(freeRAM) + " Bytes</h3>");
    htmlContent += ("<h4  style=text-align:center; ><a href=\"/chart.html\">Charts V13</a><br>");
    htmlContent += ("<a  href=\"/table.html\">Table</a></h4></div>");
        
    //include Jquery and graphing API
    htmlContent += ("<script type='text/javascript' src='https://ajax.googleapis.com/ajax/libs/jquery/1.4.4/jquery.min.js'></script>\n");
    htmlContent += ("<script type='text/javascript' src='https://www.google.com/jsapi'></script>\n");
    htmlContent += ("<script type='text/javascript'>\n");
 

    htmlContent += ("$.ajaxSetup ({\n");
    htmlContent += ("// Disable caching of AJAX responses\n");
    htmlContent += ("cache: false\n");
    htmlContent += ("});\n");

    htmlContent += ("var chartC;var chartF;var chartP;var dataC;var dataF;var dataP;google.load('visualization', '1', {packages:['gauge']});\n");
    htmlContent += ("google.setOnLoadCallback(initChart);\n");
    client.print(htmlContent);
    
    //Temp Celsius update
    htmlContent = ("function displayDataC(cValue) {\n");
    htmlContent += ("dataC.setValue(0, 0, '" + String((char)176) + "C');\r\n");//Char 176 is degrees symbol
    htmlContent += ("dataC.setValue(0, 1, cValue);\n");
    htmlContent += ("chartC.draw(dataC, optionsTempC);}\n");
    //Temp Fahrenheit update
    htmlContent += ("function displayDataF(fValue) {\n");
    htmlContent += ("dataF.setValue(0, 0, '" + String((char)176) + "F');\r\n");//Char 176 is degrees symbol
    htmlContent += ("dataF.setValue(0, 1, fValue);\n");
    htmlContent += ("chartF.draw(dataF, optionsTempF)};\n");
    //Pressure update
    htmlContent += ("function displayDataP(pValue) {\n");
    htmlContent += ("dataP.setValue(0, 0, 'Pressue mb');\n");
    htmlContent += ("dataP.setValue(0, 1, pValue);\n");
    htmlContent += ("chartP.draw(dataP, optionsPressure);}\n");

    //Get new reading using JSON
    htmlContent += ("function loadData() {var c; var f;var p;\r\n");
    htmlContent += ("$.getJSON('/data.json', function(data) {\r\n"); 
    htmlContent += ("c = data.tempC;\r\n");
    htmlContent += ("f = data.tempF;\r\n");
    htmlContent += ("p = data.pressure;\r\n");
    htmlContent += ("if(c){displayDataC(c);}\r\n");
    htmlContent += ("if(f){displayDataF(f);}\r\n");
    htmlContent += ("if(p){displayDataP(p);}\r\n");
    htmlContent += ("});}\r\n");

    //Create a new chart
    htmlContent += ("function initChart() {\n");
    htmlContent += ("chartC = new google.visualization.Gauge(document.getElementById('chart_divC'));\n");
    htmlContent += ("dataC = google.visualization.arrayToDataTable([['Label', 'Value'],['\260C', 80],]);\n");
    htmlContent += ("chartF = new google.visualization.Gauge(document.getElementById('chart_divF'));    \n");
    htmlContent += ("dataF = google.visualization.arrayToDataTable([['Label', 'Value'],['\260F', 66],]);\n");   
    htmlContent += ("chartP = new google.visualization.Gauge(document.getElementById('chart_divP'));\n");
    htmlContent += ("dataP = google.visualization.arrayToDataTable([['Label', 'Value'],['Pressure', 1100],]);\n");   
    htmlContent += ("optionsTempC = {width: 250, height: 250, redFrom: 80, redTo: 100,yellowFrom:60, yellowTo: 80, minorTicks: 5,min: 0, max: 100};\n");
    htmlContent += ("optionsTempF = {width: 250, height: 250, redFrom: 176, redTo: 212,yellowFrom:140, yellowTo: 176, minorTicks: 5,min: 32, max: 212};\n");
    htmlContent += ("optionsPressure = {width: 250, height: 250, min: 0,  max: 10000};\n");
  
    //Call loadData every second
    htmlContent += ("loadData();setInterval(loadData," + String(GAUGE_REFRESH) + ");}</script>\n");
    htmlContent += ("<div id=\"chart_divP\" style=\"float:left; width:300px; height: 300px;\"></div>\n");
    htmlContent += ("<div style=\"clear:both;\"></div>\n");
    htmlContent += ("<div id=\"chart_divC\" style=\"float:left; width:300px; height: 300px;\"></div>\n");
    htmlContent += ("<div id=\"chart_divF\" style=\"float:left; width:300px; height: 300px;\"></div>\n");
    htmlContent += ("</body></html>\n");

    //Send the data to the client
    client.print(htmlContent);
  }//End root
  else if(sPath=="/data.json"){
    htmlContent = ("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n"); 
    htmlContent += ("{\"tempC\":\"" + String(T) + "\",\"tempF\":\"" + String(F) + "\",\"pressure\":\"" + String(P) + "\"}");
    client.print(htmlContent);
  }
  else if (sPath=="/chart.html"){ 
    //Standard html header stuff here.
    htmlContent = ("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n"); 
    htmlContent += ("<html><head><title>chart</title>\n");
  
    //load google charts and create a button
    htmlContent += ("<script type=\"text/javascript\" src=\"https://www.google.com/jsapi?autoload={'modules':[{'name':'visualization','version':'1','packages':['corechart']}]}\"></script>\n");
    htmlContent += ("<button id=\"change-chart\"></button>");
    htmlContent += ("<script type=\"text/javascript\"> google.setOnLoadCallback(drawChart);\n");
    htmlContent += ("var button = document.getElementById('change-chart');");
  
    //Create a function to draw the chart and then add the data into a table
    htmlContent += ("function drawChart() {var data = google.visualization.arrayToDataTable([\n");
    htmlContent += ("['Local Time', 'Temperature C', 'Temperature F', 'Pressure'],\n");
    //Send what we have to the client (web browser)
    client.print(htmlContent);
  
    //Here we loop through the temp and pressure data to place it into the html source code
    for (int i = 0; i< count ; i++){
      htmlContent = ("[new Date(" + String(timeStamp[i]) +  "000)," + String(tempCdata[i]) +  "," + String((9.0/5.0)*tempCdata[i]+32.0) + "," + String(pressData[i]) + "],\n");
      client.print(htmlContent);
    }
    htmlContent = ("]);\n");

    //Continue to build the rest of the web page.  Here we create three function that the buttons uses to dsiplay the chart data.
    htmlContent += ("function drawChartCelsius() {var tempCview = new google.visualization.DataView(data);\n    tempCview.setColumns([0,1]);\n    chart.draw(tempCview, optionsCelsius);\n    button.innerText = 'Change to Fahrenheit';\n    button.onclick = drawChartFahrenheit;}\n");
    htmlContent += ("function drawChartFahrenheit() {var tempFview = new google.visualization.DataView(data);\n    tempFview.setColumns([0,2]);\n    chart.draw(tempFview, optionsFahrenheit);\n    button.innerText = 'Change to Pressure';\n    button.onclick = drawChartPressure;}\n");
    htmlContent += ("function drawChartPressure() {var tempPressureView = new google.visualization.DataView(data);\n    tempPressureView.setColumns([0,3]);\n    chart.draw(tempPressureView, optionsPressure);\n    button.innerText = 'Change to Celsius';\n    button.onclick = drawChartCelsius;}\n");

    //specify date format and then update x labels with this time format
    htmlContent += ("var formatter = new google.visualization.DateFormat({ formatType: 'short',timeZone: 0});\n  formatter.format(data, 0);\n");
    htmlContent += ("// Set X-Axis Labels\nvar xTicks = [];\n");
    htmlContent += ("for (var i = 0; i < data.getNumberOfRows(); i++) {\n");
    htmlContent += ("   xTicks.push({\n    v: data.getValue(i, 0),\n    f: data.getFormattedValue(i, 0) });\n}\n");

    //Here are three chart options used for each chart.  E.g. colour, chart title, etc..
    htmlContent += ("var optionsPressure = {'height': 320,chartArea:{top:20, height:\"60%\"},hAxis:{gridlines:{color:'transparent'},ticks:xTicks,slantedText: true,slantedTextAngle :70,textStyle:{fontSize: 11} },vAxis:{format:\"##,### mb\"},series:{1:{curveType:'function'},0:{color:'orange'}},legend:{position: 'none'},title:'Pressure in Millibars' };\n");
    htmlContent += ("var optionsCelsius = {'height': 320,chartArea:{top:20,  height:\"60%\"},hAxis:{gridlines:{color:'transparent'},ticks:xTicks,slantedText: true,slantedTextAngle :70,textStyle:{fontSize: 11} },vAxis:{format:\"##.## " + String((char)176) + "C\"},series:{1:{curveType:'function'},0:{color:'red'}},legend:{position: 'none'},title:'Temperature in Celsius' };\n");
    htmlContent += ("var optionsFahrenheit = {'height': 320,chartArea:{top:20, height:\"60%\"},hAxis:{gridlines:{color:'transparent'},ticks:xTicks,slantedText: true,slantedTextAngle :70,textStyle:{fontSize: 11} },vAxis:{format:\"##.## " + String((char)176) + "F\"},series:{0:{curveType: 'function'},0:{color:'Blue'}},legend:{position: 'none'},title: 'Temperature in Fahrenheit'};\n");
    client.print(htmlContent);    
  
    //Draw chart 
    htmlContent = ("var chart = new google.visualization.LineChart(document.getElementById('curve_chart'));drawChartCelsius();}\n");
    htmlContent += ("</script>\n");

    //Page heading
    htmlContent += ("<font color=\"#000000\"><body><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">\n<h1>Temperature and Pressure Chart</h1><a href=\"/\">back</a><BR><BR>\n");
    htmlContent += ("<div id=\"curve_chart\" style=\"width: 800px; height: 300px\"></div><BR><BR>Number of readings=" + String(count) + "<BR>Max allowed readings=" + String(numberOfRows) + "<BR>");

    //Display the data and time for first and last reading
    htmlContent += ("<BR><BR>First reading at : ");
    timeAndDate(timeStamp[0],htmlContent);    
    htmlContent += ("<BR>Most recent reading : ");
    timeAndDate(timeStamp[count-1],htmlContent);     
    htmlContent += ("<BR></body></html>\n");
    client.print(htmlContent);
  }//End chart
  else if(sPath=="/table.html") {
    //Build web page to show all data in a table
 
    //Standard html header stuff here.
    htmlContent = ("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n"); 
    htmlContent += ("<html><head><title>Table</title>\n");
    htmlContent += ("<a href=\"/\">back</a><BR><BR>\n");
    
    //load google charts object
    htmlContent += ("<script type=\"text/javascript\" src=\"https://www.gstatic.com/charts/loader.js\"></script>\n");
    htmlContent += ("<script type=\"text/javascript\">\n");
    htmlContent += ("   google.charts.load('current', {'packages' : ['table']});\n");
    htmlContent += (" google.charts.setOnLoadCallback(drawTable);\n");

    //Create a function to draw the table and then add the all the data
    htmlContent += ("function drawTable() {var data = google.visualization.arrayToDataTable([\n");
    htmlContent += ("['Local Time', 'Temperature C', 'Temperature F', 'Pressure'],\n");
    client.print(htmlContent);
    //Here we loop through the temp and pressure data to place it into the html source code
    for (int i = 0; i< count ; i++){
      htmlContent = ("[new Date(" + String(timeStamp[i]) +  "000)," + String(tempCdata[i]) +  "," + String((9.0/5.0)*tempCdata[i]+32.0) + "," + String(pressData[i]) + "],\n");
      client.print(htmlContent);
    }
    htmlContent = ("]);\n");

    //Create the table object then draw it
    htmlContent += ("var table = new google.visualization.Table(document.getElementById('table_div')); table.draw(data, {showRowNumber: true,  height: '100%'});}\n");
    htmlContent += ("</script></head><body>\n");

    //Page heading
    htmlContent += ("<font color=\"#000000\"><body><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\"><h1>Temperature and Pressure Table</h1><a href=\"/\">back</a><BR><BR>\n");
    htmlContent += ("<div id=\"table_div\" style=\"width: 100%; height: 700px\"></div><BR><BR>Number of readings=" + String(count) + "<BR>Max allowed readings=" + String(numberOfRows) + "<BR>");
    
    //Display the data and time for first and last reading
    htmlContent += ("<BR><BR>First reading at : ");
    timeAndDate(timeStamp[0],htmlContent);    
    htmlContent += ("<BR>Most recent reading : ");
    timeAndDate(timeStamp[count-1],htmlContent);     
    htmlContent += ("<BR></body></html>\n");
    client.print(htmlContent);
  }//End table


  //Read temperature
  status = startTemperature();
  if (status != 0){
    delay(status);
    getTemperature(T);
    Serial.print("temperature: ");
    Serial.print(T,2);
    Serial.print(" deg C, ");
    F = (9.0/5.0)*T+32.0;
    Serial.print(F,2);
    Serial.print(" deg F   ");
  }

  //Read pressure
  status = startPressure(3);
  if (status != 0){
    // Wait for the measurement to complete:
    delay(status);
    status = getPressure(P,T);
    if (status != 0){
      // Print out the measurement:
      Serial.print("absolute pressure: ");
      Serial.print(P,2);
      Serial.print(" mb, ");
      Serial.print(P*0.0295333727,2);
      Serial.print(" inHg   ");

    }
   }

   
   
   

  // Check if poll period has expired
  if (millis()>=timeKeeper){
    timeKeeper = millis() + (POLLPERIOD * 1000);   // Update timeKeeper with the latest time + poll period (multiply by 1,000 as millis() is milliseconds)
    unsigned long currentTime = now();
    // Update each row in the array until it is full
    if(!(isnan(currentTime)) || !(isnan(T))|| !(isnan(P))){  // Make sure that all values are a number before updating
      if (count < numberOfRows){
          tempCdata[count] = T;   // Temperature
          pressData[count] = P;   // Pressure
          timeStamp[count] = currentTime;  //Current time
          count++;
        }
      else{
        for (int i = 0; i<(count) ; i++){          // Cycle the array. Move everything forward by one and add the new values to the end
          tempCdata[i] = tempCdata[i+1];
          pressData[i] = pressData[i+1];
          timeStamp[i] = timeStamp[i+1];
        }
        tempCdata[numberOfRows] = T;
        pressData[numberOfRows] = P;
        timeStamp[numberOfRows] = currentTime;
       }
    }
  }
}//End of main loop






// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011; // LI, Version, Mode
  packetBuffer[1] = 0;          // Stratum, or type of clock
  packetBuffer[2] = 6;          // Polling Interval
  packetBuffer[3] = 0xEC;       // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  Udp.beginPacket(address, 123);
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

time_t getNtpTime()
{
  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP request");
  sendNTPpacket(timeServer);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }

  Serial.println("No NTP response");
  return 0;
}





void printDigits(Print *p, int digits) {
  p->print(":");
  if(digits < 10) {
    p->print('0');
  }
  p->print(digits);
}

void dumpClock(Print *p) {
  p->print(hour());
  printDigits(p, minute());
  printDigits(p, second());
  p->print(" ");
  p->print(day());
  p->print(".");
  p->print(month());
  p->print(".");
  p->print(year()); 
  p->println(); 
}


//Helper function to convert UNIX time to string
void timeAndDate (time_t tt, String& htmlContent ){
  if (hour(tt) < 10) // If hour is less than ten, it will only be one character. This would mean we would need to place a '0' in front of the character.
    htmlContent += ("0" + String(hour(tt))+ ":");
  else
    htmlContent += (String(hour(tt)) + ":");
 if (minute(tt) < 10)
    htmlContent += ("0" + String(minute(tt)) + ":");
 else
    htmlContent += (String(minute(tt)) + ":");
 if (second(tt) < 10)
   htmlContent += ("0" + String(second(tt)) + " ");
 else
   htmlContent += (String(second(tt)) + " ");
 
 if (day(tt) < 10)
   htmlContent += ("0" + String(day(tt))+ "/");
 else
  htmlContent += (String(day(tt)) + "/");
 if (month(tt) < 10)
    htmlContent += ("0" + String(month(tt))+ "/");
  else
    htmlContent += (String(month(tt)) + "/");
  htmlContent += (String(year(tt)));
}


void allocateRam(){
  //We will allocate as much RAM as we can to track historical data. This means, we need to work out how much RAM we will needed for the TCP stack and enough for the ESP8266 to run. 
  //Once we get this value, we can allocate the rest of the RAM to the array used to store the temperature and pressure values.
  //
  //Each TCP session expires every 2mins. (Default MSL is 2mins for Windows https://technet.microsoft.com/en-us/library/cc938217.aspx)
  //If using the refresh function on the main page with gauges, then there needs to be enough RAM left to accommodate the TCP sessions from the refresh as well as a little extra for the ESP8266 to run.
  //196KB RAM per refresh. 
  //To calculate the amount to leave free;
  // (number of refreshes in 2 minutes x RAM per REFRESH) + SPARE RAM = ALLOCATED_RAM
  // 
  //E.g.
  //For a refresh of every 1 second
  //(120x196KB) + 10,000 = 33,520
  //For a refresh of every 3 seconds
  //(40x196KB) + 10,000 = 17,840
  //
  //The less frequent the refresh will result in a smaller value needed to be reserved. And this also mean more RAM can be used to store historical data for a longer period.
  //If refresh isnt used, then leave ALLOCATED_RAM to 10,000

  
  //Get free RAM in bytes
  uint32_t free=system_get_free_heap_size() - ALLOCATED_RAM;

  
  //Divide the free RAM by the size of the variables used to store the data. 
  //This will allow us to work out the maximum number of records we can store. 
  //All while keeping some RAM free which is specified in ALLOCATED_RAM
  numberOfRows = free / (sizeof(float)*2+ sizeof(unsigned long)); // 2 x float for temp and pressure.  Long for time. 

  
  
  //re-declare the arrays with the number of elements
  tempCdata = new float [numberOfRows];
  pressData = new float [numberOfRows];
  timeStamp = new unsigned long [numberOfRows];



  if ( timeStamp==NULL || tempCdata==NULL || pressData==NULL)
  {
    numberOfRows=0;
    Serial.println("Error in memory allocation!");
  }
  else
  {
    Serial.print("Allocated storage for ");
    Serial.print(numberOfRows);
    Serial.println(" data points.");
    
    Serial.print("Graph poll period in sec:");
    Serial.println(POLLPERIOD);

    Serial.print("This will equal "); Serial.print((POLLPERIOD*numberOfRows)/60); Serial.println(" minutes of historical data");
  }
  delay(2000);
}

