#include <AltSoftSerial.h>
AltSoftSerial wifiSerial;
//NOTE!
//You must download and install the AltSoftSerial library from here first: https://www.pjrc.com/teensy/td_libs_AltSoftSerial.html
  
String wifiSsid = "ssidGoesHere";

String wifiAuthenticationType = "WPA2PSK";
//wifiAuthenticationType=
//  OPEN
//  SHARED
//  WPAPSK
//  WPA2PSK

String wifiEncryptionType = "AES";
//wifiEncryptionType=
//  NONE (For auth=OPEN)
//  WEP (For auth=OPEN or SHARED)
//  TKIP (For auth=WPAPSK or WPA2PSK)
//  AES (For auth=WPAPSK or WPA2PSK)

String wifiKey = "wifiKeyGoesHere";
String wifiIpAddress = "192.168.100.250";
String wifiSubnetMask = "255.255.255.0";
String wifiGateway = "192.168.100.1";
String wifiDnsServer = "192.168.100.1";

void setup() {
  
  Serial.begin(9600);
  while (!Serial); // wait for serial port to connect. Needed for Leonardo only
  
  wifiSerial.begin(9600); // AT+UART=9600,8,1,None,NFC
}

void loop() {
  //check for output from wifi module
  if (wifiSerial.available()) {
    Serial.write(wifiSerial.read());
  }
  
  //check for command input
  if (Serial.available()) {
    char currentChar = Serial.read();
    // uncomment this if you want to see whatever you type into the console displayed back
    //Serial.print(currentChar);
    
    switch(currentChar){

      case '1':
        Serial.println("1 Enter CMD Mode, sending +++ then a");
        enterCommandMode();
        break;
        
      case '2':
        Serial.println("2 Check For CMD Mode, sending AT+");
        sendWifiSerialCmd("AT+");
        break;
        
      case '3':
        Serial.println("3 Show Help, sending AT+H");
        sendWifiSerialCmd("AT+H");
        break;
        
      case '4':
        Serial.println("4 Scan for Wifi, sending AT+WSCAN");
        sendWifiSerialCmd("AT+WSCAN");
        break;
        
      case '5':
        Serial.println("5 Connect Wifi");
        
        Serial.println("5.1 Setting Wireless mode to AP+STA, sending AT+WMODE=APSTA");
        sendWifiSerialCmdWaitForOk("AT+WMODE=APSTA");
        
        Serial.println("5.2 Setting Wifi STA SSID, sending AT+WSSSID=");
        sendWifiSerialCmdWaitForOk("AT+WSSSID=" + wifiSsid);
        
        Serial.println("5.3 Setting STA Wifi Key, sending AT+WSKEY=");
        sendWifiSerialCmdWaitForOk("AT+WSKEY=" + wifiAuthenticationType + "," + wifiEncryptionType + "," + wifiKey);
        
        Serial.println("5.4 All Settings Saved Rebooting, sending AT+Z");
        sendWifiSerialCmd("AT+Z");
        
        break;
        
      case '6':
        Serial.println("6 Set IP to DHCP, sending AT+WANN=DHCP");
        sendWifiSerialCmdWaitForOk("AT+WANN=DHCP,0.0.0.0,0.0.0.0,0.0.0.0");
        Serial.println("6.1 Setting Saved Rebooting, sending AT+Z");
        sendWifiSerialCmd("AT+Z");
        break;
       
      case '7':
        Serial.println("7 Set IP to STATIC, sending AT+WADHCP=static");
        sendWifiSerialCmd("AT+WANN=static," + wifiIpAddress + "," + wifiSubnetMask + "," + wifiGateway);
        Serial.println("7.1 Setting DNS, sending AT+WSDNS=");
        sendWifiSerialCmdWaitForOk("AT+WSDNS=" + wifiDnsServer);
        Serial.println("7.2 Setting Saved Rebooting, sending AT+Z");
        sendWifiSerialCmd("AT+Z");
        break;

      case '8':
        Serial.println("8 Ping Google.com, sending AT+PING=google.com");
        sendWifiSerialCmd("AT+PING=google.com");
        //Should Show "+ok=Success" if all parameters are correct
        break;
        
      case '9':
        Serial.println("9 Send HTTP Request on Socket B");
        
        sendHttpRequestSockB();
        
        break;
        
      default: //If no command # sent pass through whatever input
        wifiSerial.write(currentChar);
    }
    
  } 
}

