#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
//ADC_MODE(ADC_TOUT);//ADC_VCC

#include <DHT.h>
#define DHTPIN 5
#define DHTTYPE DHT11

#define zdpin 16
#define SDA 4
#define SCL 14
#define sport 80

// WiFiClient client;
ESP8266WebServer server(sport);
DHT dht(DHTPIN, DHTTYPE);
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, SCL, SDA, U8X8_PIN_NONE);

struct Sdate{
  IPAddress IP;//IPAddress
  uint16_t port;
}sdate;

struct Valdata{
  String val;
  String sel;
  String light;
}valdata;

//String date = "";//用来存放日期
//String otime = "";//用来存放时间
//String weekday = "";//用来存放星期
String payload = "";
String strh = "";//获取数据储存变量
String strl = "";
String time_url = "http://quan.suning.com/getSysTime.do"; //时间接口地址
String str_0 = "0";
String str_1 = "1";

String myhtmlPage =
  String("") +
" <!DOCTYPE html>"+
" <html lang=\"en\">"+
"     <head>"+
"         <meta charset=\"UTF-8\">"+
"         <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"+
"         <meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\">"+
"         <title>ESP8266 Web Server</title>"+
"         <script type=\"text/javascript\">"+
"           setInterval(\"getTime()\",1000);"+
"           function getTime(){"+
"             var today=new Date();"+
"             var h=today.getHours();"+
"             var m = today.getMinutes();"+
    "         var s = today.getSeconds();"+
    "         var ye = today.getFullYear();"+
    "         var mo = today.getMonth()+1;"+
    "         var da = today.getDate();"+
    "         mo = checkTime(mo);"+
    "         da = checkTime(da);"+
    "         m = checkTime(m);"+
    "         s = checkTime(s);"+
"             var xq=new Date().getDay();"+
"             var weekday=new Array(7);"+
"             weekday[0]=\"Sun\";"+
"             weekday[1]=\"Mon\";"+
"             weekday[2]=\"Tue\";"+
"             weekday[3]=\"Wed\";"+
"             weekday[4]=\"Thu\";"+
"             weekday[5]=\"Fri\";"+
"             weekday[6]=\"Sat\";"+
"             var date=ye+\"/\"+mo+\"/\"+da;"+
"             var time=h+\":\"+m+\":\"+s;"+
"             document.getElementById(\"time\").innerHTML=date+\"  \"+time+\"  \"+weekday[xq];"+
"           }"+
"           function checkTime(i) {"+
"             if (i < 10)"+
"               i = \"0\" + i;"+
"               return i;"+
"           }"+
"           function local(val,sel,staut){"+
"               var xmlhttp = new XMLHttpRequest();"+
"               xmlhttp.open(\"GET\",\"/val?val=\"+val+\"&sel=\"+sel+\"&light=\"+staut,true);"+
"               xmlhttp.send();"+
"           }"+
"         </script>"+
"     </head>"+
"     <body>"+
"       <div id=\"time\"></div>"+
"       <div>"+
  "        <p>温度：<span id=\"temperature\">00.00</span>℃</p>"+
  "        <p>湿度：<span id=\"humidity\">00.00</span>％</p>"+
  "        <p>光照：<span id=\"illumination\">00.00</span>％</p>"+
  "        <script type=\"text/javascript\">"+
  "           setInterval(\"getht()\",5000);"+
  "           function getht(){"+
  "             var xhr = new XMLHttpRequest();"+
  "             xhr.open(\"GET\",\"/data\",true);"+
  "             xhr.send();"+
  "             xhr.onreadystatechange = function() {"+
  "               if(xhr.readyState == XMLHttpRequest.DONE && xhr.status == 200) {"+
  "                 var data = JSON.parse(xhr.responseText);"+
  "                 document.getElementById(\"temperature\").innerHTML = data.data[0].temperature;"+
  "                 document.getElementById(\"humidity\").innerHTML = data.data[0].humidity;"+
  "                 document.getElementById(\"illumination\").innerHTML = data.data[0].illumination;"+
  "               }"+
  "             }"+
  "           }"+
  "         </script>"+
    "     </div>"+
