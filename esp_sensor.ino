#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
// #include <ESP8266WebServer.h>
// #include <Arduino.h>
// #include <U8g2lib.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
//ADC_MODE(ADC_TOUT);//ADC_VCC

#define zdpin 16
#define sport 8201
#define pin 5
#define motor1 12
#define motor2 14
// WiFiClient client;
WiFiServer server(sport); //声明服务器对象

struct Sdate{
  IPAddress IP;
  uint16_t port;
}sdate;


String k = "def";

/************智能配网**************/
void smartConfig() {
  WiFi.mode(WIFI_STA);
  Serial.println("\r\nWait for Smartconfig...");
  WiFi.beginSmartConfig();//等待手机端发出的用户名与密码
  while (1){
    Serial.print(".");
    digitalWrite(LED_BUILTIN, HIGH);  
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);    
    delay(1500);
    if (WiFi.smartConfigDone()){//退出等待
      Serial.println("SmartConfig Success");
      Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
      Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
      break;
    }
  }
}

void key_scan() {
  for(int i = 0; i <= 5; i++){
    delay(500);
    if(digitalRead(zdpin) == LOW) {
      //digitalWrite(LED_BUILTIN, LOW);
      delay(1000);
      if(digitalRead(zdpin) == LOW) {
        smartConfig();
      }
    }
  }
}

void smartC(){
  int count=0;
  bool WIFI_Status = true;
  key_scan();
  Serial.println("\r\n正在连接");
  while(WiFi.status()!=WL_CONNECTED){
    if(WIFI_Status){
      Serial.print(".");
      digitalWrite(LED_BUILTIN, HIGH);  
      delay(500);
      digitalWrite(LED_BUILTIN, LOW);    
      delay(500);
      count++;
      if(count>=5){
        WIFI_Status = false;
        Serial.println("WiFi连接失败，请用手机进行配网"); 
      }
    }else{
      smartConfig();
    }
  }  
  Serial.println("连接成功");  
  Serial.print("IP:");
  Serial.println(WiFi.localIP());
  
  digitalWrite(LED_BUILTIN, HIGH);
}

void setup() {
  Serial.begin(115200);
  delay(100);
  pinMode(LED_BUILTIN, OUTPUT);
 	pinMode(pin,OUTPUT);
	digitalWrite(pin, HIGH);
  pinMode(motor1,OUTPUT);
  pinMode(motor2,OUTPUT);
//  dht.begin();
//  u8g2.begin();

  smartC();
  mdns_init();
  server.begin(sport); //服务器启动监听端口号22333

}

void loop() {
  rain();
  delay(2000);
  MDNS.update();
  tcp_server();
  // server.handleClient();
}


/************开启mDNS***********/
void mdns_init(){
  // char hostString[16] = {0};
  // sprintf(hostString, "ESP_%06X", ESP.getChipId());
  // Serial.print("Hostname: ");
  // Serial.println(hostString);
  WiFi.hostname("esp_sensor");  // 修改主机名
  if (!MDNS.begin("esp_sensor")) {
    Serial.println("Error setting up MDNS responder!");
  }
  Serial.println("mDNS responder started");
  MDNS.addService("esp", "tcp", sport); // Announce esp tcp service on port 8080
}
/***********mDNS查找***********/
void mdns_query(String& str){
  // Serial.println("Sending mDNS query");
  int n = MDNS.queryService("esp", "tcp"); // Send out query for esp tcp services
  // Serial.println("mDNS query done");
  if (n == 0) {
    Serial.println("no services found");
  } else {
    Serial.print(n);
    Serial.println(" service(s) found");
    for (int i = 0; i < n; ++i) {
      // Print details for each service found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(MDNS.hostname(i));
      Serial.print(" (");
      Serial.print(MDNS.IP(i));
      Serial.print(":");
      Serial.print(MDNS.port(i));
      Serial.println(")");
      if(MDNS.hostname(i) == str){
        
        sdate.IP = MDNS.IP(i);
        sdate.port = MDNS.port(i);
        // return sdate
      }
    }
  }
  Serial.println();

  // Serial.println("loop() next");
}
/**********TCP Server***********/
void tcp_server(){
  Serial.println("Client ......");
    WiFiClient client = server.available(); //尝试建立客户对象
    if (client) //如果当前客户可用
    {
        Serial.println("[Client connected]");
        while (client.connected()) //如果客户端处于连接状态
        {
            if (client.available()) //如果有可读数据
            {
              String line = client.readStringUntil('\n'); //读取数据到换行符
              Serial.println(line);
              if(line[0] == '0'){
              	//蜂鸣器开关
              	if (line[1] == '0'){
//            			digitalWrite(pin, LOW);
            			k = "on";
            		}	else if(line[1] == '1'){
//            			digitalWrite(pin, HIGH);
            			k = "off";
            		}
              }else if(line[0] == '1'){
              	//电机控制
              	if (line[1] == '0'){
            			if (line[2] == '2'){
	            			M_zf();
	            		}	else if(line[2] == '1'){
	            			M_zs();
	            		}else if(line[2] == '4'){
	            			M_ff();
	            		}else if(line[2] == '3'){
	            			M_fs();
	            		}
            		}	else if(line[1] == '1'){
            			M_off();
            		}
              }
              break; //跳出循环
            }
        }
        client.stop(); //结束当前连接:
        Serial.println("[Client disconnected]");
    }
}
void rain(){
  float i = 100-analogRead(A0)/10.24;
  Serial.println(String(i));
  delay(500);
  if(i >= 10){
  	if(k == "off"){
      digitalWrite(pin, HIGH);
//      k = "def";
  	}else{
  		digitalWrite(pin, LOW);
  	}
  }else{
    if(k == "on"){
  		digitalWrite(pin, LOW);
//      delay(1000);
//      k = "def";
  	}else{
  		digitalWrite(pin, HIGH);
      k = "def";
  	}
  }
  // 向网页发信息
  // String str = "esp_server.local";
  // mdns_query(str);
}

void M_zf(){
	digitalWrite(motor1,LOW);
	digitalWrite(motor2,LOW);
	delay(500);
  digitalWrite(motor1,HIGH);
  digitalWrite(motor2,LOW);
}

void M_zs(){
	digitalWrite(motor1,LOW);
	digitalWrite(motor2,LOW);
	delay(500);
	analogWrite(motor1,512);
  digitalWrite(motor2,LOW);
  
}

void M_ff(){
	digitalWrite(motor1,LOW);
	digitalWrite(motor2,LOW);
	delay(500);
	digitalWrite(motor1,LOW);
  digitalWrite(motor2,HIGH);
  
}

void M_fs(){
	digitalWrite(motor1,LOW);
	digitalWrite(motor2,LOW);
	delay(500);
  digitalWrite(motor1,LOW);
  analogWrite(motor2,512);

}

void M_off(){
  digitalWrite(motor1,LOW);
  digitalWrite(motor2,LOW);
  delay(500);
  digitalWrite(motor1,HIGH);
  digitalWrite(motor2,HIGH);
}
