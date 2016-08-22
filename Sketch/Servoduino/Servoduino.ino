#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h> 
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiManager.h>
#include <Servo.h>
#include <WiFiUdp.h>
#include <EEPROM.h>

struct oconfig {
  int bValidConfig1;
  int bValidConfig2;
  int iConfigVersion;
  int iAn;
  int iAus;
  int iKorrektur;
  int bSwitchAsPushButton[2];
  int iSwitchTimer[2];
  int bPushButtonTimer[2];
  int iPushButtonTimer[2];
  int bMagnetsensor;
  int iAnalogSchwelle;
  int bHomematic;
  char sHomematicIP[16];
  char sHomematicGeraet[20];
  char sHomematicGeraetRelay[20];
  int bLoxone;
  char sLoxoneIP[16];
  char sLoxonePort[20];  
  int bIP;
  char sIP[16];
  char sSubNet[16];
  char sGateway[16]; 
};
typedef struct oconfig oConfig;

unsigned int localPort = 2390;      // local port to listen on

char packetBuffer[255]; //buffer to hold incoming packet
char  ReplyBuffer[] = "acknowledged";       // a string to send back

WiFiManager wifiManager;
ESP8266WebServer server(80); // Webserver initialisieren auf Port 80
Servo myservo;
ESP8266HTTPUpdateServer httpUpdater;
WiFiUDP Udp;


oConfig myConfig = {1234, 4321, 2, 0, 180, 10, {0, 0}, {60, 60}, {0, 0}, {120, 120}, 0 , 1000, 0, "000.000.000.000", "Seriennummer","Seriennummer", 0, "000.000.000.000", "7000" , 0, "000.000.000.000", "000.000.000.000", "000.000.000.000"};

//Version
//$LastChangedDate: 2016-08-16 19:12:43 +0200 (Di, 16 Aug 2016) $
//$Revision: 48 $
String sVersionDatum = "2016-08-19";
String sVersion = "0.7";

//Pin Definitionen
int iServoPin = 2; //WeMos mini D4
int iPushButtonPin[] = {14, 12}  ; //WeMos mini D5 und D6
int iSwitchPin[] = {13, 15}; //WeMos mini D7 und D8
int iRelayPin = 5; //Wemos mini D1 -> RelayBoard
int iMagnetsensorPin = 4; //Wemos mini D2 - MagnetSensor
// Analog Pin auf Wemos A0 => int sensorValue = analogRead(A0); 


// OTA
const char* update_path = "/firmware";
const char* update_username = "admin";
const char* update_password = "admin";

int iUeber = 0; //notwendiger "negatiever" Korrekturfaktor fue den Bereich ueber 180-iKorrektur.

int iPushButtonLastState[] = { 2, 2 };
unsigned long tPushButtonLast[2]; //letzter Tastendruck
int iSwitchLastState[] = { 2, 2 };
unsigned long tSwitchLast[2]; //letzte Schalterbetätigung

int iAnalogLastState = 0;
int iAnaloglastMillis = millis();

void saveConfig() {
  int eeAddress = 0;
  EEPROM.begin(256);
  EEPROM.put(eeAddress, myConfig);
  EEPROM.end();

//EEPROM.begin(0);
//Serial.println("__________ Save Config ________");
//EEPROM_writeAnything(0, tmpConfig);
//EEPROM.end();
}


int getKorrigiert (int iGrad) {
  int iErg = iGrad;
  iErg = iErg + myConfig.iKorrektur;
  iUeber=0;
  if (iErg > 180) { 
    iUeber=iErg-180;
    if (iUeber>myConfig.iKorrektur) iUeber=myConfig.iKorrektur;
    iErg=180;
  }
  if (iErg < 0) iErg=0;
  // Serial.println("getKorrigiert - Input: " + String(iGrad) + " Rueckgabe: "+ String(iErg));
  return iErg;
}

int getStatus() {
  int iErg = myservo.read();
  iErg=iErg - myConfig.iKorrektur;
  if (iErg > 180) iErg=180;
  if (iErg < 0) iErg=0;
  //Serial.println("getKorrigiert - ServoPos: " + String( myservo.read()) + " Rueckgabe: "+ String(iErg));
  return iErg+iUeber;
}

int getOnOffStatus() {
    int iErg=-1;
    if (myConfig.bMagnetsensor==1) {
    if (digitalRead(iMagnetsensorPin) == HIGH) {
      iErg=1;
    } else {
      iErg=0;
    }
  } else {    
    int iStatus = getStatus();
    if (iStatus == myConfig.iAn) iErg=1; 
    if (iStatus == myConfig.iAus) iErg=0; 
  }
  return iErg;
}

// Rückmeldungen an Loxone

void sendUDP(String sUDP) {
    if (myConfig.bLoxone != 1) {return;}
    int iPort =  atoi(myConfig.sLoxonePort); 
    Serial.print("Anzahl Zeichen:");
    Serial.println(sizeof(sUDP)+2);
    char  sBuffer[sizeof(sUDP)] = "";
    sUDP.toCharArray(sBuffer,sizeof(sUDP)+2);
    Serial.print("Sende an Loxone: ");
    Serial.println(sBuffer); 
    Udp.beginPacket(myConfig.sLoxoneIP, iPort);
    Udp.write(sBuffer);
    Udp.endPacket(); 
}
void sendTabState2Loxone() {
//    if (myConfig.bLoxone != 1) {return;}
//    int iPort =  atoi(myConfig.sLoxonePort); 
//    int iStatus = getOnOffStatus();
//    char  sBuffer[6] = "";
//    String("TAB" + String(iStatus)).toCharArray(sBuffer,6);
//    Serial.print("Sende an Loxone: ");
//    Serial.println(sBuffer); 
//    Udp.beginPacket(myConfig.sLoxoneIP, iPort);
//    Udp.write(sBuffer);
//    Udp.endPacket();
    if (myConfig.bLoxone != 1) {return;}
    int iStatus = getOnOffStatus();
    sendUDP("TAB" + String(iStatus));
}
void sendRelayState2Loxone() {
    if (myConfig.bLoxone != 1) {return;}
//    int iPort  = atoi(myConfig.sLoxonePort); 
//    int iStatus = digitalRead(iRelayPin);
//    char  sBuffer[8] = "";
//    String("RELAY" + String(iStatus)).toCharArray(sBuffer,8); 
//    Serial.print("Sende an Loxone: ");
//    Serial.println(sBuffer); 
//    Udp.beginPacket(myConfig.sLoxoneIP, iPort);
//    Udp.write(sBuffer);
//    Udp.endPacket();
    int iStatus = digitalRead(iRelayPin);
    sendUDP("Relay" + String(iStatus));
}
// Rückmeldungen an Homematic
void sendHTML(String sURL) {
  HTTPClient http;
  Serial.print("[HTTP] begin...\n");
  Serial.println(sURL);
  http.begin(sURL);
  int httpCode = http.GET();
  if(httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if(httpCode == HTTP_CODE_OK) {
          String payload = http.getString();
          Serial.println(payload);
      }
  } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();

}

void sendTabState2HM(){
  if (myConfig.bHomematic != 1) {return;}
  String sURL ="http://";
  sURL += myConfig.sHomematicIP;
  sURL += ":8181/cuxd.exe?x=dom.GetObject(\"CUxD.";
  sURL += myConfig.sHomematicGeraet;
  sURL +=".SET_STATE\").State(";
  sURL += String(getOnOffStatus());
  sURL += ")";
  sendHTML(sURL);  
}