boolean sendWifiSerialCmd(String command){
  wifiSerial.print(command + "\r");
}

boolean sendWifiSerialCmdWaitForOk(String command){
  
  sendWifiSerialCmd(command);
  
  if(wifiSerial.find("+ok")){
    return true;
  }else{
    Serial.println("Error! No +ok received from command!");
    return false;
  }
  
}

void sendHttpRequestSockB()
{
  Serial.println("Send HTTP Request on Socket B!");
  
  //Serial.println("Disable Command Echo");
  //sendWifiSerialCmdWaitForOk("AT+E=off");
  
  Serial.println("Disconnecting Socket B, sending AT+TCPDISB=off");
  sendWifiSerialCmdWaitForOk("AT+TCPDISB=off");
  
  Serial.println("Setting Socket B Parameters, sending AT+SOCKB=");
  sendWifiSerialCmdWaitForOk("AT+SOCKB=TCP,80,www.google.com");
  
  Serial.println("Connecting Socket B, sending AT+TCPDISB=on");
  sendWifiSerialCmdWaitForOk("AT+TCPDISB=on\r");
  
  Serial.println("Wait for socket B to connect");
  if( !waitForSocketBToConnect() ){
    Serial.println("Timeout waiting 1 second for connection to be made. Quitting.");
    sendWifiSerialCmdWaitForOk("AT+TCPDISB=off");
    return;
  }

  //Woops - Pass through only applies to socket A. Save this for later
  //Serial.println("Enabling Serial Pass Through");
  //sendWifiSerialCmdWaitForOk("AT+ENTM\r");

  char HttpRequest[] = "GET / HTTP/1.1\r\n"
      "Host: www.google.com\r\n";
      "\r\n";
      
  unsigned int HttpRequestLength = sizeof(HttpRequest);
  
  Serial.println("Sending Data, sending AT+SNDB=");
  Serial.println("AT+SNDB=" + (String)HttpRequestLength );
  wifiSerial.print("AT+SNDB=" + (String)HttpRequestLength  + "\r" );
  wifiSerial.find(">");
  
  Serial.println("Sending HTTP Request");
  //output to terminal for debugging
  //Serial.print(HttpRequest);
  wifiSerial.print(HttpRequest);
  //wifiSerial.find("+ok");
  
  wifiSerial.flush();
  
  Serial.println("Receiving Back 16 Bytes");
  
  //Wait for OK? To see full output waiting for OK seems to rollover the buffer since google.com is pretty big
  //sendWifiSerialCmdWaitForOk("AT+RCVB=12\r");
  
  wifiSerial.print("AT+RCVB=12\r");
  //Expecting to see "HTTP/1.1 200 OK" on the terminal
  
  //wait a bit
  delay(250);
  
  //Serial.println("Disconnecting Socket B, sending AT+TCPDISB=off");
  sendWifiSerialCmdWaitForOk("AT+TCPDISB=off");
  
}

boolean waitForSocketBToConnect(){
 unsigned long waitTimeout = 1000; // wait 1 second to connect
 unsigned long start = millis();
 char lastCmdResultChar;
 
  wifiSerial.print("AT+TCPLKB\r");
    
  while( millis() - start < waitTimeout ){
    
    if (wifiSerial.available()) { //Output should be "+ok=on"
        lastCmdResultChar = wifiSerial.read();
        //uncomment this for debugging. Will show "+ok=off" until a connection is made
        //Serial.write(lastCmdResultChar);
    }
    if(lastCmdResultChar == 'n'){ // Command Returned "+ok=on"
       Serial.println("Connected in " + (String)(millis() - start) + " ms");
      return true;
    }else if(lastCmdResultChar == 'f'){ // Command Returned "+ok=off"
      lastCmdResultChar = 'x'; //Reset the character so we don't issue this command repeatedly
      wifiSerial.print("AT+TCPLKB\r");
    }
  }
  return false;
}

boolean enterCommandMode()
{
  Serial.println("Enter Command Mode");
  
  wifiSerial.write('+');
  delay(50);
  wifiSerial.write('+');
  delay(50);
  wifiSerial.write('+');
  if(!wifiSerial.find("a")){
    Serial.println("Error! No Ack from wifi module!");
    return false;
  }
  
  wifiSerial.write('a');
  if(!wifiSerial.find("+ok")){
    Serial.println("Error! No OK received from wifi module!");
    return false;
  }  
  Serial.println("Ready for commands!");
  return true;
}
