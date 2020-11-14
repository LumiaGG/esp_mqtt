#include <ESP8266WiFi.h>
#include <PubSubClient.h>

void serial_mqtt_sendmsg();
void setup_wifi();
void reconnect();
void esp_to_stm_c_data(byte *payload, unsigned int length);

const char *ssid = "ROBOT";
const char *password = "iopjklbnm";
const char *mqtt_server = "192.168.123.100"; // 使用HIVEMQ 的信息中转服务
const char *client_id = "Esp_8266_MQTT_YJH";         // 标识当前设备的客户端编号
const char *MQTT_SUB_TOPIC = "topic_s_YJH/#";      // 订阅信息主题
const char *MQTT_PUB_TOPIC = "topic_p_YJH/";       // 发布主题

WiFiClient espClient;           // 定义wifiClient实例
PubSubClient client(espClient); // 定义PubSubClient的实例
long lastMsg = 0;               // 记录上一次发送信息的时长                                                          //存放json数据

char c_data_list[20];


void setup_wifi()
{

  delay(10);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length)
{
  // Serial.print("Message arrived [");
  // Serial.print(topic); // 打印主题信息
  // Serial.print("] ");

  esp_to_stm_c_data(payload,length);

}

void reconnect()
{
  digitalWrite(LED_BUILTIN, HIGH);
  while (!client.connected())
  {
    Serial.print("connection...");
    // Attempt to connect
    if (client.connect(client_id))
    {
      Serial.println("connected");
      // 连接成功时订阅主题
      client.subscribe(MQTT_SUB_TOPIC);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(100);
    }
  }
  
  digitalWrite(LED_BUILTIN, LOW);
}

void esp_to_stm_c_data(byte *payload, unsigned int length)
{
  for (int i = 0; i < length; i++)
    c_data_list[i] = (char) payload[i];


  for (int i = 0; i < 20; i++)
    Serial.write(c_data_list[i]);
}

void serial_mqtt_sendmsg(){
   
    //-------------------Begin----------------------//
   //功能： 通过串口给MQTT服务器发送数据
    if(Serial.available()>0){
      delay(40);
      String  comdata = Serial.readString();
 
      Serial.println(comdata);
      while(Serial.read()>= 0){}
      
       char charBuf[100];
       comdata.toCharArray(charBuf, 100) ;  
       client.publish(MQTT_PUB_TOPIC,charBuf);

    }
    //-------------------End----------------------//
   
  }

void setup()
{
  Serial.begin(115200);
  setup_wifi();                        //执行Wifi初始化，下文有具体描述
  client.setServer(mqtt_server, 1883); //设定MQTT服务器与使用的端口，1883是默认的MQTT端口
  client.setCallback(callback);        //设定回调方式，当ESP8266收到订阅消息时会调用此方法
  client.setKeepAlive(1);    //设置心跳
  client.setSocketTimeout(1);  //设置套接字超时
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
  serial_mqtt_sendmsg();
  
}
