# Servoduino
Mit einem Wemos Microcontroller und einem Servo Wandtablets mit Magnetsensor an und ausschalten. 

Zuerst einmal Danke an alle, die ihre Informationen zur Arduino Programmierung mit anderen teilen. ICh nutze in diesem Projekt Bibliotheken, die über den Manager downloadbar sind. 
Für die meisten Codeteile habe ich mich im Internet inspirieren lassen. Sollte sich jemand nicht ausreichend erwähnt finden, bitte direkt bescheid sagen, damit ich nachbessern kann.

####Was ist Servoduino?
Im Homematic Forum sind viele aktive User, die ein Tablett an die Wand hängen um ihre Hausautomation zu steuern. Dabei werden die verschiedensten Verfahren genutzt um as Tablett zur richtigen Zeit an- und auszschalten.
Desweitern hat jeder User seine eigene Ladestrategie um den Tablettakku zu Schonen.
Servoduino soll viele dieser Möglichkeiten zusammenfassen und kombinieren und das mit einem überschaubaren Hardwareaufwand.

####Ist Servoduino einsatzfähig
Teilweise ist es schon einsatzfähig.
#####Was läuft:
	-	Servo an/aus per Taster, Schalter und HTTP/UDP
	-	Relais an/aus per HTTP/UDP
	-	Rückmeldung an die Homematic CCU
#####Was läuft vieleicht:
	-	Rückmeldung an Loxone (ungetestet, muß gegebenenfalls korrigiert werden)
	-	Rückmeldung mit zusätzlichem Reed/Magnet Kontakt
	-	Analoger Eingang
#####Was läuft nicht:
	-	Option Schalter wie Taster (mit eigenem Timer)
	-	Option Taster mit eigenem Timer
	
####Welche Hausautomationssysteme werden unterstützt?
Aktuell werden unterstützt:
	-	Homematic
	-	Loxone
	
####Welche Protokolle werden zur Steuerung genutzt?
	-	HTTP
	- 	UDP
	
####Was wird benötigt?
Auf jeden Fall einen Wemos D1 Mini. (http://www.wemos.cc/Products/d1_mini.html)
Optional:
	-	Servo mit Magnet 
	-	Relay oder Relayshield (http://www.wemos.cc/Products/relay_shield_v2.html)
	-	Max. 2 Taster
	-	Max. 2 Schalter
	-	Analoger Ausgang bis 3V (z.B. Bewegungsmelder)

	
	
####Welche Befehle werden verstanden?
#####HTTP:
	http://ip						Einfache Übersichts Seite 
	http://ip/setup 				Konfiguration
	http://ip/an					stellt den Servomotor in Position an
	http://ip/aus					stellt den Servomotor in Position aus
	http://ip/ran					Relais an
	http://ip/raus					Relais aus
	
	http://ip/status				Status des Servomotor 

	http://ip/resetwifi 			WIFI Einstellungen zurücksetzen
	http://ip/resetconfig 			Einstellungen löschen 
	http://ip/werkseinstellungen	Einstellungen und WIFI löschen
	http://ip/restart				Wemos neu starten
	
#####UDP:
	an								Servo auf an
	aus								Servo auf aus
	ran								Relais an
	raus							Relais aus
	status							liefert 1 oder 0 zurück für Position des Servo
	rstatus							liefert 1 oder 0 zurück für Position des Relais
	
######Warum sind die HTML Befehle so "komisch"
Viele von Euch würden jetzt warscheinlich Befehle im Format HTTP://IP/SERVO?POS=1 oder ähnlich erwarten. 
Grund dafür ist die Homematic Benutzeroberfläche. Da gibt es bei der Eingabe von Sonderzeichen für URLs leicht Probleme. Die o.a. Befehle lassen sich ohne Tricks eingeben.

#### Wie binde ich die Homematic ein?
Dafür wird uf der CCU das Addon CUxD benötigt (http://homematic-forum.de/forum/viewtopic.php?f=37&t=15298)
Ein Gerät (28) System als Schalter anlegen.
![alt tag](https://raw.githubusercontent.com/akwak2015/Servoduino/trunk/Docs/images/HM0.jpg)
Mit 2 Kanälen für Servo und Relais.
In Param1: /usr/local/addons/cuxd/curl -s -m 5
in Param2: http://ip.des.wemos.eintragen
![alt tag](https://raw.githubusercontent.com/akwak2015/Servoduino/trunk/Docs/images/HM1.jpg)
Die Seriennummern beider Kanäle notieren
![alt tag](https://raw.githubusercontent.com/akwak2015/Servoduino/trunk/Docs/images/HM2.jpg)

Die IP der Homematic und die Seriennummern der beiden Kanäle im Setup auf dem Wemos eintragen
![alt tag](https://raw.githubusercontent.com/akwak2015/Servoduino/trunk/Docs/images/IF0.jpg)

####Wie binde ich Loxone ein?
Die Anwendung sendet an die angegebene IP Adresse auf dem angegebenen Port folgende Status:
	TAB[1|0]
	RELAY[1|0]

 


  
