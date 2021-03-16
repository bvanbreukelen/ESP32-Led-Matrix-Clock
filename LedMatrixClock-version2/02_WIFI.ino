/*
   Functions concerning WIFI
*/

boolean connectToNetwork(String s, String p ) {
  const char* ssid = s.c_str();
  const char* password = p.c_str();

  Serial.print("ACCESSING WIFI: "); Serial.println(ssid);
  WiFi.begin(ssid, password);

  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(2000);
    Serial.println("Connecting to WiFi..");
    if (timeout == 5) {
      return false;
      break;
    }
    timeout++;
  }
  //Serial.println(WiFi.localIP());
  return true;
} // network()

/*
   A function to show all available NETWORKS and returns this as a HTML formatted STRING
*/
String scanNetworks() {
  // scan for nearby networks:
  String networkList = "<h3> == Available Networks == </h2>";
  byte numSsid = WiFi.scanNetworks();
  for (int thisNet = 0; thisNet < numSsid; thisNet++) {
    networkList += "<br/> ";
    networkList += thisNet;
    networkList += " Network: ";
    networkList += WiFi.SSID(thisNet);
  }
  return networkList;
} // scanNetworks()

/* Function to run in ACCESSPOINT MODE */
// RunAPmode code
void RunAPmode( void * parameter) {
  WiFi.disconnect(true);                                                      // End All connections.
  AsyncWebServer server(80);
  WiFi.softAP(ssidAP, passwordAP);                                            // Start ACCESSPOINT MODE with basic credantials
  IPAddress IP = WiFi.softAPIP();                                             // GET THE ACCESSPOINT IP
  Serial.print("The IP of the accesspoint is: ");                             // SHOW IP IN SERIAL MONITOR
  Serial.println(WiFi.localIP());
  preferences.begin("wificreds", false);                                      // Make sure we have something to store our preferences in
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {              // The Home page so to say.
    request->send_P(200, "text/html", INDEXAP_HTML, processor);
  });

  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest * request) {          // WHEN SOMEONE SUBMITS SOMETHING.... Like the credentials :)
    String inputMessage;
    const char* PARAM_WIFI = "SSIDname";
    const char* PARAM_PWD = "SSIDpwd";
    preferences.begin("wificreds", false);

    if (request->hasParam(PARAM_WIFI)) {
      inputMessage = request->getParam(PARAM_WIFI)->value();
      preferences.putString("ssid", inputMessage);
    }
    else if (request->hasParam(PARAM_PWD)) {
      inputMessage = request->getParam(PARAM_PWD)->value();
      preferences.putString("password", inputMessage);
    }
    else {
      inputMessage = "Restarting and usiing new credentials";
      ESP.restart();
    }
    Serial.println(inputMessage);                                              // This prints the submitted variable on the serial monitor.. as a check
    request->send(200, "text/text", inputMessage);
  });
  server.onNotFound(notFound);
  server.begin();
  Serial.print("AP mode runs on core: "); Serial.println(xPortGetCoreID());

  for (;;) {
    preferences.begin("wificreds", false);
    delay(5000);
    Serial.println(preferences.getString("ssid"));
    Serial.println(preferences.getString("password"));
    preferences.end();
  }
}

