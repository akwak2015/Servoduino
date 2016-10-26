// TOOLs allgemein------
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

bool logichandler(int iPin) {
  int iStateTmp = digitalRead(iPin);
  if (myConfig.bExpLogic == 1) { // High Logic
    //Serial.println("Positive Logic"); 
    return iStateTmp;
  }
  else {  // GND Logic daher 0 und 1 vertauschen
    //Serial.println("Negative Logic"); 
    if (iStateTmp == 0) {
      return 1;
    } else {
      return 0;
    }
  }
}

void handlePushButton(int iPB) {
    unsigned long currentMillis = millis();
    // Entprellen 
    if ((currentMillis - tPushButtonLast[iPB]) > 500) {
        //Serial.printf("Handle Pushbutton - Current: %i, Last: %i, Diff: %i", currentMillis, tPushButtonLast[iPB], (currentMillis - tPushButtonLast[iPB]));
        //Serial.println("");
        int iPushButtonState = logichandler(iPushButtonPin[iPB]);
        if ((iPushButtonState == 1) and iPushButtonState != iPushButtonLastState[iPB]) {
            if (myConfig.bPushButtonTimer[iPB]) {
              setTimerSec(myConfig.iPushButtonTimer[iPB]);
              setPosAn();
            }
            else {
              toggle();
              bTimerAktiv=false; //Wenn manuell an oder ausgeschaltet, dann brauchen wir keinen Timer mehr
            }
            iPushButtonLastState[iPB] = iPushButtonState;
            tPushButtonLast[iPB]=currentMillis;
            sendUDP("Taster[" + String(iPB) + "]1");
        }
        // Status Losgelassen merken und an Geräte übermitteln
        else if ((iPushButtonState == 0) and iPushButtonState != iPushButtonLastState[iPB]) {
            iPushButtonLastState[iPB] = iPushButtonState;
            sendUDP("Taster[" + String(iPB) + "]0");
            }
    }
}
void handleSwitch(int iSW){
    unsigned long currentMillis = millis();
    int iSwitchState = logichandler(iSwitchPin[iSW]);  
    if (iSwitchState != iSwitchLastState[iSW]) {
        //Switch on
        if (iSwitchState == 1) {
            if (myConfig.bSwitchAsPushButton[iSW]) {
              setTimerSec(myConfig.iSwitchTimer[iSW]);
            }
            else {
              bTimerAktiv=false; //Wenn manuell an oder ausgeschaltet, dann brauchen wir keinen Timer mehr
            }
            setPosAn();
            sendUDP("Schalter[" + String(iSW) + "]1");
         } 
         //Switch off
         if (iSwitchState == 0) {
                if (myConfig.bSwitchAsPushButton[iSW]) {
                  Serial.println("Switch turns off in Timermode... nothinmg to do");
                }
                else {
                  setPosAus();
                  bTimerAktiv=false; //Wenn manuell an oder ausgeschaltet, dann brauchen wir keinen Timer mehr
                }
                sendUDP("Schalter[" + String(iSW) + "]0");
         }
         iSwitchLastState[iSW] = iSwitchState; 
         tSwitchLast[iSW]=currentMillis;
      }
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
         int iStatus = getOnOffStatus(true);
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

void handleTimer(){
  if (bTimerAktiv) {
    int iTime=millis();
    //Serial.println(String(iTimerMillis) + " - " + String(iTime) + " = " + String(iTimerMillis - iTime));
    if (iTime > iTimerMillis) {
         setPosAus();
         sendUDP("Timer0");
         bTimerAktiv=false;
    }
  } 
}

void handleRelayTimer(){
  if (bRelayTimerAktiv) {
    int iTime=millis();
    //Serial.println(String(iRelayTimerMillis) + " - " + String(iTime) + " = " + String(iRelayTimerMillis - iTime));
    if (iTime > iRelayTimerMillis) {
         wwwrelayaus();
         sendUDP("RTimer0");
         bRelayTimerAktiv=false;
    }
  } 
}
