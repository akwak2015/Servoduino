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
  int bExpLogic;  // Schaltlogic der eingänge (switch gegen +5V oder GND) (0 fuer gegen GND)
};
typedef struct oconfig oConfig;


// *******************************************************************
// ******* Globale Variablen *****************************************
// *******************************************************************

//Version
//$Revision: 21 $
String sVersionDatum = "2016-09-05";
String sVersion = "0.8.2";

oConfig myConfig = {1234, 4321, 3, 0, 180, 10, {0, 0}, {60, 60}, {0, 0}, {120, 120}, 0 , 1000, 0, "000.000.000.000", "Seriennummer","Seriennummer", 0, "000.000.000.000", "7000" , 0, "000.000.000.000", "000.000.000.000", "000.000.000.000", 0};

//Pin Definitionen
int iServoPin = 2; //WeMos mini D4
int iPushButtonPin[] = {14, 12}  ; //WeMos mini D5 und D6
int iSwitchPin[] = {13, 15}; //WeMos mini D7 und D8
int iRelayPin = 5; //Wemos mini D1 -> RelayBoard
int iMagnetsensorPin = 4; //Wemos mini D2 - MagnetSensor
// Analog Pin auf Wemos A0 => int sensorValue = analogRead(A0); 

// Aktueller Servo Status
int iServoPosAkt = -1;

// OTA
const char* update_path = "/firmware";
const char* update_username = "admin";
const char* update_password = "admin";

int iUeber = 0; //notwendiger "negatiever" Korrekturfaktor fue den Bereich ueber 180-iKorrektur.

int iPushButtonLastState[] = { 0, 0 };
unsigned long tPushButtonLast[2]; //letzter Tastendruck
int iSwitchLastState[] = { 0, 0 };
unsigned long tSwitchLast[2]; //letzte Schalterbetätigung

int iAnalogLastState = 0;
int iAnaloglastMillis = millis();

unsigned long iTimerMillis = millis();
bool bTimerAktiv = false;
unsigned long iRelayTimerMillis = millis();
bool bRelayTimerAktiv = false;

unsigned int localPort = 2390;      // local port to listen on
char packetBuffer[255]; //buffer to hold incoming packet
char  ReplyBuffer[] = "acknowledged";       // a string to send back

// *******************************************************************
// ******* Libraries         *****************************************
// *******************************************************************
WiFiManager wifiManager;
ESP8266WebServer server(80); // Webserver initialisieren auf Port 80
Servo myservo;
ESP8266HTTPUpdateServer httpUpdater;
WiFiUDP Udp;


// *******************************************************************
// ****** Prototypes  ************************************************
// *******************************************************************
//mycss
String cssForm();
String cssSwitch();
//myforms
void htmlresponse(String sTitel, String sInhalt);
String htmlFunction();
String htmlSwitch(String sName, int bStatus);
void wwwSetup();
void wwwRoot();
void wwwmelde_status();
void wwwmelde_FullStatus();
void htmlreloadpresite(String sTarget, String sText);
void htmlresponse0(String sText);
void wwwSwitchLogic();
//myhandler
void handlePushButton(int iPB);
void handleSwitch(int iSW);
void handleudp();
void handleAnalog();
void handleTimer();
void handleRelayTimer();
//Main
int getServo (bool bForcePinRead = false);
int getOnOffStatus(bool bForcePinRead = false);
int getStatus(bool bForcePinRead = false);
bool logichandler(int iPin = 0);

// *******************************************************************
// ******* Programm          *****************************************
// *******************************************************************

void setServo(int iPos) {
  myservo.attach(iServoPin); 
  delay(20);
  myservo.write(iPos);
  iServoPosAkt = iPos;
  delay(500);
  myservo.detach(); 
}

int getServo (bool bForcePinRead) {
  int iErg=0;
  if (bForcePinRead) {Serial.println("Force " + bForcePinRead);}
  if ((iServoPosAkt == -1) or (bForcePinRead)) {
    myservo.attach(iServoPin); 
    delay(500);
    iErg = myservo.read();
    delay(500);
    myservo.detach(); 
  }
  else {
    iErg=iServoPosAkt;
  }
  return iErg;
}

// Timer in Sekunden
void setTimerSec(int iTime) { 
  iTimerMillis = millis()+(1000 * iTime);
  bTimerAktiv = true;
  Serial.println("Timer aktiv: " + String(1000*iTime));
}

void setRelayTimerSec(int iTime) { 
  iRelayTimerMillis = millis()+(1000 * iTime);
  bRelayTimerAktiv = true;
  Serial.println("Relay Timer aktiv: " + String(1000*iTime));
}

