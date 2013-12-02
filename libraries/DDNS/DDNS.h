#ifndef _DDNS_H
#define _DDNS_H

#include <Ethernet.h>
#include <EEPROM.h>


#define DDNS_CHECK_IP (uint32_t)0xD055F16B //208,85,241,107
#define DDNS_CHECK_PORT 8245


class DDNS_c
{
public:
	DDNS_c( );
	void init( IPAddress ddns , char *pUsername, char *pPassword );

	void update();
	
	
private:
	
	EthernetClient client;
	
	IPAddress ipadsl;
	IPAddress ipcheck; // dyndns ip checker
	uint16_t checkPort;
	IPAddress ddns; // http://nic.ChangeIP.com/nic/update
	char *phostname;
	char *puserpwdb64;  //http://www.functions-online.com/base64_encode.html    use->   username:password


	void ddns_changer();
	

};


#endif