void sendRelayState2HM(){
  if (myConfig.bHomematic != 1) {return;}
  String sURL ="http://";
  sURL += myConfig.sHomematicIP;
  sURL += ":8181/cuxd.exe?x=dom.GetObject(\"CUxD.";
  sURL += myConfig.sHomematicGeraetRelay;
  sURL +=".SET_STATE\").State(";
  sURL +=  digitalRead(iRelayPin);
  sURL += ")";
  sendHTML(sURL);  
}

void sendTabState() {
  sendTabState2Loxone();
  sendTabState2HM();  
}

void sendRelayState() {
  sendRelayState2Loxone();
  sendRelayState2HM();
}

void sendState() {
  sendTabState();
  sendRelayState();
}

void wwwmelde_status() {
  int iErg = getOnOffStatus();
  server.send(200, "text/plain", String(iErg));
}

void wwwmelde_FullStatus() {
  int iErg = getStatus();
  int iAnAusStatus = getOnOffStatus();
  int iRelay = digitalRead(iRelayPin);
  int sensorValue = analogRead(A0);
  String sResp = "";
  sResp += "<table><tr>";
  sResp += "<td>Aktueller Status: </td>";
  if (iAnAusStatus == 1) { sResp += "<td class=\"on\">\n";}
  if (iAnAusStatus == 0) { sResp += "<td class=\"off\">\n";}
  if (iAnAusStatus == -1) { sResp += "<td>\n";}

  sResp += String(iErg) + "&deg;</td>\n";
  sResp += "</tr><tr>";
  sResp += "<td>Aktueller Relay Status: </td>";
  if (iRelay == 1) { sResp += "<td class=\"on\">An</td>\n";}
  if (iRelay == 0) { sResp += "<td class=\"off\">Aus</td>\n";}
  sResp += "</tr><tr>";
  sResp += "<td>Analog Wert:      </td><td>" + String(sensorValue) + "</td>\n";
  sResp += "</tr></table>";
  server.send(200, "text/plain", sResp);
}

void htmlreloadpresite(String sTarget, String sText="") {
    String sResponse;
    Serial.println("htmlreload begin");
    sResponse  = "<html><head><title>";
    sResponse += "GoBack";
    sResponse += "</title></head>";
    sResponse += "<body>";
    sResponse += "<meta http-equiv=\"refresh\" content=\"2;URL=";
    sResponse += sTarget; 
    sResponse += "\">";
    sResponse += "<body>";
    sResponse += sText;
    sResponse += "</body></html>";
    Serial.println("htmlreload write to client " + sTarget);
    server.send(200, "text/html", sResponse);
    Serial.println("htmlreload end");
}

void htmlresponse0(String sText = "") {
    String sResponse;
    Serial.println("htmlresponse0");
    server.send(200, "text/plain", sText);
    Serial.println("htmlresponse0 end");
}

void goPos() {
  if (server.arg("pos")!="") {
//  String sStatus = server.arg("pos");
//  iStatus=sStatus.toInt();    
  int iStatus = server.arg("pos").toInt();
  if (iStatus<0) { iStatus = 0; }
  if (iStatus>360) { iStatus = 360; }
   
  myservo.write(getKorrigiert(iStatus));
  htmlresponse0(String(getStatus()));
  }
}

void wwwrelayan() {
  String bReload = server.arg("reload");
  digitalWrite(iRelayPin, HIGH);
  if (bReload == "1") {
    Serial.println("Zur Root Seite weiterleiten");
    htmlreloadpresite("/","AN");
  }
  else {
    Serial.println("NICHT weiterleiten");
    htmlresponse0("1");
  }
    sendUDP("RWeb1");
    sendRelayState();  
}

void wwwrelayaus() {
  String bReload = server.arg("reload");
  digitalWrite(iRelayPin, LOW);
  if (bReload == "1") {
    Serial.println("Zur Root Seite weiterleiten");
    htmlreloadpresite("/","AUS");
  }
  else {
    Serial.println("NICHT weiterleiten");
    htmlresponse0("0");
  }
  sendUDP("RWeb0");
  sendRelayState(); 
}

void wwwan() {
  String bReload = server.arg("reload");
  Serial.println("Reload? = " + bReload);
  int iStatus=myConfig.iAn;
  myservo.write(getKorrigiert(iStatus));
  if (bReload == "1") {
    Serial.println("Zur Root Seite weiterleiten");
    htmlreloadpresite("/","AN");
  }
  else {
    Serial.println("NICHT weiterleiten");
    htmlresponse0("1");
  }
  sendUDP("Web1");
  sendTabState();  
}

void wwwaus() {
  String bReload = server.arg("reload");
  Serial.println("Reload? = " + bReload);
  int iStatus=myConfig.iAus;
  myservo.write(getKorrigiert(iStatus));
  if (bReload == "1") {
    Serial.println("Zur Root Seite weiterleiten");
    htmlreloadpresite("/","AUS");
  }
  else {
    Serial.println("NICHT weiterleiten");
    htmlresponse0("0");
  }
  sendUDP("Web0");
  sendTabState();  
}

void  wwwanwert() {
    int sensorValue = analogRead(A0);
    htmlresponse0(String(sensorValue));  
}


String htmlSwitch(String sName, int bStatus) {
  String sReturn;
  String sStatus = "";
  if (bStatus==1) {
    sStatus="checked";
  }
  sReturn = "<label class=\"switch\">";
  sReturn += "<input type=\"checkbox\" name=\""+sName+"\" "+ sStatus +">";
  sReturn += "<div class=\"slider round\"></div>";
  sReturn += "</label>";
  return(sReturn);
}

String cssSwitch() {
  String sReturn;
  sReturn = "  /* The switch - the box around the slider */\n";
  sReturn += ".switch {\n";
  sReturn += "  position: relative;\n";
  sReturn += "  display: inline-block;\n";
  sReturn += "  width: 60px;\n";
  sReturn += "  height: 34px;\n";
  sReturn += "}\n";
  
  sReturn += "/* Hide default HTML checkbox */\n";
  sReturn += ".switch input {display:none;}\n";
  
  sReturn += "/* The slider */\n";
  sReturn += ".slider {\n";
  sReturn += "  position: absolute;\n";
  sReturn += "  cursor: pointer;\n";
  sReturn += "  top: 0;\n";
  sReturn += "  left: 0;\n";
  sReturn += "  right: 0;\n";
  sReturn += "  bottom: 0;\n";
  sReturn += "  background-color: #ccc;\n";
  sReturn += "  -webkit-transition: .4s;\n";
  sReturn += "  transition: .4s;\n";
  sReturn += "}\n";
  
  sReturn += ".slider:before {\n";
  sReturn += "  position: absolute;\n";
  sReturn += "  content: \"\";\n";
  sReturn += "  height: 26px;\n";
  sReturn += "  width: 26px;\n";
  sReturn += "  left: 4px;\n";
  sReturn += "  bottom: 4px;\n";
  sReturn += "  background-color: white;\n";
  sReturn += "  -webkit-transition: .4s;\n";
  sReturn += "  transition: .4s;\n";
  sReturn += "}\n";
  
  sReturn += "input:checked + .slider {\n";
  sReturn += "  background-color: #2196F3;\n";
  sReturn += "}\n";
  
  sReturn += "input:focus + .slider {\n";
  sReturn += "  box-shadow: 0 0 1px #2196F3;\n";
  sReturn += "}\n";
  
  sReturn += "input:checked + .slider:before {\n";
  sReturn += "  -webkit-transform: translateX(26px);\n";
  sReturn += "  -ms-transform: translateX(26px);\n";
  sReturn += "  transform: translateX(26px);\n";
  sReturn += "}\n";
  
  sReturn += "/* Rounded sliders */\n";
  sReturn += ".slider.round {\n";
  sReturn += "  border-radius: 34px;\n";
  sReturn += "}\n";
  
  sReturn += ".slider.round:before {\n";
  sReturn += "  border-radius: 50%;\n";
  sReturn += "}\n";

  sReturn += "P { text-align: center }";

  
  return(sReturn);
}


