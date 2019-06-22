#include <Debug.h>
#include <JSN270.h>
#include <Arduino.h>
#include <SoftwareSerial.h>


#define SSID      "hanul302_2.4G"    // your wifi network SSID
#define KEY       "hanul302"    // your wifi network password
#define AUTH       "WPA2"     // your wifi network security (NONE, WEP, WPA, WPA2)

#define USE_DHCP_IP 1

#if !USE_DHCP_IP
#define MY_IP          "192.168.0.30"
#define SUBNET         "255.255.255.0"
#define GATEWAY        "192.168.0.1"
#endif

#define SERVER_PORT    80
#define PROTOCOL       "TCP"

//전등 상태
int led_status = 0;
//보일러 상태
int boiler_status = 0;

//사용할 변수 선언 ========================================
int btn = 13;
int led_red = 9;
int led_blue = 4;
int boiler = 12;
int servoPin = 11;
int m_max=2400;
int check_window = 3;
//int temperature = 0;
//=======================================================
    
SoftwareSerial mySerial(3, 2); // RX, TX

JSN270 JSN270(&mySerial);

char c;

void setup() { 

  mySerial.begin(9600);
  Serial.begin(9600);
  
  //pinMode 셋팅 =====================================================
  pinMode(led_red, OUTPUT);
  pinMode(A0,INPUT);  // 습도 체크
  pinMode(5,INPUT);
  pinMode(led_blue,OUTPUT);
  //pinMode(12,OUTPUT); 
  //pinMode(0,OUTPUT);
  pinMode(servoPin, OUTPUT);
  pinMode(boiler,OUTPUT);
//  pinMode(gasPin, OUTPUT);
  //=================================================================

  
  Serial.println("--------- JSN270 Simple HTTP server Test --------");

  // wait for initilization of JSN270
  delay(5000);
  //JSN270.reset();
  delay(1000);

  //JSN270.prompt();
  JSN270.sendCommand("at+ver\r");
  delay(5);
  while(JSN270.receive((uint8_t *)&c, 1, 1000) > 0) {
    Serial.print((char)c);
  }
  delay(1000);

#if USE_DHCP_IP
  JSN270.dynamicIP();
#else
  JSN270.staticIP(MY_IP, SUBNET, GATEWAY);
#endif    
    
  if (JSN270.join(SSID, KEY, AUTH)) {
    Serial.println("WiFi connect to " SSID);
  }
  else {
    Serial.println("Failed WiFi connect to " SSID);
    Serial.println("Restart System");

    return;
  }    
  delay(1000);

  JSN270.sendCommand("at+wstat\r");
  delay(5);
  while(JSN270.receive((uint8_t *)&c, 1, 1000) > 0) {
    Serial.print((char)c);
  }
  delay(1000);        

  JSN270.sendCommand("at+nstat\r");
  delay(5);
  while(JSN270.receive((uint8_t *)&c, 1, 1000) > 0) {
    Serial.print((char)c);
  }
  delay(1000);
}