"         <div>"+
"             <p>报警器"+
"             <a href=\"#\" onclick=\"local(3,0,'on')\"><input type=\"button\" value=\"OPEN\"></a>"+
"             <a href=\"#\" onclick=\"local(3,0,'off')\"><input type=\"button\" value=\"CLOSE\"></a></p>"+
"         </div>"+
"         <div>"+
"             <p>电机"+
"             <a href=\"#\" onclick=\"local(1,2,'on')\"><input type=\"button\" value=\"Z FAST\"></a>"+
"             <a href=\"#\" onclick=\"local(1,1,'on')\"><input type=\"button\" value=\"Z SLOW\"></a>"+
"             <a href=\"#\" onclick=\"local(1,4,'on')\"><input type=\"button\" value=\"F FAST\"></a>"+
"             <a href=\"#\" onclick=\"local(1,3,'on')\"><input type=\"button\" value=\"F SLOW\"></a>"+
"             <a href=\"#\" onclick=\"local(1,0,'off')\"><input type=\"button\" value=\"CLOSE\"></a></p>"+
"         </div>"+
"         <div>"+
"           <p>继电器1"+
"             <a href=\"#\" onclick=\"local(2,1,'on')\"><input type=\"button\" value=\"OPEN\"></a>"+
"             <a href=\"#\" onclick=\"local(2,1,'off')\"><input type=\"button\" value=\"CLOSE\"></a></p>"+
"         </div>"+
"         <div>"+
"           <p>继电器2"+
"             <a href=\"#\" onclick=\"local(2,2,'on')\"><input type=\"button\" value=\"OPEN\"></a>"+
"             <a href=\"#\" onclick=\"local(2,2,'off')\"><input type=\"button\" value=\"CLOSE\"></a></p>"+
"         </div>"+
"         <div>"+
"             <p>继电器3"+
"             <a href=\"#\" onclick=\"local(2,3,'on')\"><input type=\"button\" value=\"OPEN\"></a>"+
"             <a href=\"#\" onclick=\"local(2,3,'off')\"><input type=\"button\" value=\"CLOSE\"></a></p>"+
"         </div>"+
"         <div>"+
"             <p>继电器4"+
"             <a href=\"#\" onclick=\"local(2,4,'on')\"><input type=\"button\" value=\"OPEN\"></a>"+
"             <a href=\"#\" onclick=\"local(2,4,'off')\"><input type=\"button\" value=\"CLOSE\"></a></p>"+
"         </div>"+
"         <div>"+
"             <p>继电器5"+
"             <a href=\"#\" onclick=\"local(2,5,'on')\"><input type=\"button\" value=\"OPEN\"></a>"+
"             <a href=\"#\" onclick=\"local(2,5,'off')\"><input type=\"button\" value=\"CLOSE\"></a></p>"+
"         </div>"+
"     </body>"+
" </html>";

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
/***********把数据发送到网页上***********/
void data(){ 
	float h = dht.readHumidity();
	float t = dht.readTemperature();
  float i = 100-analogRead(A0)/10.24;
  String json = "{\"data\":[{\"temperature\":\""+String(t)+
  "\",\"humidity\":\""+String(h)+"\",\"illumination\":\""+String(i)+"\"}]}";
  server.send(200,"text/plain",json);
}

void setup() {
  Serial.begin(115200);
  delay(100);
  pinMode(LED_BUILTIN, OUTPUT);
  dht.begin();
  u8g2.begin();

  smartC();
  mdns_init();

  server.on("/", handleRoot);//显示主页
  server.on("/data", HTTP_GET, data);
  server.on("/val", HTTP_GET, val);
  
//-------------响应-----------------
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  MDNS.update();
  server.handleClient();
  get_time();
}

/***************显示网页*********************/
void handleRoot() {
  server.send(200, "text/html", myhtmlPage);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  server.send(404, "text/plain", message);
}

void get_time(){
  HTTPClient http;
  http.begin(time_url);
  int httpCode = http.GET();
  if (httpCode > 0) {
    payload = http.getString();
    strh= payload.substring(13,23);//日期
    strl= payload.substring(24,32);//时间
    //delay(1000);
    oled_display(strh, strl);
    //delay(1000);
    //Serial.println(str);
  }
  http.end();
}
/************OLED显示函数**************/
void oled_display(String& s1, String& s2) {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  u8g2.setFontDirection(0);
  u8g2.clearBuffer();         
  //u8g2.setFont(u8g2_font_crox5t_tr);  
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.setCursor(0, 15);
  u8g2.print(s1);
//  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.setCursor(60,15);//(0, 46);
  u8g2.print(WiFi.localIP());
//  u8g2.setFont(u8g2_font_ncenB08_tr);//u8g2_font_wqy12_t_gb2312
  u8g2.setCursor(0, 31);
  u8g2.print("H:");//("湿度:");
  u8g2.setCursor(16, 31);
  u8g2.print(h);
  u8g2.setCursor(46, 31);
  u8g2.print("%");
  u8g2.setCursor(64, 31);
  u8g2.print("T :");//("温度:");
  u8g2.setCursor(80, 31);
  u8g2.print(t);
  u8g2.setCursor(110, 31);
  u8g2.print("⁰C");
  u8g2.setFont(u8g2_font_crox5t_tr);  
  u8g2.setCursor(0, 64);
  u8g2.print(s2);   
  u8g2.sendBuffer();
  delay(200);  
}