String cssForm() {
  String sReturn = "";
  sReturn += "  H1 {\n";
  sReturn += "    background-color: lightgray;\n";
  sReturn += "    border-radius: 5px;\n";
  sReturn += "    padding: 5px;\n";
  sReturn += "    clear: both;\n";
  sReturn += "    width: 350px;\n";
  sReturn += "    }\n";
  sReturn += "  table {\n";
  sReturn += "    border-radius: 5px;\n";
  sReturn += "    padding: 5px;\n";
  sReturn += "    clear: both;\n";
  sReturn += "    }\n";
  sReturn += "  table.gray {\n";
  sReturn += "    background-color: lightgray;\n";
  sReturn += "    border-radius: 5px;\n";
  sReturn += "    padding: 5px;\n";
  sReturn += "    clear: both;\n";
  sReturn += "    }\n";
  sReturn += "  tr.hidden {\n";
  sReturn += "    background-color: lightyellow;\n";
  sReturn += "    padding: 5px;\n";
  sReturn += "    margin:5px;\n";
  sReturn += "    clear: both;\n";
  sReturn += "    }\n";
  sReturn += "  table.fixed {table-layout:fixed; width:300px;}";
  sReturn += "  table.fixed td {overflow:hidden;}";
  sReturn += "  table.fixed td:nth-of-type(1) {width:100px;}";
  sReturn += "  table.fixed td:nth-of-type(2) {width:200px;}";
  sReturn += "  td.first {\n";
  sReturn += "    width: 100px;\n";
  sReturn += "    width: 100px;\n";  
  sReturn += "    }\n";
  sReturn += "  td.second {\n";
  sReturn += "    width: 200px;\n";
  sReturn += "    }\n";
  sReturn += "  table.HM {\n";
  sReturn += "    background-color: lightblue;\n";
  sReturn += "    border-radius: 5px;\n";
  sReturn += "    padding: 5px;\n";
  sReturn += "    clear: both;\n";
  sReturn += "    }\n";
  sReturn += "  table.Loxone {\n";
  sReturn += "    background-color: lightgreen;\n";
  sReturn += "    border-radius: 5px;\n";
  sReturn += "    padding: 5px;\n";
  sReturn += "    clear: both;\n";
  sReturn += "    }\n";
  sReturn += "  td.on {background-color: lightgreen;}\n";
  sReturn += "  td.off {background-color: #ff8080;}\n";
  sReturn += "  p.sys2inf {\n";
  sReturn += "    border-style: none none solid none;\n";
  sReturn += "    border-width: 1px;\n";
  sReturn += "    border-color: lightyellow;\n";
  sReturn += "    padding: 5px;\n";
  sReturn += "    width: 80%\n";
  sReturn += "    }\n";
  sReturn += "  div.all {\n";
  sReturn += "    background-color: lightgrey;\n";
  sReturn += "    border-radius: 5px;\n";
  sReturn += "    width: 350px;\n";
  sReturn += "    padding: 5px;\n";
  sReturn += "    clear: both;\n";
  sReturn += "    }\n";
  sReturn += "  p.small {";
  sReturn += "    font-size:75%;\n";
  sReturn += "    text-align: left;\n";
  sReturn += "    }\n";
  sReturn += "  input.field {width:200px;}\n";

  return sReturn;
}

String htmlFunction() {
  String sReturn ="\n";
  sReturn += "<script type=\"text/javascript\">\n";
  sReturn += "function toggle(control){\n";
  sReturn += "    var elem = document.getElementById(control);\n";
  sReturn += "\n";
  sReturn += "    if(elem.style.display == \"none\"){\n";
  sReturn += "        elem.style.display = \"inline-block\";\n";
  sReturn += "    }else{\n";
  sReturn += "        elem.style.display = \"none\";\n";
  sReturn += "    }\n";
  sReturn += "}\n";

  sReturn += "function showhide(control, val){\n";
  sReturn += "  var elem = document.getElementById(control);\n";
  sReturn += "  if(val){\n";
  sReturn += "    elem.style.display = \"inline-block\"\n";
  sReturn += "  }else{\n";
  sReturn += "    elem.style.display = \"none\";\n";
  sReturn += "  }\n";
  sReturn += "}\n";  
  sReturn += "</script>\n";

  sReturn += "<script type=\"text/javascript\">\n";
  sReturn += "var xmlHttpObject = false;\n";
  sReturn += "// Überprüfen ob XMLHttpRequest-Klasse vorhanden und erzeugen von Objekte für IE7, Firefox, etc.\n";
  sReturn += "if (typeof XMLHttpRequest != 'undefined') \n";
  sReturn += "{\n";
  sReturn += "    xmlHttpObject = new XMLHttpRequest();\n";
  sReturn += "}\n";
  sReturn += "";
  sReturn += "// Wenn im oberen Block noch kein Objekt erzeugt, dann versuche XMLHTTP-Objekt zu erzeugen\n";
  sReturn += "// Notwendig für IE6 oder IE5\n";
  sReturn += "if (!xmlHttpObject) \n";
  sReturn += "{\n";
  sReturn += "    try \n";
  sReturn += "    {\n";
  sReturn += "        xmlHttpObject = new ActiveXObject(\"Msxml2.XMLHTTP\");\n";
  sReturn += "    }\n";
  sReturn += "    catch(e) \n";
  sReturn += "    {\n";
  sReturn += "        try \n";
  sReturn += "        {\n";
  sReturn += "            xmlHttpObject = new ActiveXObject(\"Microsoft.XMLHTTP\");\n";
  sReturn += "        }\n";
  sReturn += "        catch(e) \n";
  sReturn += "        {\n";
  sReturn += "            xmlHttpObject = null;\n";
  sReturn += "        }\n";
  sReturn += "    }\n";
  sReturn += "}</script>\n";

  sReturn += "<script type=\"text/javascript\">\n";
  sReturn += "function sndReq()\n";
  sReturn += "{\n";   
  sReturn += "xmlHttpObject.open('get', '/anwert', true);\n";
  sReturn += "xmlHttpObject.send();";
  sReturn += "  xmlHttpObject.onreadystatechange = function () {\n";
  sReturn += "    if(xmlHttpObject.readyState == 4)\n";
  sReturn += "    {\n";
  sReturn += "      document.getElementById('divAnWert').innerHTML = xmlHttpObject.responseText\n";
  sReturn += "    }\n";
  sReturn += "  }\n";
  sReturn += "  http.send(null);\n";
  sReturn += "}\n";
  sReturn += "</script>  \n";

  sReturn += "<script type=\"text/javascript\">\n";
  sReturn += "function sndReqFull()\n";
  sReturn += "{\n";   
  sReturn += "xmlHttpObject.open('get', '/fullstatus', true);\n";
  sReturn += "xmlHttpObject.send();\n";
  sReturn += "  xmlHttpObject.onreadystatechange = function () {\n";
  sReturn += "    if(xmlHttpObject.readyState == 4)\n";
  sReturn += "    {\n";
  sReturn += "      document.getElementById('divFull').innerHTML = xmlHttpObject.responseText\n";
  sReturn += "    }\n";
  sReturn += "  }\n";
//  sReturn += "  http.send(null);\n";
  sReturn += "}\n";
  sReturn += "</script>  \n";
  return sReturn;
}
void htmlresponse(String sTitel, String sInhalt) {
    String sResponse;
    Serial.println("htmlresponse begin");
    sResponse  = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\n";
    sResponse += "<html lang=\"de\"><head>\n";
    sResponse += "<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">\n";
    sResponse += "<Style type=\"text/css\">\n";
//  sResponse += cssSwitch();
    sResponse += cssForm();
    sResponse += "</style>\n";
    sResponse += htmlFunction();
    sResponse += "<title>\n";
    sResponse += sTitel;
    sResponse += "</title></head><body>\n";
    sResponse += sInhalt;
    sResponse += "</body></html>\n";
    Serial.println("htmlresponse write to client");
    server.send(200, "text/html", sResponse);
    Serial.println("htmlresponse end");
}