void saveConfig() {
  Serial.println("__________ Save Config ________");
  int eeAddress = 0;
  EEPROM.begin(256);
  EEPROM.put(eeAddress, myConfig);
  delay(200);
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

int getStatus(bool bForcePinRead) {
  int iErg = getServo(bForcePinRead);
  iErg=iErg - myConfig.iKorrektur;
  if (iErg > 180) iErg=180;
  if (iErg < 0) iErg=0;
  //Serial.println("getKorrigiert - ServoPos: " + String( getServo()) + " Rueckgabe: "+ String(iErg));
  return iErg+iUeber;
}

int getOnOffStatus(bool bForcePinRead) {
    int iErg=-1;
    if (myConfig.bMagnetsensor==1) {
    delay(500);
    if (logichandler(iMagnetsensorPin) == HIGH) {
      iErg=1;
    } else {
      iErg=0;
    }
  } else {    
    int iStatus = getStatus(bForcePinRead);
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
    Serial.println(sizeof(sUDP)+2); //+2 um sicher alle Zeichen zu erwischen.
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
//    int iStatus = (iRelayPin);
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


void goPos() {
  if (server.arg("pos")!="") {
//  String sStatus = server.arg("pos");
//  iStatus=sStatus.toInt();    
  int iStatus = server.arg("pos").toInt();
  if (iStatus<0) { iStatus = 0; }
  if (iStatus>360) { iStatus = 360; }
   
  setServo(getKorrigiert(iStatus));
  htmlresponse0(String(getStatus()));
  }
}

void wwwrelayan() {
  String bReload = server.arg("reload");
  String sTimer = server.arg("timer");
  int iTimer=0;
  digitalWrite(iRelayPin, HIGH);
  if (sTimer!="") {
    iTimer = sTimer.toInt();
      if (iTimer > 0) {
          setRelayTimerSec(iTimer);   
      }
  }
  else {
     bRelayTimerAktiv = false;
  } 

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
  bRelayTimerAktiv = false;
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
  String sTimer = server.arg("timer");
  int iTimer=0;
  Serial.println("Reload? = " + bReload);
  Serial.println("Timer? = " + sTimer);
  int iStatus=myConfig.iAn;
  setServo(getKorrigiert(iStatus));
  if (sTimer!="") {
    iTimer = sTimer.toInt();
      if (iTimer > 0) {
          setTimerSec(iTimer);   
      } 
  }
  else {
      bTimerAktiv = false;
  }

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
  setServo(getKorrigiert(iStatus));
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
  bTimerAktiv=false;  
}

void  wwwanwert() {
    int sensorValue = analogRead(A0);
    htmlresponse0(String(sensorValue));  
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
  String bExpLogic = server.arg("Logic");

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
  Serial.println("Logic               " + String(bExpLogic));
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

  if (bExpLogic =="on") {
    myConfig.bExpLogic = 1;
    }
    else {
      myConfig.bExpLogic = 0;
    }
  saveConfig();
  delay(200);
  htmlreloadpresite("/setup","Speicher Parameter");
 
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
 Serial.println("Logic Inputs : " + String(myConfig.bExpLogic)); 

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
    if (tmpConfig.iConfigVersion>=3) {
        myConfig.bExpLogic=tmpConfig.bExpLogic;
      }
    }
  else {
     Serial.println("Kein gueltige Konfiguration gefunden.... arbeite mit Default Werten");   
  }
}

void setPosAn() {
        int iStatus=myConfig.iAn;
        setServo(getKorrigiert(iStatus));
        Serial.printf("Switch AN Taster oder Schalter neuer Status: %i", getOnOffStatus());
        Serial.println("");
        sendTabState();  
}
void setPosAus() {
        int iStatus=myConfig.iAus;
        setServo(getKorrigiert(iStatus));
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


String IpAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3])  ; 
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
  Serial.begin(115200); 
  Serial.println("--------------- Setup -------------");

  Serial.println("--------------- Setup Parameter laden -------------");
  Serial.println("Default: ");
  printConfig();
  // Config aus eeprom lesen
  getConfig();
  Serial.println("eeprom: ");
  printConfig();
  int iStatus = myConfig.iAus;

  Serial.println("--------------- Starting WifiManager -------------");

  if (!wifiManager.autoConnect("Servoduino","Servoduino")) {
     Serial.println("failed to connect, we should reset as see if it connects"); 
     delay(3000); 
     ESP.reset(); 
     delay(5000); 
  }
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
  server.on("/an", wwwan); //Parameter timer=sekunden möglich
  server.on("/aus", wwwaus);
  server.on("/ran", wwwrelayan);
  server.on("/raus", wwwrelayaus);
  server.on("/anwert", wwwanwert); //Analog Wert für Anzeige auf Startsite
  server.on("/fullstatus", wwwmelde_FullStatus);
  server.on("/switchlogic", wwwSwitchLogic);
  server.begin();
  Serial.println("--------------- Setup UDP --------");
  Udp.begin(localPort);
  
  Serial.println("--------------- Setup Servo -------------");
  myservo.attach(iServoPin); 
  setServo(getKorrigiert(iStatus));
  delay(500);
  myservo.detach();
  Serial.println("--------------- Setup Pins -------------");
  for (int i=0; i<2 ; i++) {
      pinMode(iPushButtonPin[i], INPUT);
      pinMode(iSwitchPin[i], INPUT);
      if ( myConfig.bExpLogic == 0) {
        digitalWrite(iPushButtonPin[i],HIGH);
        digitalWrite(iSwitchPin[i],HIGH);
      } else {
        digitalWrite(iPushButtonPin[i],LOW);
        digitalWrite(iSwitchPin[i],LOW);      
      }
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
    handleTimer();
    handleRelayTimer();
    delay(100);
}