/*
   Find and replace method, to inject variables into a HTML page
*/
String processor(const String & var) {
  //String "#";
  if (var == "wifilist") {
    return scanNetworks();
  }
  else if (var == "effect") {
    // return readFile(SPIFFS, "/inputInt.txt");
    return whichFX;
  }
  else if (var == "digitOnOff") {
    return digitAnimation;
  }
  else if (var == "digitHTMLcol") {
    return digitHTMLcol;
  }
  else if (var == "BGHTMLcol") {
    return BGHTMLcol;
  }
  else if (var == "nightMode") {
    if (nightMode) {
      return "On";
    } else
      return  "Off";
  }
  else if (var == "nightTimeHour") {
    return String(nightTimeHour);
  }
  else if (var == "nightTimeMinute") {
    return String(nightTimeMinute);
  }
  else if (var == "morningTimeHour") {
    return String(morningTimeHour);
  }
  else if (var == "morningTimeMinute") {
    return String(morningTimeMinute);
  }
  else if (var == "overAllBrightness") {
    return String(overAllBrightness);
  }
  else if (var == "backgroundBrightness") {
    return String(backgroundBrightness);
  }
  else if (var == "digitBrightness") {
    return String(digitBrightness);
  }
  else if (var == "iRebooted") {
    return String(iRebooted);
  }
  else if (var == "randomAnimation") {
    return String(randomAnimation);
  }
  else if (var == "randomWhat") {
    return String(randomWhat);
  }
  else if (var == "scrolltext") {
    return String(scrolltext);
  }
  return String();
}

/*
   Sometimes a page just doesn't exists.. we need to tell them
*/
void notFound(AsyncWebServerRequest * request) {
  request->send(404, "text/plain", "Not found");
} //runAPmode()


void displayIP() {
  std::map<int, CRGB> test;
  CRGB dcolor = CRGB::Red;
  test = makeDigits(letterMatrix[9], test, -2, 0, true, dcolor);
  test = makeDigits(letterMatrix[16], test, 3, 0, false, dcolor);
  displayTimeNoAnimation(test);
  FastLED.show();
  FastLED.delay(200);

  String ip = WiFi.localIP().toString();
  String iplast0 = ip.substring(ip.length() - 1);
  String iplast1 = ip.substring(ip.length() - 2, ip.length() - 1);
  String iplast2 = ip.substring(ip.length() - 3, ip.length() - 2);

  if (iplast2 == ".") { // a two digit IP
    test = makeDigits(numberMatrix[iplast1.toInt()], test, 10, 0, false, dcolor);
    test = makeDigits(numberMatrix[iplast0.toInt()], test, 17, 0, false, dcolor);
  } else if (iplast1 == ".") { // a one digit ip
    test = makeDigits(numberMatrix[iplast0.toInt()], test, 17, 0, false, dcolor);
  }
  else { // a 3 digit IP
    test = makeDigits(numberMatrix[iplast2.toInt()], test, 10, 0, false, dcolor);
    test = makeDigits(numberMatrix[iplast1.toInt()], test, 17, 0, false, dcolor);
    test = makeDigits(numberMatrix[iplast0.toInt()], test, 24, 0, false, dcolor);
  }
  displayTimeNoAnimation(test);
  FastLED.show();
  FastLED.delay(4000);

}


