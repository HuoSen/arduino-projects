/*
        Script for DDNS for Xboard
        Todo:
                turn it into a library
                clean up and comment
        
        Licence CC = CC BY-SA
        Tested on Arduino Xboard, written by Jose
*/


#include "DDNS.h"




DDNS_c::DDNS_c( )
{
	ipadsl = INADDR_NONE;

	ipcheck = DDNS_CHECK_IP;
    checkPort = DDNS_CHECK_PORT;

}

void DDNS_c::init( IPAddress ddnsPara, char *pUser, char *pPwd )
{
	ddns = ddnsPara;
	phostname = pUser;
	puserpwdb64 = pPwd;
}



void DDNS_c::update( )
{
  int timeout=0;
  int counter = 0;
  int ipi = 0;
  int codetemp=0;

  IPAddress ipadslold;

  Serial.print("getting current IP \n");
  Serial.print("connecting to ");
  Serial.print(ipcheck);

  if (client.connect(ipcheck, 8245)) 
  {
    Serial.println("\n       client connected ");
    client.println("GET / HTTP/1.0");
    client.println();
  } 
  else {
    Serial.println("\nconnection failed");
  }

  while (!client.available() && timeout<50)
  {
    timeout++;
    Serial.print("Time out ");
    Serial.println(timeout);
    delay(100);
  }

  while (client.available())
  {
    if (client.available())
    { 
      char c = client.read();
      if (c == '\n')
      {
        counter++;
      } 
      if (counter == 9 && c != '\n')
      {
        if(c == '.' )
        {
          ipadsl[ipi] = codetemp;
          codetemp = 0;
          ipi++;
        } 
        else {
          codetemp = codetemp *10 + (c-0x30);
        }
        ipadsl[3] = codetemp;
      }
    }
  }
  client.flush();

  if (!client.connected())
  {
    Serial.println("disconnecting. \n");
    client.stop();
    delay (1000);
    if ( ipadsl == INADDR_NONE )
	{
      //nothing
	}
	else
    {
      Serial.print("New IP: >");
      Serial.print(ipadsl);
      Serial.println("<");             
      for (int i=0; i<4; i++) {    //save in memory
        ipadslold[i] = EEPROM.read(i);
      }
      Serial.print("Old IP: >");
      Serial.print(ipadslold);
      Serial.println("<");
      if (ipadsl != ipadslold)
      {
        Serial.println("IP different from PUBLIC IP");
        ddns_changer();
      }
      else
      {
        Serial.println("same IP");
      }

      ipadslold = ipadsl;
      for (int i=0; i<4; i++)
      {
        EEPROM.write(i, ipadsl[i]);                // http://www.arduino.cc/en/Reference/EEPROMWrite
      }
      Serial.println("IP saved !");
    }
	
  }
}



void DDNS_c::ddns_changer()
{
  int timeout=0;
  Serial.print("connecting to");
  Serial.println(ddns);

  Serial.print("Public IP");
  Serial.println(ipadsl);

  if (client.connect(ddns, 80)) {
    Serial.println("connected");
    client.print("GET /nic/update?hostname=");
    client.print(phostname);
    client.print("&myip=");
    client.print(ipadsl);
    client.println(" HTTP/1.1 ");
    client.print("Host: ");
    client.println(ddns);

    client.print("Authorization: Basic ");
    client.println(puserpwdb64);

    client.println("User-Agent: DFRobot - jose at dfrobot.com");
    client.println();
  } 
  else {
    Serial.println("connection failed");
  }

  while (!client.available() && timeout<50)
  {
    timeout++;
    Serial.print("Time out");
    Serial.println(timeout);
    delay(100);
  }

  while (client.available())
  {
    if (client.available())
    {
      char c = client.read();
      Serial.print(c);
    }
  }

  if (!client.connected())
  {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
  }
}