String htmlDisplay(int iState) {
  String sRet = "";
  if (iState == 1) {
     sRet = "style=\"display: inline-block;\"";
  }
  else {
     sRet = "style=\"display: none;\"";
  }
  return sRet;
}

void wwwSetup() {
  Serial.println("wwwrSetup begin");

  String sResp="";
  sResp += "<h1>Setup Page</h1>\n";
  sResp += "<div class=\"all\">\n";
  sResp += "<form action=\"/save\" id=\"Servoduino\"  method=\"post\" autocomplete=\"off\">\n";
  sResp += "  <p class=\"sys2inf\"><b>Servo Parameter</b></p>\n";
  sResp += "  <table>\n";
  sResp += "    <TR>\n";
  sResp += "      <TD class=\"first\">\n";
  sResp += "        An\n";
  sResp += "      </TD>\n";
  sResp += "      <TD class=\"second\">\n";
  sResp += "        <input class=\"field\" name=\"an\" type=\"number\" min=\"0\" max=\"180\" step=\"5\" value=\""+ String(myConfig.iAn) + "\">\n";
  sResp += "      </TD>\n";
  sResp += "    </TR>\n";
  sResp += "    <TR>\n";
  sResp += "      <TD class=\"first\">\n";
  sResp += "        Aus\n";
  sResp += "      </TD>\n";
  sResp += "      <TD class=\"second\">\n";
  sResp += "        <input class=\"field\" name=\"aus\" type=\"number\" min=\"0\" max=\"180\" step=\"5\" value=\""+ String(myConfig.iAus) + "\">\n";
  sResp += "      </TD>\n";
  sResp += "    </TR>\n";
  sResp += "    <TR>\n";
  sResp += "      <TD class=\"first\">\n";
  sResp += "        Korrektur\n";
  sResp += "      </TD>\n";
  sResp += "      <TD class=\"second\">\n";
  sResp += "        <input class=\"field\" name=\"korrektur\" type=\"number\" min=\"-90\" max=\"90\" step=\"5\" value=\""+ String(myConfig.iKorrektur) + "\">\n";
  sResp += "      </TD>\n";
  sResp += "    </TR>\n";
  sResp += "  </Table>\n";
  sResp += "\n";

// -----------------------
  sResp += "  <p class=\"sys2inf\"><b>Hardware Parameter</b></p>\n";
  sResp += "  <table class=\"fixed\">\n";
  // -- Analog Schwelle ---
  sResp += "    <TR>\n";
  sResp += "      <TD class=\"first\">\n";
  sResp += "        Schwelle Analoger Eingang\n";
  sResp += "      </TD>\n";
  sResp += "      <TD class=\"second\">\n";
  sResp += "        <input class=\"field\" name=\"AnalogSchwelle\" type=\"number\" min=\"0\" max=\"9999\" step=\"1\" value=\""+ String(myConfig.iAnalogSchwelle) + "\">\n";
  sResp += "      </TD>\n";
  sResp += "    </TR>\n";

  // -- MagnetSchalter 
  sResp += "    <TR>\n";
  sResp += "      <TD class=\"first\">\n";
  sResp += "          <input type=\"checkbox\" name=\"MagnetSchalter\"\n";
                        if(myConfig.bMagnetsensor==1) { sResp += "checked=\"checked\"\n";}
                      sResp += ">\n";
  sResp += "    </TD>\n";
  sResp += "      <TD class=\"second\">\n";
  sResp += "        Magnet Sensor vorhanden\n";
  sResp += "      </TD>\n";
  sResp += "  </TR>\n";
  // -- Schalter1   
  sResp += "  <TR>\n";
  sResp += "      <TD class=\"first\">\n";
  sResp += "          <!-- sResp += htmlSwitch(\"SchalterWieTaster1\",myConfig.bSwitchAsPushButton[0]); -->\n";
  sResp += "          <input type=\"checkbox\" name=\"SchalterWieTaster1\" onchange = \"showhide('swt1', this.checked)\" ";
                        if(myConfig.bSwitchAsPushButton[0]==1) { sResp += "checked=\"checked\"\n";}
                      sResp += ">\n";
  sResp += "    </TD>\n";
  sResp += "    <TD class=\"second\">\n";
  sResp += "        Schalter 1 wie Taster\n";
  sResp += "    </TD>\n";
  sResp += "  </TR>\n";
  sResp += "  <TR id=\"swt1\" class=\"hidden\" " + htmlDisplay(myConfig.bSwitchAsPushButton[0]) + ">\n";
  sResp += "    <TD class=\"first\">Schaltzeit: </TD>\n";
  sResp += "    <TD class=\"second\"><input class=\"field\" name=\"SchalterTime1\" type=\"number\" min=\"5\" max=\"3600\" step=\"5\" value=\""+ String(myConfig.iSwitchTimer[0]) +  "\"/></TD>\n";
  sResp += "  </TR>\n";
  // -- Schalter2 
  sResp += "  <TR>\n";
  sResp += "      <TD class=\"first\">\n";
  sResp += "          <!-- sResp += htmlSwitch(\"SchalterWieTaster2\",myConfig.bSwitchAsPushButton[1]); -->\n";
  sResp += "          <input type=\"checkbox\" name=\"SchalterWieTaster2\" onchange = \"showhide('swt2', this.checked)\" ";
                        if(myConfig.bSwitchAsPushButton[1]==1) { sResp += "checked=\"checked\"\n";}
                      sResp += ">\n";
  sResp += "    </TD>\n";
  sResp += "      <TD class=\"second\">\n";
  sResp += "        Schalter 2 wie Taster\n";
  sResp += "      </TD>\n";
  sResp += "        <!-- if (myConfig.bSwitchAsPushButton) { -->\n";
  sResp += "  </TR>\n";
  sResp += "  <TR id=\"swt2\" class=\"hidden\" " + htmlDisplay(myConfig.bSwitchAsPushButton[1]) + ">\n";
  sResp += "    <TD class=\"first\">Schaltzeit: </TD>\n";
  sResp += "    <TD class=\"second\"><input class=\"field\" name=\"SchalterTime2\" type=\"number\" min=\"5\" max=\"3600\" step=\"5\" value=\""+ String(myConfig.iSwitchTimer[1]) +  "\"/></TD>\n";
  sResp += "  </TR>\n";
  // -- Taster 1
  sResp += "  <TR>\n";
  sResp += "    <TD class=\"first\">\n";
  sResp += "       <!--htmlSwitch(\"TasterMitTimer\",myConfig.bPushButtonTimer[0]);-->\n";
  sResp += "       <input type=\"checkbox\" name=\"TasterMitTimer1\" onchange = \"showhide('tmt1', this.checked)\" ";
                     if(myConfig.bPushButtonTimer[0]==1) { sResp += "checked=\"checked\"\n";}
                        sResp += ">\n";
  sResp += "    </TD>\n";
  sResp += "    <TD class=\"second\">\n";
  sResp += "      Taster mit eigenem Timer\n";
  sResp += "    </TD>\n";
  sResp += "      <!--if (myConfig.bPushButtonTimer[0]) {-->\n";
  sResp += "  </TR>\n";
  sResp += "  <TR id=\"tmt1\" class=\"hidden\" " + htmlDisplay(myConfig.bPushButtonTimer[0]) + ">\n";
  sResp += "    <TD class=\"first\">Schaltzeit: </TD>\n";
  sResp += "    <TD class=\"second\"><input class=\"field\" name=\"TasterTime1\" type=\"number\" min=\"5\" max=\"3600\" step=\"5\" value=\""+ String(myConfig.iPushButtonTimer[0]) + "\"/></TD>\n";
  sResp += "        <!--}-->\n";
  sResp += "    </TR>\n";
  // -- Taster 2
  sResp += "  <TR>\n";
  sResp += "    <TD class=\"first\">\n";
  sResp += "       <!--htmlSwitch(\"TasterMitTimer\",myConfig.bPushButtonTimer[0]);-->\n";
  sResp += "       <input type=\"checkbox\" name=\"TasterMitTimer2\" onchange = \"showhide('tmt2', this.checked)\" ";
                     if(myConfig.bPushButtonTimer[1]==1) { sResp += "checked=\"checked\"\n";}
                        sResp += ">\n";
  sResp += "    </TD>\n";
  sResp += "    <TD class=\"second\">\n";
  sResp += "      Taster mit eigenem Timer\n";
  sResp += "    </TD>\n";
  sResp += "      <!--if (myConfig.bPushButtonTimer[1]) {-->\n";
  sResp += "  </TR>\n";
  sResp += "  <TR id=\"tmt2\" class=\"hidden\" " + htmlDisplay(myConfig.bPushButtonTimer[1]) + ">\n";
  sResp += "    <TD class=\"first\">Schaltzeit: </TD>\n";
  sResp += "    <TD class=\"second\"><input class=\"field\" name=\"TasterTime2\" type=\"number\" min=\"5\" max=\"3600\" step=\"5\" value=\""+ String(myConfig.iPushButtonTimer[1]) + "\"/></TD>\n";
  sResp += "        <!--}-->\n";
  sResp += "    </TR>\n";

  sResp += "  </table>\n";
  sResp += "\n";
// ---------------------------
  sResp += " <div id=\"toinform\">\n";
  sResp += "  <p class=\"sys2inf\"><b>Systeme die aktualisiert werden sollen</b></p>\n";
  sResp += "  <!-- Homematic -->\n";
  sResp += "    <input type=\"checkbox\" name=\"HM\" id=\"HM\" onchange = \"showhide('Homematic', this.checked)\" ";
                       if(myConfig.bHomematic==1) { sResp += "checked=\"checked\"\n";}
                        sResp += ">\n";

  sResp += "    <label for=\"HM\">Homematic</label>\n";
  sResp += "    <div id=\"Homematic\" " + htmlDisplay(myConfig.bHomematic) + ">\n";
  sResp += "    <table  class=\"HM\">\n";
  sResp += "      <TR>\n";
  sResp += "        <TD class=\"first\">\n";
  sResp += "          IP Adresse:\n";
  sResp += "        </TD>\n";
  sResp += "        <TD class=\"second\">\n";
  sResp += "          <input class=\"field\" name=\"HMIP\" type=\"TEXT\" value=\""+ String(myConfig.sHomematicIP) + "\"/>\n";
  sResp += "        </TD>\n";
  sResp += "      </TR>\n";
  sResp += "          <TR>\n";
  sResp += "        <TD class=\"first\">\n";
  sResp += "          CUxD Ger&auml;t (Tab):\n";
  sResp += "        </TD>\n";
  sResp += "        <TD class=\"second\">\n";
  sResp += "          <input class=\"field\" name=\"HMGERAET\" type=\"TEXT\" value=\""+ String(myConfig.sHomematicGeraet) + "\"/>\n";
  sResp += "        </TD>\n";
  sResp += "      </TR>\n";
  sResp += "      <TR>\n";
  sResp += "        <TD class=\"first\">\n";
  sResp += "          CUxD Ger&auml;t (Relay):\n";
  sResp += "        </TD>\n";
  sResp += "        <TD class=\"second\">\n";
  sResp += "          <input class=\"field\" name=\"HMGERAETRELAY\" type=\"TEXT\" value=\""+ String(myConfig.sHomematicGeraetRelay) + "\"/>\n";
  sResp += "        </TD>\n";
  sResp += "      </TR>\n";
  sResp += "    </table>\n";
  sResp += "    </div>\n";
  sResp += "    <br>\n";
  sResp += "  <!-- Loxone -->\n";
  sResp += "    <input type=\"checkbox\" name=\"Lox\" id=\"Lox\" onchange = \"showhide('Loxone', this.checked)\" ";
                       if(myConfig.bLoxone==1) { sResp += "checked=\"checked\"\n";}
                        sResp += ">\n";

  sResp += "    <label for=\"Lox\">Loxone</label>\n";
  sResp += "    <div id=\"Loxone\" " + htmlDisplay(myConfig.bLoxone) + ">\n";
  sResp += "    <table  class=\"Loxone\">\n";
  sResp += "      <TR>\n";
  sResp += "        <TD class=\"first\">\n";
  sResp += "          IP Adresse:\n";
  sResp += "        </TD>\n";
  sResp += "        <TD class=\"second\">\n";
  sResp += "          <input class=\"field\" name=\"LoxIP\" type=\"TEXT\" value=\""+ String(myConfig.sLoxoneIP) + "\"/>\n";
  sResp += "        </TD>\n";
  sResp += "      </TR>\n";
  sResp += "          <TR>\n";
  sResp += "        <TD class=\"first\">\n";
  sResp += "          UDP Port:\n";
  sResp += "        </TD>\n";
  sResp += "        <TD class=\"second\">\n";
  sResp += "          <input class=\"field\" name=\"LoxPort\" type=\"number\" min=\"0\" max=\"99999\" step=\"1\" value=\""+ String(myConfig.sLoxonePort) + "\"/>\n";
  sResp += "        </TD>\n";
  sResp += "      </TR>\n";
  sResp += "    </table>\n";
  sResp += "  </div>\n";
  sResp += "  </div>\n";
  sResp += "\n";
  sResp += "  <div style =\"padding-top: 30px;position: relative;\">\n";
  sResp += "  <button type=\"submit\" name =\"action\" style =\"position: absolute;right: 10px;top: 5px;\"> Absenden </button>\n";
  sResp += "  </div>\n";
  sResp += "</form>\n";
  sResp += "</div>\n";
  sResp += "<br><a href='/'><button type='button'>Startseite</BUTTON></a> \n";
  htmlresponse("Setup", sResp);
  Serial.println("wwwrSetup end");
}

