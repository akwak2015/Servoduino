String htmlSwitch(String sName, int bStatus, String sID ="") {
  String sReturn;
  String sStatus = "";
  if (bStatus==1) {
    sStatus="checked";
  }
  sReturn = "<label class=\"switch\">";
  sReturn += "<input type=\"checkbox\" name=\""+sName+"\" "+ sStatus ;
  if (sID !="") {sReturn += " onchange = \"showhide('"+sID + "' , this.checked)\"";}
  sReturn += ">";
  sReturn += "<div class=\"slider round\"></div>";
  sReturn += "</label>";
  return(sReturn);
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
    sResponse += cssSwitch();
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


void wwwSetup() {
  Serial.println("wwwrSetup begin");
  String sResp="";
  sResp += "<h1>Setup Page</h1>\n";
  sResp += "<div class=\"all\">\n";
  sResp += "<form action=\"/save\" id=\"Servoduino\"  method=\"post\" autocomplete=\"off\">\n";
  sResp += "  <p class=\"sys2inf\"><b>Servo Parameter</b></p>\n";
  sResp += "  <table class=\"fixed\">\n";
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
  sResp += "        Schwelle Analogeingang\n";
  sResp += "      </TD>\n";
  sResp += "      <TD class=\"second\">\n";
  sResp += "        <input class=\"field\" name=\"AnalogSchwelle\" type=\"number\" min=\"0\" max=\"9999\" step=\"1\" value=\""+ String(myConfig.iAnalogSchwelle) + "\">\n";
  sResp += "      </TD>\n";
    //Analog Timer
  sResp += "    </TR>\n";
  sResp += "    <TR>\n";
  sResp += "      <TD class=\"first\">\n";
  sResp += "        Timer Analogeingang (Sek)\n";
  sResp += "      </TD>\n";
  sResp += "      <TD class=\"second\">\n";
  sResp += "        <input class=\"field\" name=\"AnalogTime\" type=\"number\" min=\"5\" max=\"3600\" step=\"5\" value=\""+ String(myConfig.iAnalogTimer) + "\"/></TD>\n";
  sResp += "      </TD>\n";
  sResp += "    </TR>\n";

  sResp += "</table>";

  //Schalter
  sResp += "  <table class=\"fixed\">\n";
  // -- MagnetSchalter 
//  sResp += "    <TR>\n";
//  sResp += "      <TD class=\"firstButton\">\n";
//  sResp += htmlSwitch("MagnetSchalter",myConfig.bMagnetsensor,"");
//  sResp += "    </TD>\n";
//  sResp += "      <TD class=\"second\">\n";
//  sResp += "        Magnet Sensor vorhanden\n";
//  sResp += "      </TD>\n";
//  sResp += "  </TR>\n";
  // -- Schalter1   
  sResp += "  <TR>\n";
  sResp += "      <TD class=\"firstButton\">\n";
  sResp += htmlSwitch("SchalterWieTaster1",myConfig.bSwitchAsPushButton[0] ,"swt1");
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
//  sResp += "  <TR>\n";
//  sResp += "      <TD class=\"firstButton\">\n";
//  sResp += htmlSwitch("SchalterWieTaster2",myConfig.bSwitchAsPushButton[1] ,"swt2");
//  sResp += "    </TD>\n";
//  sResp += "      <TD class=\"second\">\n";
//  sResp += "        Schalter 2 wie Taster\n";
//  sResp += "      </TD>\n";
//  sResp += "        <!-- if (myConfig.bSwitchAsPushButton) { -->\n";
//  sResp += "  </TR>\n";
//  sResp += "  <TR id=\"swt2\" class=\"hidden\" " + htmlDisplay(myConfig.bSwitchAsPushButton[1]) + ">\n";
//  sResp += "    <TD class=\"first\">Schaltzeit: </TD>\n";
//  sResp += "    <TD class=\"second\"><input class=\"field\" name=\"SchalterTime2\" type=\"number\" min=\"5\" max=\"3600\" step=\"5\" value=\""+ String(myConfig.iSwitchTimer[1]) +  "\"/></TD>\n";
//  sResp += "  </TR>\n";
  // -- Taster 1
  sResp += "  <TR>\n";
  sResp += "    <TD class=\"firstButton\">\n";
  sResp += htmlSwitch("TasterMitTimer1",myConfig.bPushButtonTimer[0] ,"tmt1");
  sResp += "    </TD>\n";
  sResp += "    <TD class=\"second\">\n";
  sResp += "      Taster 1 mit eigenem Timer\n";
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
  sResp += "    <TD class=\"firstButton\">\n";
  sResp += htmlSwitch("TasterMitTimer2",myConfig.bPushButtonTimer[1] ,"tmt2");
  sResp += "    </TD>\n";
  sResp += "    <TD class=\"second\">\n";
  sResp += "      Taster 2 mit eigenem Timer\n";
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
  sResp += htmlSwitch("HM",myConfig.bHomematic ,"Homematic");
// 
//  sResp += "    <input type=\"checkbox\" name=\"HM\" id=\"HM\" onchange = \"showhide('Homematic', this.checked)\" ";
//                       if(myConfig.bHomematic==1) { sResp += "checked=\"checked\"\n";}
//                        sResp += ">\n";
//
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
  sResp += "          CUxD Ger&auml;t (Servo):\n";
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

    sResp += htmlSwitch("Lox",myConfig.bLoxone ,"Loxone");
//  sResp += "    <input type=\"checkbox\" name=\"Lox\" id=\"Lox\" onchange = \"showhide('Loxone', this.checked)\" ";
//                       if(myConfig.bLoxone==1) { sResp += "checked=\"checked\"\n";}
//                        sResp += ">\n";

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
  // ------------------
  // Expertenparameter
  // ------------------
//  sResp += "  <p class=\"sys2inf\"><b>Sonstige Einstellungen</b></p>\n";
//  sResp += " <div id=\"ExpertParameter\">\n";
//  sResp +=      htmlSwitch("Expert",0 ,"Expertenparameter");
//  sResp += "    <label for=\"Expert\">Expertenparameter</label>\n";
//  sResp += "    <div id=\"Expertenparameter\" " + htmlDisplay(0) + ">\n";
//  sResp += "    Änderungen in diesem Bereich können die Funktionalität nachhaltig stören";
//  sResp += "      <table class=\"Expert\">\n";
//  sResp += "        <TR>\n";
//  sResp += "          <TD class=\"firstButton\">\n";
//  sResp +=              htmlSwitch("Logic",myConfig.bExpLogic,"");
//  sResp += "          </TD>\n";
//  sResp += "          <TD class=\"second\">\n";
//  sResp += "            Taster & Schalter ueber +5V (Standard = OFF)\n";
//  sResp += "          </TD>\n";
//  sResp += "        </TR>\n";
//  sResp += "      </table>\n";
//  sResp += "    </div>\n";
//  sResp += "  </div>\n";
    
  sResp += "  <div style =\"padding-top: 30px;position: relative;\">\n";
  sResp += "  <button type=\"submit\" name =\"action\" style =\"position: absolute;right: 10px;top: 5px;\"> Absenden </button>\n";
  sResp += "  </div>\n";
  sResp += "</form>\n";
  sResp += "</div>\n";
  sResp += "<br><a href='/'><button type='button'>Startseite</BUTTON></a> \n";
  htmlresponse("Setup", sResp);
  Serial.println("wwwrSetup end");
}

void wwwRoot() {
  Serial.println("wwwrRoot begin");

  String sResp;
  sResp = "<script type=\"text/javascript\" language=\"JavaScript\">";
//  sResp +="   setInterval(sndReq, 500);";
  sResp +="   setInterval(sndReqFull, 1000);";
  sResp +="</script>\n";
  sResp += "<h1>Servo Steuerung</h1>\n";
  if (myConfig.bExpLogic == 1) {
    sResp += "<H1 stype=\"color:red\">Achtung Falsche Logik für Holgers Rahmen!</H1>";
  }
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

void wwwmelde_status() {
  int iErg = getOnOffStatus(true);
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
  
  if (bTimerAktiv) { 
      int iTime = round((iTimerMillis-millis())/1000);
      sResp += "<td class=\"timeron\">Timer aktiv für : </td><td class=\"timeron\">" + String(iTime)  + "</TD>\n";  
      }
  
  sResp += "</tr><tr>";
  sResp += "<td>Aktueller Relay Status: </td>";
  if (iRelay == 1) { sResp += "<td class=\"on\">An</td>\n";}
  if (iRelay == 0) { sResp += "<td class=\"off\">Aus</td>\n";}
   if (bRelayTimerAktiv) { 
      int iTime = round((iRelayTimerMillis-millis())/1000);
      sResp += "<td class=\"timeron\">Timer aktiv für :  </td><td class=\"timeron\">" + String(iTime)  + "</TD>\n";  
      }
  

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
    Serial.println("htmlresponse0");
    if (sText != "") {
      server.send(200, "text/plain", sText);
      Serial.println("htmlresponse0 Plain Text send");
    }
}

void wwwSwitchLogic() {
  String sResponse;
  if (myConfig.bExpLogic == 1) {
     myConfig.bExpLogic = 0; 
     Serial.println("Logic entsprechend Voreinstellung - Schalten ueber GND");
     sResponse = "Toggle Logic: Logic entsprechend Voreinstellung - Schalten &uuml;ber GND";
  }
  else {
     myConfig.bExpLogic = 1;
     Serial.println("Logic abweichend von Voreinstellung - Schalten ueber +5V");
     sResponse = "Toggle Logic: Logic abweichend von Voreinstellung - Schalten &uuml;ber +5V";
  }
  saveConfig;
  delay(200);
  htmlreloadpresite("/", sResponse); 
}