/*
   This function runs the main webpage and handles all webtrafic.
   New settings will be stored in the globals AND in memory
*/
void RunWebserver( void * parameter) {
  AsyncWebServer server(80);                                // Start the webserver
  Serial.print("The IP of ledclock: ");                     // SHOW IP IN SERIAL MONITOR
  Serial.println(WiFi.localIP());
  Serial.print("The wifi server runs on core: ");
  Serial.println(xPortGetCoreID());                         // Webserver should run on second core (0)

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", INDEX_HTML, processor);
  });

  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest * request) {
    preferences.end();                                      // In case preferences was still 'open' but in a different 'domain'
    String inputMessage;
    preferences.begin("matrixsettings", false);             // Open the matrixsettings preferences 'domain'

    /*
       The website will send the parameters that the user selects
       This part catches the paramters and stores them in preferences.
       This can later be picked-up by the rest of the code.
       These user parameters also will to be put in the GLOBALS.
    */
    if (request->hasParam("effect")) {
      inputMessage = request->getParam("effect")->value();
      preferences.putString("effect", inputMessage);
      //writeFile(SPIFFS, "/inputString.txt", inputMessage.c_str());
      whichFX = inputMessage;
    }
    else if (request->hasParam("digitOnOff")) {
      inputMessage = request->getParam("digitOnOff")->value();
      preferences.putString("digitOnOff", inputMessage);
      digitAnimation = inputMessage;
    }
    else if (request->hasParam("inputDigitCol")) {
      inputMessage = request->getParam("inputDigitCol")->value();
      //preferences.putString("inputDigitCol", inputMessage);
      digitColor = strtol(inputMessage.substring(1).c_str(), NULL, 16);
      Serial.print("submitted color digit long: "); Serial.println(digitColor);
      preferences.putLong("inputDigitCol", digitColor);
      preferences.putString("digitHTMLcol", inputMessage);
      digitHTMLcol = inputMessage;
    }
    else if (request->hasParam("inputBGCol")) {
      inputMessage = request->getParam("inputBGCol")->value();
      backgroundColor = strtol(inputMessage.substring(1).c_str(), NULL, 16);
      preferences.putLong("inputBGCol", backgroundColor);
      preferences.putString("BGHTMLcol", inputMessage);
      BGHTMLcol = inputMessage;
    }
    else if (request->hasParam("nightMode")) {
      inputMessage = request->getParam("nightMode")->value();
      preferences.putString("nightMode", inputMessage);
      if (inputMessage == "On") {
        nightMode = true;
      } else {
        nightMode = false;
      }
    }
    else if (
      request->hasParam("nightTimeHour")) {
      inputMessage = request->getParam("nightTimeHour")->value();
      nightTimeHour = inputMessage.toInt(); //store globally
      preferences.putUInt("nightTimeHour", inputMessage.toInt());
    }
    else if (
      request->hasParam("nightTimeMinute")) {
      inputMessage = request->getParam("nightTimeMinute")->value();
      nightTimeMinute = inputMessage.toInt(); //store globally
      preferences.putUInt("nightTimeMinute", inputMessage.toInt());
    }
    else if (
      request->hasParam("morningTimeHour")) {
      inputMessage = request->getParam("morningTimeHour")->value();
      morningTimeHour = inputMessage.toInt(); //store globally
      preferences.putUInt("morningTimeHour", inputMessage.toInt());
    }
    else if (
      request->hasParam("morningTimeMinute")) {
      inputMessage = request->getParam("morningTimeMinute")->value();
      morningTimeMinute = inputMessage.toInt(); //store globally
      preferences.putUInt("morningTimeMinute", inputMessage.toInt());
    }
    else if (
      request->hasParam("overAllBrightness")) {
      inputMessage = request->getParam("overAllBrightness")->value();
      overAllBrightness = inputMessage.toInt();
      preferences.putUInt("oaBrightness", overAllBrightness);
    }
    else if (
      request->hasParam("backgroundBrightness")) {
      inputMessage = request->getParam("backgroundBrightness")->value();
      backgroundBrightness = inputMessage.toInt();
      preferences.putUInt("bgBrightness", backgroundBrightness);
    }
    else if (
      request->hasParam("digitBrightness")) {
      inputMessage = request->getParam("digitBrightness")->value();
      preferences.putUInt("fgBrightness", inputMessage.toInt());
      digitBrightness = inputMessage.toInt();
    }
    else if ( // randomAnimation
      request->hasParam("randomAnimation")) {
      inputMessage = request->getParam("randomAnimation")->value();
      preferences.putString("randomAnimation", inputMessage);
      randomAnimation = inputMessage;
    }
    else if (
      request->hasParam("scrolltext")) {
      inputMessage = request->getParam("scrolltext")->value();
      preferences.putString("scrolltext", inputMessage);
      scrolltext = inputMessage;
    }
    else if (
      request->hasParam("randomWhat")) {
      inputMessage = request->getParam("randomWhat")->value();
      preferences.putString("randomWhat", inputMessage);
      randomWhat = inputMessage;
    }
    else
    {
      inputMessage = "No message sent";
    }
    Serial.print("New variable received: "); Serial.println(inputMessage);
    request->send(200, "text/text", inputMessage);
    preferences.end();                                          // don't leave the door open :) always close when you leave.
  });

  server.onNotFound(notFound);
  server.begin();

  for (;;) {

  }
}