void wwwSave() {
  String sAn = server.arg("an");
  String sAus = server.arg("aus");
  String sKorrektur = server.arg("korrektur");
  String sSchalterWieTaster1 = server.arg("SchalterWieTaster1");
  String sSchalterTime1 = server.arg("SchalterTime1");
  String sTasterMitTimer1 = server.arg("TasterMitTimer1");
  String sTasterTime1 = server.arg("TasterTime1");
  String sSchalterWieTaster2 = server.arg("SchalterWieTaster2");
  String sSchalterTime2 = server.arg("SchalterTime2");
  String sTasterMitTimer2 = server.arg("TasterMitTimer2");
  String sTasterTime2 = server.arg("TasterTime2");
  String sMagnetSchalter = server.arg("MagnetSchalter");
  String sAnalogSchwelle = server.arg("AnalogSchwelle");
    
  String sHM = server.arg("HM");
//  -----
  String sHMIP = server.arg("HMIP");
  String sHMGERAET = server.arg("HMGERAET");
  String sHMGERAETRELAY = server.arg("HMGERAETRELAY");
//  -----  
  String sLox = server.arg("Lox");
  String sLoxIP = server.arg("LoxIP");
  String sLoxPort = server.arg("LoxPort");
// IP ---
  String bIP = server.arg("FesteIP");
  String sIP = server.arg("IP");
  String sSubNet = server.arg("Sub");
  String sGateway = server.arg("GW"); 


  Serial.println("Save Parameter Temporarly ------");
  Serial.println("an:                 " + sAn );
  Serial.println("aus:                " + sAus );
  Serial.println("korrektur:          " + sKorrektur );
  Serial.println("-------------------------------");
  Serial.println("SchalterWieTaster1: " + sSchalterWieTaster1 );
  Serial.println("SchalterTime1:      " + sSchalterTime1 );
  Serial.println("TasterMitTimer1:    " + sTasterMitTimer1 );
  Serial.println("TasterTime1:        " + sTasterTime1 );
  Serial.println("---");
  Serial.println("SchalterWieTaster2: " + sSchalterWieTaster2 );
  Serial.println("SchalterTime2:      " + sSchalterTime2 );
  Serial.println("TasterMitTimer2:    " + sTasterMitTimer2 );
  Serial.println("TasterTime2:        " + sTasterTime2 );
  Serial.println("MagnetSchater:      " + sMagnetSchalter );
  Serial.println("AnalogSchwelle:     " + sAnalogSchwelle );

  Serial.println("-------------------------------");
  Serial.println("Homematic           " + sHM );
  Serial.println("Homematic IP        " + sHMIP );
  Serial.println("Homematic Geraet    " + sHMGERAET );
  Serial.println("Homematic Geraet Re " + sHMGERAETRELAY );
  Serial.println("Loxone              " + sLox );
  Serial.println("Loxone IP           " + sLoxIP );
  Serial.println("Loxone Geraet       " + sLoxPort );
  Serial.println("-------------------------------");
  Serial.println("Feste IP:           " + bIP);
  Serial.println("IP                  " + sIP);
  Serial.println("Subnet              " + sSubNet);
  Serial.println("Gateway             " + sGateway); 
  Serial.println("-------------------------------");
  
  if (sAn!="") {myConfig.iAn = sAn.toInt();}
  if (sAus!="") {myConfig.iAus = sAus.toInt();}
  if (sKorrektur!="") {myConfig.iKorrektur = sKorrektur.toInt();}

// -------------------- Schalter wie Taster   --------------
// Schalter 1
  if (sSchalterWieTaster1 =="on") {
      myConfig.bSwitchAsPushButton[0] = 1;
      if (sSchalterTime1!="") {
         myConfig.iSwitchTimer[0] = sSchalterTime1.toInt();
      }
  }
  else {
    myConfig.bSwitchAsPushButton[0] = 0;
  }

//Schalter 2
  if (sSchalterWieTaster2 =="on") {
      myConfig.bSwitchAsPushButton[1] = 1;
      if (sSchalterTime2!="") {
         myConfig.iSwitchTimer[1] = sSchalterTime2.toInt();
      }
  }
  else {
    myConfig.bSwitchAsPushButton[1] = 0;
  }

// ---------------------- Taster mit Timer -------------------
//Taster 1  
  if (sTasterMitTimer1 =="on") {
       myConfig.bPushButtonTimer[0] = 1;
      if (sTasterTime1!="") {
         myConfig.iPushButtonTimer[0] = sTasterTime1.toInt();
      }
  }
  else {
     myConfig.bPushButtonTimer[0] = 0;
  }
  
// Taster 2
  if (sTasterMitTimer2 =="on") {
       myConfig.bPushButtonTimer[1] = 1;
      if (sTasterTime2!="") {
         myConfig.iPushButtonTimer[1] = sTasterTime2.toInt();
      }
  }
  else {
     myConfig.bPushButtonTimer[1] = 0;
  }

  if (sMagnetSchalter =="on") {
          myConfig.bMagnetsensor = 1;
      } else {
          myConfig.bMagnetsensor = 0;

      }
  if (sAnalogSchwelle !="") { myConfig.iAnalogSchwelle = sAnalogSchwelle.toInt();}
  
// ----- Homematic
  if (sHM =="on") { 
        myConfig.bHomematic = 1;
     } else {
        myConfig.bHomematic = 0;        
     }
   
  if (sHMIP !="") { sHMIP.toCharArray(myConfig.sHomematicIP,16);}
  if (sHMGERAET !="") { sHMGERAET.toCharArray(myConfig.sHomematicGeraet ,20);}
  if (sHMGERAETRELAY !="") { sHMGERAETRELAY.toCharArray(myConfig.sHomematicGeraetRelay ,20);}

// -----Loxone
  if (sLox =="on") { 
        myConfig.bLoxone = 1;
     } else {
        myConfig.bLoxone = 0;
     }

  if (sLoxIP !="") { sLoxIP.toCharArray(myConfig.sLoxoneIP,16) ;}
  if (sLoxPort  !="") { sLoxPort.toCharArray( myConfig.sLoxonePort ,20);}

  if (bIP =="on") { 
        myConfig.bIP = 1;
     } else {
        myConfig.bIP = 0;        
     }
   
  if (sIP != "") { sIP.toCharArray(myConfig.sIP,16);}
  if (sSubNet != "") { sSubNet.toCharArray(myConfig.sSubNet ,16);}
  if (sGateway != "") { sGateway.toCharArray(myConfig.sGateway,16);} 

  saveConfig();
  htmlreloadpresite("/setup","Speicher Parameter");
 
}