void loop() {
  if (!JSN270.server(SERVER_PORT, PROTOCOL)) {
    Serial.println("Failed connect ");
    Serial.println("Restart System");
  } else {
    Serial.println("Waiting for connection...");
  }

  if(check_window == 1 ){
    open();
  }else if(check_window == 2){
    close();
  }
  
  String currentLine = "";                // make a String to hold incoming data from the client
  int get_http_request = 0;
  check_window = 3;
  int tmp; // 푸쉬알림 체크용
  int moisture = 0;
  char light[2];

  //문 상태 체크 ==================
  int a = digitalRead(5);
//  char doorCheck[20];
//  if(tmp != a){
//    strcpy(doorCheck,"push");
//  }else{
//    strcpy(doorCheck,"noPush");
//  }
  if(a == 1){
    digitalWrite(led_blue,LOW);
  }else{
    digitalWrite(led_blue,HIGH);
    a = 0;
  }
//  tmp = a;
 //===============================

  //습도체크===========================
  //moisture = analogRead(A0);  
  
  //==================================
  //온도체크============================
//  int val;
//  val=analogRead(1);
//  temperature=(500*val)>>10;
  //====================================
  while (1) {
    if (mySerial.overflow()) {
      Serial.println("SoftwareSerial overflow!");
      mySerial.flush();
    }
   
    if (JSN270.available() > 0) {
      //명령어
      char c = JSN270.read();
      Serial.print(c);

      if (c == '\n') {                    // if the byte is a newline character
        if (currentLine.length() == 0) {
          if (get_http_request) {
            //시작시 값 셋팅 ============================================
            //analogWrite(12,map(moisture,0,1024,0,255));

//            //전등 체크
//            int tmp = digitalRead(led_red);
//            if(tmp){
//              strcpy(light,"Y");
//            }else{
//              strcpy(light,"N");
//            }
            // ========================================================

            
            Serial.println("new client");
            //Serial.println("HTTP RESPONSE");
            // Enter data mode
            JSN270.sendCommand("at+exit\r");
            delay(100);
            
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            JSN270.println("HTTP/1.1 200 OK");
            JSN270.println("Content-type:text/html");
            JSN270.println();

            // the content of the HTTP response follows the header:           
            JSN270.print("{\"moisture\":");
            JSN270.print(moisture);
//            JSN270.print(",\"light\":\"");
//            JSN270.print(digitalRead(led_red));
//            JSN270.print("\"");
//            JSN270.print(",\"temperature\":");
//            JSN270.print(temperature);
//            JSN270.print("");
            JSN270.print(" ,\"boiler\":\"");
            JSN270.print(boiler_status);
            JSN270.print("\",\"door\":\"");
            JSN270.print(a);
//            JSN270.print("\",\"doorCheck\":\"");
//            JSN270.print(doorCheck);
            JSN270.print("\"}");
            // The HTTP response ends with another blank line:
            JSN270.println();

            // wait until all http response data sent:
            delay(500);

            // Enter command mode:
            JSN270.print("+++");
            delay(100);
            
            // break out of the while loop:
            break;
          }
        }
        // if you got a newline, then clear currentLine:
        else {                
          // Check the client request:
          if (currentLine.startsWith("GET / HTTP")) {
            Serial.println("HTTP REQUEST");
            get_http_request = 1;
           
          }
          else if (currentLine.startsWith("GET /H")) {
            //Serial.println("GET HIGH");
            get_http_request = 1;
            digitalWrite(led_red,HIGH);
            led_status = 1;
          }
          else if (currentLine.startsWith("GET /L")) {
            //Serial.println("GET LOW");
            get_http_request = 1;
            digitalWrite(led_red,LOW);
            led_status = 0;
          }
          //창문 열게 만들어야함
          else if (currentLine.startsWith("GET /O")) {
            get_http_request = 1;
            check_window = 1;
          }
          //창문 닫게 만들어야함
          else if (currentLine.startsWith("GET /C")) {
            get_http_request = 1;
            check_window = 2;
          }
          //보일러 켜게 만들어야함
          else if (currentLine.startsWith("GET /BY")) {
            //Serial.println("GET HIGH");
            get_http_request = 1;
            digitalWrite(boiler,HIGH);
            boiler_status = 1;
          }
          //보일러 끄게 만들어야함
          else if (currentLine.startsWith("GET /BN")) {
            //Serial.println("GET LOW");
            get_http_request = 1;
            digitalWrite(boiler,LOW);
            boiler_status = 0;
          }
          //가스 끄게 만들어야함
//          else if (currentLine.startsWith("GET /GY")) {
//            //Serial.println("GET HIGH");
//            get_http_request = 1;
//            for(int i=2400;i>1200;i--){
//               digitalWrite(gasPin, HIGH);  
//               delayMicroseconds(i);     
//               digitalWrite(gasPin, LOW);   
//               delayMicroseconds(m_max-i);    
//              }
//          }
          currentLine = "";
        }
      }
      else if (c != '\r') {    // if you got anything else but a carriage return character,
        currentLine += c;      // add it to the end of the currentLine
      }
    }
  }

  digitalWrite(boiler,boiler_status);
  digitalWrite(led_red,led_status);
  
  // close the connection
  JSN270.sendCommand("at+nclose\r");
  Serial.println("client disonnected");
}

void open(){
  for(int i=1400;i<2400;i++){
               digitalWrite(servoPin, HIGH);  
               delayMicroseconds(i);     
               digitalWrite(servoPin, LOW);   
               delayMicroseconds(m_max-i);    
              } 
}

void close(){
  for(int i=2400;i>1400;i--){
               digitalWrite(servoPin, HIGH);  
               delayMicroseconds(i);     
               digitalWrite(servoPin, LOW);   
               delayMicroseconds(m_max-i);    
              } 
}
