#include <SoftwareSerial.h>
#include <sim900.h>

const String PHONE = "+XXxxxxxxxxxx";

SoftwareSerial gsm(4, 5); //(rxPin,txPin)

#define RELAY_1 6
#define RELAY_2 7

String senderNumber, date, msg;
boolean isReply = false;

void setup() {

  Serial.begin(9600);
  Serial.println("Arduino serial initialize");

  gsm.begin(9600);
  SIM900 sim900(gsm);
 
  pinMode(RELAY_1, OUTPUT); //Relay 1
  pinMode(RELAY_2, OUTPUT); //Relay 2

  gsm.println("AT"); //Handshaking with SIM900
  updateSerial();
}

void loop()
{
  updateSerial();
}

void updateSerial()
{
  delay(500);
  while (Serial.available()) 
  {
    gsm.write(Serial.read());//Forward what Serial received to Software Serial Portu
  }
  while(gsm.available()) 
  {
    parseData(gsm.readString());
    Serial.write(gsm.read());//Forward what Software Serial received to Serial Port
  }
}

void parseData(String buff){
  Serial.println(buff);

  unsigned int len, index;
  //////////////////////////////////////////////////
  //Remove sent "AT Command" from the response string.
  index = buff.indexOf("\r");
  buff.remove(0, index+2);
  buff.trim();
  //////////////////////////////////////////////////
  // Extract sms data and take action accordingly
  //////////////////////////////////////////////////
  if(buff != "OK"){
    index = buff.indexOf(":");
    String cmd = buff.substring(0, index);
    cmd.trim();
    
    buff.remove(0, index+2);
    
    if(cmd == "+CMT"){
      extractSms(buff);

      if(senderNumber == PHONE){
        doAction();
        //delete all sms
        gsm.println("AT+CMGD=1,4");
        delay(1000);
        gsm.println("AT+CMGDA= \"DEL ALL\"");
        delay(1000);
      }
    }
  //////////////////////////////////////////////////
  }
  else{
  //The result of AT Command is "OK"
  }
}

//************************************************************
void extractSms(String buff){
   unsigned int index;
   
    index = buff.indexOf(",");
    senderNumber = buff.substring(1, index-1); 
    Serial.println("senderNumber :" + senderNumber);
    buff.remove(0, index+2);
    
    index = buff.indexOf("\n");
    date = buff.substring(0, index-1);
    Serial.println("date :" + date);
    buff.remove(0,index+1);
    buff.trim();

    index =buff.indexOf("\n\r");
    buff = buff.substring(0, index);
    buff.trim();

    msg = buff;
    buff = "";
    msg.toLowerCase();
}

void doAction(){
  Serial.println("MSG :" + msg);

  if(msg == "light1 off"){  
    digitalWrite(RELAY_1, LOW);
    Reply("Light 1 has been OFF");
  }
  else if(msg == "light1 on"){
    digitalWrite(RELAY_1, HIGH);
    Reply("Light 1 has been ON");
  }
  else if(msg == "light2 off"){
    digitalWrite(RELAY_2, LOW);
    Reply("Light 2 has been OFF");
  }
  else if(msg == "light2 on"){
    digitalWrite(RELAY_2, HIGH);
    Reply("Light 2 has been ON");
  }
  
  senderNumber="";
  date="";
  msg="";  
}

void Reply(String text)
{
    gsm.print("AT+CMGF=1\r");
    delay(1000);
    gsm.print("AT+CMGS=\""+PHONE+"\"\r");
    delay(1000);
    gsm.print(text);
    delay(100);
    gsm.write(0x1A);
    delay(1000);
    Serial.println("SMS Sent Successfully.");
}