void wwwRoot() {
  Serial.println("wwwrRoot begin");

  String sResp;
  sResp = "<script type=\"text/javascript\" language=\"JavaScript\">";
//  sResp +="   setInterval(sndReq, 500);";
  sResp +="   setInterval(sndReqFull, 1000);";
  sResp +="</script>\n";
  sResp += "<h1>Servo Steuerung</h1>\n";
  sResp += "<p class=\"small\">Version: " + sVersion + "<br> vom " + sVersionDatum + " </p>\n";
//  sResp += "Aktueller Status: <div id=\"divGrad\" name=\"divGrad\" style=\"display: inline-block;\">" + String(getStatus())+"</div><br>\n";
//  sResp += "Analog Wert:      <div id=\"divAnWert\" name=\"divAnWert\" style=\"display: inline-block;\">0</div>\n";
  sResp += "<div id=\"divFull\" name=\"divFull\">Status wird geladen....</div>\n";
  sResp += "<hr>\n";
  sResp += "<table>";
  sResp += "<tr><td>Tablet : </td><td><a href=\"/an?reload=1\"><button type='button'>An</button></a></td>\n";
  sResp += "<td><a href=\"/aus?reload=1\"><button type='button'>Aus</button></a></td></tr>\n";
  sResp += "<tr><td>Relay : </td><td><a href=\"/ran?reload=1\"><button type='button'>An</button></a></td>\n";
  sResp += "<td><a href=\"/raus?reload=1\"><button type='button'>Aus</button></a></td></tr>\n";
  sResp += "</table>";
  sResp += "<hr>\n";
  sResp += "<a href=\"/setup\"><button type='button'>Konfiguration</button></a><br>\n";
  sResp += "<a href=\"/firmware\"><button type='button'>Update Firmware</button></a><br>\n";
  sResp += "\n";
  sResp += "\n";
  sResp += "\n";

  htmlresponse("Servo", sResp);
  Serial.println("wwwrRoot end");
}

