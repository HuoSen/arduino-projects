/*
 * Copyright (C) 2014 DFRobot                                                  
 *                                                                             
 * wido_ddns is free software: you can redistribute it and/or         
 * modify it under the terms of the GNU General Public License as       
 * published by the Free Software Foundation, either version 3 of              
 * the License, or any later version.                                          
 *                                                                             
 * wido_ddns is distributed in the hope that it will be useful,       
 * but WITHOUT ANY WARRANTY; without even the implied warranty of              
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               
 * GNU General Public License for more details.                         
 *                                                                             
 * wido_ddns is distributed in the hope that it will be useful,       
 * but WITHOUT ANY WARRANTY; without even the implied warranty of              
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               
 * GNU General Public License for more details.                         
 *                                                                             
 * You should have received a copy of the GNU General Public            
 * License along with wido_ddns. If not, see                          
 * <http://www.gnu.org/licenses/>.                                             
 *                                                                             
 */

/*
 *	name:				wido_ddns library
 *	version:			0.1
 *	Author:				lisper <lisper.li@dfrobot.com>
 *	Date:				2014-08-21
 *	official website:		http://www.dfrobot.com
 *	Description:			ddns that run on wido board, this is sample code
 */
 
#include <Adafruit_CC3000.h>
#include <SPI.h>
#include "utility/debug.h"
#include "utility/socket.h"
#include "wido_ddns.h"

// DDNS settings
#define username "yourname"
#define password "***********"
#define hostname "yourhostname"

// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   7  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
		SPI_CLOCK_DIVIDER); // you can change this clock speed



#define WLAN_SSID       "ssid-name"           // cannot be longer than 32 characters!
#define WLAN_PASS       "*********"
// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2

#define LISTEN_PORT           80   // What TCP port to listen on for connections.

Adafruit_CC3000_Server webServer(LISTEN_PORT);
wido_ddns ddns = wido_ddns (cc3000);
//
void setup(void) {
	pinMode (13, OUTPUT);
	pinMode (12, OUTPUT);
	Serial.begin(115200);
	Serial.println(F("Hello, CC3000!\n")); 
	//while (!Serial);
	//Serial.println ("Input any key to start:");
	//while (!Serial.available ());
	Serial.print("Free RAM: "); 
	Serial.println(getFreeRam(), DEC);

	/* Initialise the module */
	Serial.println(F("\nInitializing..."));
	if (!cc3000.begin()) {
		Serial.println(F("Couldn't begin()! Check your wiring?"));
		while(1);
	}

	Serial.print(F("\nAttempting to connect to ")); 
	Serial.println(WLAN_SSID);
	if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
		Serial.println(F("Failed!"));
		while(1);
	}

	Serial.println(F("Connected!"));

	Serial.println(F("Request DHCP"));
	while (!cc3000.checkDHCP()) {
		delay(100); // ToDo: Insert a DHCP timeout!
	}  

	/* Display the IP address DNS, Gateway, etc. */
	while (! displayConnectionDetails()) {
		delay(1000);
	}

	/*********************************************************/
	/* You can safely remove this to save some flash memory! */
	/*********************************************************/


	// Start listening for connections
	webServer.begin();
	uint32_t myip = 0;
	while (myip == 0) {
		if (! cc3000.getHostByName("nic.changeip.com", &myip)) {
			Serial.println(F("Couldn't resolve!"));
		}
		delay(500);
	}
	Serial.println("ddns ip:");
	cc3000.printIPdotsRev(myip);
	Serial.println();

	//ddns.set_checkip (cc3000.IP2U32 (209,208,4,56));
	ddns.set_ddnsip (myip);
	uint32_t ddnstime = 60000;
	ddns.set_ddns_time (ddnstime);
	Serial.println (ddnstime);
	ddns.set_ddns_get_string (username, password, hostname);

	Serial.println(F("Listening for connections..."));
}

//
void loop(void) {
	ddns.ddns_update ();
	webServerRun ();
}

//
void webServerRun () {
	char databuffer[45];
	// Try to get a client which is connected.
	Adafruit_CC3000_ClientRef client = webServer.available();
	if (client) {
		while (client.available ()) {
			client.read (databuffer, 40);

			char* sub = strchr (databuffer, '\r');
			if (sub > 0)
				*sub = '\0';
			Serial.println (databuffer);
			sub = strstr (databuffer, "control");
			if (!sub) {
				//Serial.println ("no control");			
				break;
			}
			sub = strstr (sub, "led");
			if (!sub) {
				//Serial.println ("no led");			
				break; 
			}
			sub += 4;
			if (strncmp (sub, "open", 4) == 0) {
				Serial.println ("clicked open");
				digitalWrite (12, HIGH);  
				digitalWrite (13, HIGH);  
			} 
			else if (strncmp (sub, "close", 5) == 0) {
				Serial.println ("clicked close");
				digitalWrite (12, LOW);
				digitalWrite (13, LOW);
			}
			break;

		}
		webServer.write ("<!DOCTYPE html>");
		webServer.write ("<html>");
		webServer.write ("<body>");
		webServer.write ("<form action=\"control\" method=\"get\">");
		webServer.write ("<button name=\"led\" type=\"submit\" value=\"open\">Open</button><br />");
		webServer.write ("<button name=\"led\" type=\"submit\" value=\"close\">Close</button>");
		webServer.write ("</form>");
		webServer.write ("</body>");
		webServer.write ("</html>");
		client.close();
	}
	client.close();
}

//
bool displayConnectionDetails(void) {
	uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;

	if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv)) {
		Serial.println(F("Unable to retrieve the IP Address!\r\n"));
		return false;
	} 
	else {
		Serial.print(F("\nIP Addr: ")); 
		cc3000.printIPdotsRev(ipAddress);
		Serial.print(F("\nNetmask: ")); 
		cc3000.printIPdotsRev(netmask);
		Serial.print(F("\nGateway: ")); 
		cc3000.printIPdotsRev(gateway);
		Serial.print(F("\nDHCPsrv: ")); 
		cc3000.printIPdotsRev(dhcpserv);
		Serial.print(F("\nDNSserv: ")); 
		cc3000.printIPdotsRev(dnsserv);
		Serial.println();
		return true;
	}
}


