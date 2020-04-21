#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
// #include <ESP8266WebServer.h>
// #include <Arduino.h>
// #include <U8g2lib.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
//ADC_MODE(ADC_TOUT);//ADC_VCC

#define sport 8202
#define zdpin 16
#define hcsr 5
#define IN1 4
#define IN2 14
#define IN3 12
#define IN4 13
#define IN5 15
#define IN6 3
//#define IN7 1


// WiFiClient client;
WiFiServer server(sport); //声明服务器对象

struct Sdate{
  IPAddress IP;
  uint16_t port;
}sdate;



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
  pinMode(zdpin, INPUT);
  pinMode(hcsr, INPUT);

  pin_init();


  smartC();
  mdns_init();
  server.begin(sport); //服务器启动监听端口号22333

}

void loop() {
  MDNS.update();
  tcp_server();
  HC_SR501();
  delay(500);
  // server.handleClient();
}


/************开启mDNS***********/
void mdns_init(){
  // char hostString[16] = {0};
  // sprintf(hostString, "ESP_%06X", ESP.getChipId());
  // Serial.print("Hostname: ");
  // Serial.println(hostString);
  WiFi.hostname("esp_switch");  // 修改主机名
  if (!MDNS.begin("esp_switch")) {
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
              client.print(line);
              Serial.println(line[0]);
              Serial.println(line[1]);
              switch(line[0]){
                case '1':
                  if (line[1] == '0'){
                    digitalWrite(IN1,LOW);
                    Serial.println("on ok");
                  }else if(line[1] == '1'){
                    digitalWrite(IN1,HIGH);
                    Serial.println("off ok");
                  }
                  break;
                case '2':
                  if (line[1] == '0'){
                    digitalWrite(IN2,LOW);
                  }else if(line[1] == '1'){
                    digitalWrite(IN2,HIGH);
                  }
                  break;
                case '3':
                  if (line[1] == '0'){
                    digitalWrite(IN3,LOW);
                  }else if(line[1] == '1'){
                    digitalWrite(IN3,HIGH);
                  }
                  break;
                case '4':
                  if (line[1] == '0'){
                    digitalWrite(IN4,LOW);
                  }else if(line[1] == '1'){
                    digitalWrite(IN4,HIGH);
                  }
                  break;
                case '5':
                  if (line[1] == '0'){
                    digitalWrite(IN5,LOW);
                  }else if(line[1] == '1'){
                    digitalWrite(IN5,HIGH);
                  }
                  break;
                case '6':
                  if (line[1] == '0'){
                    digitalWrite(IN6,LOW);
                  }else if(line[1] == '1'){
                    digitalWrite(IN6,HIGH);
                  }
                  break;
              //                client.print(line);
              }
              break; //跳出循环
            }
        }
        client.stop(); //结束当前连接:
        Serial.println("[Client disconnected]");
    }
}
void HC_SR501(){
  float i = 100-analogRead(A0)/10.24;
  Serial.println(String(i));
  delay(500);
  if(i <= 30){
    if(digitalRead(hcsr) == HIGH){
      digitalWrite(IN6, LOW);
    }else{
      digitalWrite(IN6, HIGH);
    }
  }else{
      digitalWrite(IN6, HIGH);
  }
  // 向网页发信息
  // String str = "esp_server.local";
  // mdns_query(str);
}

void pin_init(){
	pinMode(IN1, OUTPUT);
	pinMode(IN2, OUTPUT);
	pinMode(IN3, OUTPUT);
	pinMode(IN4, OUTPUT);
	pinMode(IN5, OUTPUT);
	pinMode(IN6, OUTPUT);
//	pinMode(IN7, OUTPUT);
//	
	 digitalWrite(IN1, HIGH);
	 digitalWrite(IN2, HIGH);
	 digitalWrite(IN3, HIGH);
	 digitalWrite(IN4, HIGH);
	 digitalWrite(IN5, HIGH);
	 digitalWrite(IN6, HIGH);
	// digitalWrite(IN7, HIGH);
	
}