void printConfig() {
 Serial.println("Valid1 (1234): " + String(myConfig.bValidConfig1));
 Serial.println("Valid2 (4321): " + String(myConfig.bValidConfig2));  
 Serial.println("Config Ver.  : " + String(myConfig.iConfigVersion));
 Serial.println("An           : " + String(myConfig.iAn));
 Serial.println("Aus          : " + String(myConfig.iAus));
 Serial.println("Korrektur    : " + String(myConfig.iKorrektur));
 Serial.println("SwitchAsPush : " + String(myConfig.bSwitchAsPushButton[0]));
 Serial.println("SwitchTimer  : " + String(myConfig.iSwitchTimer[0]));
 Serial.println("PB with Timer: " + String(myConfig.bPushButtonTimer[0]));
 Serial.println("PB Timer     : " + String(myConfig.iPushButtonTimer[0]));  
 Serial.println("SwitchAsPush : " + String(myConfig.bSwitchAsPushButton[1]));
 Serial.println("SwitchTimer  : " + String(myConfig.iSwitchTimer[1]));
 Serial.println("PB with Timer: " + String(myConfig.bPushButtonTimer[1]));
 Serial.println("PB Timer     : " + String(myConfig.iPushButtonTimer[1]));  
 Serial.println("Homematic    : " + String(myConfig.bHomematic)); 
 Serial.println("Homatic IP   : " + String(myConfig.sHomematicIP)); 
 Serial.println("Homematic Ger: " + String(myConfig.sHomematicGeraet)); 
 Serial.println("Loxone       : " + String(myConfig.bLoxone)); 
 Serial.println("Loxone IP    : " + String(myConfig.sLoxoneIP)); 
 Serial.println("Loxone Geraet: " + String(myConfig.sLoxonePort)); 

}


void getConfig() {
  oConfig tmpConfig;
  int eeAddress = 0;
  EEPROM.begin(256);
  EEPROM.get(eeAddress, tmpConfig);
  EEPROM.end();
  Serial.println("------------------");
  Serial.println("valid1: " + String(tmpConfig.bValidConfig1));
  Serial.println("valid2: " + String(tmpConfig.bValidConfig2));
  Serial.println("------------------");
  if ((String(tmpConfig.bValidConfig1)=="1234") and (String(tmpConfig.bValidConfig2)=="4321")) {
    Serial.println("Gueltige Konfiguration gefunden.... Lade Daten");
    myConfig.bValidConfig1=tmpConfig.bValidConfig1;
    myConfig.bValidConfig2=tmpConfig.bValidConfig2;
    //myConfig.iConfigVersion=tmpConfig.iConfigVersion;
    myConfig.iAn=tmpConfig.iAn;
    myConfig.iAus=tmpConfig.iAus;
    myConfig.iKorrektur=tmpConfig.iKorrektur;
    if (tmpConfig.iConfigVersion>=1) {
        myConfig.bSwitchAsPushButton[0]=tmpConfig.bSwitchAsPushButton[0];
        myConfig.iSwitchTimer[0]=tmpConfig.iSwitchTimer[0];
        myConfig.bPushButtonTimer[0]=tmpConfig.bPushButtonTimer[0];
        myConfig.iPushButtonTimer[0]=tmpConfig.iPushButtonTimer[0];
        myConfig.bSwitchAsPushButton[1]=tmpConfig.bSwitchAsPushButton[1];
        myConfig.iSwitchTimer[1]=tmpConfig.iSwitchTimer[1];
        myConfig.bPushButtonTimer[1]=tmpConfig.bPushButtonTimer[1];
        myConfig.iPushButtonTimer[1]=tmpConfig.iPushButtonTimer[1];
    }
    if (tmpConfig.iConfigVersion>=2) {
        myConfig.iAnalogSchwelle = tmpConfig.iAnalogSchwelle;
        myConfig.bMagnetsensor = tmpConfig.bMagnetsensor;
        myConfig.bHomematic=tmpConfig.bHomematic;
        strncpy(myConfig.sHomematicIP,tmpConfig.sHomematicIP,16);
        strncpy(myConfig.sHomematicGeraet,tmpConfig.sHomematicGeraet,20);
        strncpy(myConfig.sHomematicGeraetRelay,tmpConfig.sHomematicGeraetRelay,20);        
        myConfig.bLoxone=tmpConfig.bLoxone;
        strncpy(myConfig.sLoxoneIP,tmpConfig.sLoxoneIP,16);
        strncpy(myConfig.sLoxonePort,tmpConfig.sLoxonePort,20);
        myConfig.bIP = tmpConfig.bIP;        
        strncpy(myConfig.sIP,tmpConfig.sIP,16);
        strncpy(myConfig.sSubNet,tmpConfig.sSubNet,16);
        strncpy( myConfig.sGateway,tmpConfig.sGateway,16); 
      }
    }
  else {
     Serial.println("Kein gueltige Konfiguration gefunden.... arbeite mit Default Werten");   
  }
}

void setPosAn() {
        int iStatus=myConfig.iAn;
        myservo.write(getKorrigiert(iStatus));
        Serial.printf("Switch AN Taster oder Schalter neuer Status: %i", getOnOffStatus());
        Serial.println("");
        sendTabState();  
}
void setPosAus() {
        int iStatus=myConfig.iAus;
        myservo.write(getKorrigiert(iStatus));
        Serial.printf("Switch Aus Taster oder Schalter neuer Status: %i", getOnOffStatus());
        Serial.println("");
        sendTabState();  
}

void toggle() {
    int iState = getOnOffStatus();  
    if (iState == 1) {
      setPosAus();
      } 
    if (iState == 0) {
      setPosAn();
    } 
}