void val() {
//  String data = server.arg("val");
  valdata.val = server.arg("val");
//  Serial.println("val="+valdata.val);
  valdata.sel = server.arg("sel");
//  Serial.println("sel="+valdata.sel);
  valdata.light = server.arg("light");
//  Serial.println("light="+valdata.light);
  switch(valdata.val[0]) {
    case '1':
      //电机处理函数
      Motor();
      break;
    case '2':
      // 继电器处理函数
      controller();
      break;
    case '3':
      // 报警器处理函数
      buzzer();
      break;
  }
}

void Motor(){
  String s = "esp_sensor.local";
  mdns_query(s);
  
  Serial.println("*********Motor**********");
//  String data = server.arg("sel");
//  String sp = server.arg("sel");//data[0];
  if (valdata.light == "on") {
    String s2 = str_1+str_0+valdata.sel;
    Serial.println(s2);
    tcp_client(sdate.IP, sdate.port, s2);
  }else if (valdata.light == "off") {
    String s2 = str_1+str_1+valdata.sel;
    Serial.println(s2);
    tcp_client(sdate.IP, sdate.port, s2);
  }
}

void controller(){
  String s = "esp_switch.local";
  mdns_query(s);
  if (valdata.light == "on") {
    String s2 = valdata.sel+str_0;
    tcp_client(sdate.IP, sdate.port, s2);
  }else if (valdata.light == "off") {
    String s2 = valdata.sel+str_1;
    tcp_client(sdate.IP, sdate.port, s2);
  }
}

void buzzer(){
  String s = "esp_sensor.local";
  mdns_query(s);
  
  Serial.println("*********buzzer**********");
  String key = "";
  if (valdata.light == "on") {
    key = str_0+str_0;
    Serial.println(key);
    tcp_client(sdate.IP, sdate.port, key);
  }else if (valdata.light == "off") {
    key = str_0+str_1;
    Serial.println(key);
    tcp_client(sdate.IP, sdate.port, key);
  }
}

/************开启mDNS***********/
void mdns_init(){
  // char hostString[16] = {0};
  // sprintf(hostString, "ESP_%06X", ESP.getChipId());
  // Serial.print("Hostname: ");
  // Serial.println(hostString);
  WiFi.hostname("esp_server");  // 修改主机名
  if (!MDNS.begin("esp_server")) {
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
        
        sdate.IP = MDNS.IP(i);//.toString();
        sdate.port = MDNS.port(i);
        Serial.println("************IP*****************");
        Serial.println(sdate.IP);
        Serial.println(String(sdate.port));
        Serial.println("*******************");
        // return sdate
      }
    }
  }
  Serial.println();

  // Serial.println("loop() next");
}
/**********TCP Client***********/
void tcp_client(IPAddress serverIP, uint16_t serverPort, String& str)//IPAddress
{
  WiFiClient client;
  Serial.println("尝试访问服务器");
//  Serial.println(serverIP);
//  Serial.println(serverPort);
  client.print(str);
  Serial.println(str);
  if (client.connect(serverIP, serverPort)){ //尝试访问目标地址
    Serial.println("访问成功");
    client.print(str);                    //向服务器发送数据
    while (client.connected() || client.available()){ //如果已连接或有收到的未读取的数据    
      if (client.available()) {//如果有数据可读取      
        String line = client.readStringUntil('\n'); //读取数据到换行符
        Serial.print("读取到数据：");
        Serial.println(line);
        if (line == str){
        	break;
        }
        // client.print(String("Receive:") + line);
        // break;
      }
    }
    Serial.println("关闭当前连接");
    client.stop(); //关闭客户端
  }else {
    Serial.println("访问失败");
    client.stop(); //关闭客户端
  }
  delay(5000);
}