void handlePushButton(int iPB) {
    unsigned long currentMillis = millis();
    // Entprellen 
    if ((currentMillis - tPushButtonLast[iPB]) > 500) {
        //Serial.printf("Handle Pushbutton - Current: %i, Last: %i, Diff: %i", currentMillis, tPushButtonLast[iPB], (currentMillis - tPushButtonLast[iPB]));
        //Serial.println("");
        int iPushButtonState = digitalRead(iPushButtonPin[iPB]);
        if ((iPushButtonState == 1) and iPushButtonState != iPushButtonLastState[iPB]) {
            toggle();
            iPushButtonLastState[iPB] = iPushButtonState;
            tPushButtonLast[iPB]=currentMillis;
            sendUDP("Taster[" + String(iPB) + "]1");
            }
        else if ((iPushButtonState == 0) and iPushButtonState != iPushButtonLastState[iPB]) {
            iPushButtonLastState[iPB] = iPushButtonState;
            sendUDP("Taster[" + String(iPB) + "]0");
            }
    }
}
void handleSwitch(int iSW){
    unsigned long currentMillis = millis();
    int iSwitchState = digitalRead(iSwitchPin[iSW]);  
    if (iSwitchState != iSwitchLastState[iSW]) {
        if (iSwitchState == 1) {
                setPosAn();
                sendUDP("Schalter[" + String(iSW) + "]1");

            } 
         if (iSwitchState == 0) {
                setPosAus();
                sendUDP("Schalter[" + String(iSW) + "]0");
            }
            iSwitchLastState[iSW] = iSwitchState; 
            tSwitchLast[iSW]=currentMillis;
      }
}
bool charcomp(char* cbuf, String scmp) {
      bool bErg = true;
      for (int i = 0; i<sizeof(cbuf) ;i++) {
        if (int(cbuf[i])!=10) {
            //Serial.println(String (i)+ ": "+scmp[i] + " = " + int(cbuf[i]));
            bErg = bErg and (scmp[i]==cbuf[i]);  
        }
      }
      Serial.println (String (cbuf) + " = " +scmp + " : " +bErg);
      
      return bErg;        
}

void handleudp(){
    int packetSize = Udp.parsePacket();
    if (packetSize) {
        Serial.print("Received packet of size ");
        Serial.println(packetSize);
        Serial.print("From ");
        IPAddress remoteIp = Udp.remoteIP();
        Serial.print(remoteIp);
        Serial.print(", port ");
        Serial.println(Udp.remotePort());
    
        // read the packet into packetBufffer
        int len = Udp.read(packetBuffer, 255);
        if (len > 0) {
          packetBuffer[len] = 0;
        }
 
    Serial.println("Contents:");
    Serial.println(packetBuffer);
    char ReplyBufferState[]="5";
   //Funktionalität.....
   
    if (charcomp(packetBuffer,"an")) {
         Serial.println("UDP An");
         setPosAn();
         ReplyBufferState[0]='1';
    }
    if (charcomp(packetBuffer,"aus")) {
         Serial.println("UDP Aus");
         setPosAus();
         ReplyBufferState[0]='0';
    }
    if (charcomp(packetBuffer,"status")) {
         int iStatus = getOnOffStatus();
         ReplyBufferState[0]=String(iStatus).charAt(1);
    }
    if (charcomp(packetBuffer,"ran")) {
         Serial.println("UDP Relay An");
         wwwrelayan();
         ReplyBufferState[0]='1';
    }
    if (charcomp(packetBuffer,"raus")) {
         Serial.println("UDP Relay Aus");
         wwwrelayaus();
         ReplyBufferState[0]='0';
    }
    if (charcomp(packetBuffer,"rstatus")) {
         Serial.println("UDP Relay Status");
         if (digitalRead(iRelayPin)==1){
              ReplyBufferState[0]='1';
         }
        if (digitalRead(iRelayPin)==0) {
              ReplyBufferState[0]='0';
         }
    }

      
    // send a reply, to the IP address and port that sent us the packet we received
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(ReplyBufferState);
    Udp.endPacket();
    }
}

String IpAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3])  ; 
}

void handleAnalog() {
  int iSensorValue = analogRead(A0);
  unsigned long currentMillis = millis();
  if  (currentMillis < iAnaloglastMillis + 1000) { return ;} // Statuswechsel innerhalb 1 Sekunden ignorieren
  //Serial.println(String(iSensorValue) + ":" + String(myConfig.iAnalogSchwelle)); 
  if (iSensorValue > myConfig.iAnalogSchwelle) {
      if (iAnalogLastState==0){
         setPosAn();
         sendUDP("Analog1");
         iAnalogLastState = 1;
      }
  } 
  else {
      if (iAnalogLastState==1){
          setPosAus();
          sendUDP("Analog0");
          iAnalogLastState = 0;
      }
  }
  iAnaloglastMillis = currentMillis ; 
}

void resetWiFi() {
      wifiManager.resetSettings();
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.reset();
      delay(5000);
}

void resetConfig() {
   EEPROM.begin(0);
   for (int i = 0 ; i < 512 ; i++) {
     EEPROM.write(i, 0);
     EEPROM.end();
  }
}

void resetAll() {
  resetConfig();
  resetWiFi();
}

void restart() {
    ESP.restart();  // normal reboot 
    //ESP.reset();  // hard reset
}

void setup() {
  // put your setup code here, to run once:
  wifiManager.autoConnect("Servoduino","Servoduino");
  // put your setup code here, to run once:
  Serial.begin(115200); 
  Serial.println("--------------- Setup -------------");
  Serial.println("--------------- Setup get Wifi -------------");
  IPAddress ip = WiFi.localIP();
  String sSSID = WiFi.SSID();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.print("SSID: ");
  Serial.println(sSSID);
  Serial.println("--------------- Setup OTA -------------");
  httpUpdater.setup(&server, update_path, update_username, update_password);
  char cIP[16];
  String sIP = IpAddress2String(ip);
  sIP.toCharArray(cIP,16);
  Serial.printf("HTTPUpdateServer ready! Open http://%s%s in your browser and login with username '%s' and password '%s'\n", cIP , update_path, update_username, update_password);
  Serial.println("--------------- Setup Webserver --------");
  server.on("/", wwwRoot);
  server.on("/status", wwwmelde_status);
  server.on("/g", goPos);  
  server.on("/resetwifi", resetWiFi);
  server.on("/resetconfig", resetConfig);
  server.on("/werkseinstellungen", resetAll);
  server.on("/restart", restart);
  server.on("/setup", wwwSetup);
  server.on("/save", wwwSave);
  server.on("/an", wwwan);
  server.on("/aus", wwwaus);
  server.on("/ran", wwwrelayan);
  server.on("/raus", wwwrelayaus);
  server.on("/anwert", wwwanwert); //Analog Wert für Anzeige auf Startsite
  server.on("/fullstatus", wwwmelde_FullStatus);
  server.begin();
  Serial.println("--------------- Setup UDP --------");
  Udp.begin(localPort);
  Serial.println("--------------- Setup Parameter laden -------------");
  Serial.println("Default: ");
  printConfig();
  // Config aus eeprom lesen
  getConfig();
  Serial.println("eeprom: ");
  printConfig();
  int iStatus = myConfig.iAus;
  
  Serial.println("--------------- Setup Servo -------------");
  myservo.attach(iServoPin); 
  myservo.write(getKorrigiert(iStatus));
  Serial.println("--------------- Setup Pins -------------");
  for (int i=0; i<2 ; i++) {
      pinMode(iPushButtonPin[i], INPUT);
      pinMode(iSwitchPin[i], INPUT);
  }
  pinMode(iRelayPin, OUTPUT);
  Serial.println("--------------- Hausautomation aktualisieren --------");
  sendState();
  Serial.println("--------------- Setup Ende --------");
  }

void loop() {
  // put your main code here, to run repeatedly:
    server.handleClient(); // auf HTTP-Anfragen warten
    for (int i=0; i<2 ; i++) {
        handlePushButton(i);
        handleSwitch(i);
    }
    handleAnalog();
    handleudp();
//    handleSwitch(1);
    delay(100);
}
