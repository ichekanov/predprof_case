#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp8266.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

//*************** Настройка вывода отладочных сообщений ***************//
//#define DEBUG  // закомментрировать эту строку чтобы отключить вывод отладочной информации
#ifdef DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#else
#define DEBUG_PRINT(x)
#endif

//************************* Задание постоянных ************************//
#define SERVER_NAME "srv1.clusterfly.ru"
#define SERVER_PORT  9124
#define SERVER_USERNAME "--- your mqtt username ---"
#define SERVER_PASSWORD "--- your mqtt password ---"
#define NETWORK_SSID "--- your wifi username ---"
#define NETWORK_PASS "--- your wifi password ---"
const char topic_debug[] = SERVER_USERNAME "/debug";
const char topic_data[] = SERVER_USERNAME "/data";
char token[] = "--- your blynk token ---";
WiFiClient esp_client;
PubSubClient client(esp_client);
WiFiClientSecure httpsClient;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
unsigned long last_millis = 0;
unsigned long last_time_update = 0;
const char host[] = "docs.google.com";
const int httpsPort = 443; 
String link = "/forms/d/e/--- your google form token ---/formResponse?usp=pp_url&";
const char fingerprint[] = "79 d5 16 07 e2 38 0e 84 51 4f f2 7b 52 98 9b 9d 01 fc b1 e8";


bool setupWIFI() {
  // если устройство уже подключено к сети - ничего не делаем
  if (WiFi.status() == WL_CONNECTED) return true; 
  // иначе - пытаемся подключиться в течение трех секунд 
  // (на самом деле сделано больше для красоты, ведь 
  // esp автоматически переподключается к сети):
  DEBUG_PRINT("--- WiFi ---\nConnecting to ");
  DEBUG_PRINT(NETWORK_SSID "\n");
  WiFi.begin(NETWORK_SSID, NETWORK_PASS);
  int r = 0;
  while ((WiFi.status() != WL_CONNECTED) && (r<6)) {
    delay(500);
    DEBUG_PRINT(".");
    r++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    DEBUG_PRINT("\nWiFi connected!     IP address: ");
    DEBUG_PRINT(WiFi.localIP());
    DEBUG_PRINT("\n");
    return true;
  }
  else {
    Serial.print("No network.");
    delay(500);
    return false;
  }  
}


void setupMQTT() {
  while (!client.connected()) {
    DEBUG_PRINT("\n--- MQTT ---\nAttempting MQTT connection...\n");
    // создаем уникальный идентификатор устройства
    String clientId = "ESP8266";
    clientId += String(random(UINT16_MAX), HEX);
    DEBUG_PRINT(clientId);
    // пытаемся подключиться
    if (client.connect(clientId.c_str(), SERVER_USERNAME, SERVER_PASSWORD)) {
      DEBUG_PRINT("\nConnected!\n");
      client.publish(topic_debug, "Connected!");
      //client.subscribe("topic"); заготовка для подписки на топик
    } 
    else {
      // если не удалось подключиться - выводим код ошибки
      DEBUG_PRINT("\nFailed, error=");
      DEBUG_PRINT(client.state());
      DEBUG_PRINT(" Next try in 5 seconds\n");
      // ждем 5 секунд для следующей поппытки
      delay(5000);
    }
  }
}


void setupHTTPS() {
  DEBUG_PRINT("\n--- HTTPS ---\nConnecting to host: ");
  DEBUG_PRINT(host);
  DEBUG_PRINT("\nThe fingerprint is: ");
  DEBUG_PRINT(fingerprint);
  DEBUG_PRINT("\n");
  httpsClient.setFingerprint(fingerprint); // хэш сертификата сайта host
  httpsClient.setTimeout(5000);
}


void sendMQTT(String s);
void sendBlynk(String parsedData[]);
void sendHTTPS(String parsedData[]);


// данные поступают в виде x,y,z,a,b,c (шесть переменных, разделенных запятой)
// температура, влажность, CO2, движение, освещенность, шум
void readFromArduino(bool flag) { // функция чтения и передачи данных
  Serial.print("$get");
  if (flag) { // если есть подключение к сети, то начинаем обрабатывать строку
    String s = Serial.readString(); // читаем строку из Serial
    String parsedData[6]; // массив для передаваемых данных
    if (s.length() > 5) { // если есть ответ от Arduino
      int l = s.length(); // длина строки
      int n = 0; // счётчик индекса массива
      for (int i = 0; i < l; i++) {
        if (s[i] == ',') {
          n++; // если обнаружен разделитель - переходим в следующую ячейку
        }
        else {
          parsedData[n] += s[i]; // иначе продолжаем писать в нынешнюю
        }
      }
    }
    // если Arduino не отвечает (не подключено) - отправляем тестовые сообщения
    else {
      DEBUG_PRINT("\nNo data from Arduino :c\n");
      s = "testMQTT";
      for (int i = 0; i < 6; i++) {
        parsedData[i] = "test"+String(i);
      }
    }
    sendMQTT(s);            // передаем сообщение от Arduino в MQTT
    sendBlynk(parsedData);  // передаем сообщение от Arduino в Blynk
    sendHTTPS(parsedData);  // передаем сообщение от Arduino в Google Sheets
    DEBUG_PRINT("All data has been sent to services!\n\n");
  }
  else { // если подключения к сети нет - просто очищаем буфер
    Serial.readString();
    DEBUG_PRINT("No connection, but data has been stored to SD.\n\n");
  }
  // такая логика реализована для того, чтобы информация точно записалась на карту
  // памяти ардуино, вне зависимости от наличия подключения к сети
}


void sendMillis() {
  // отладочная функиця, отправляющая по MQTT время от начала работы контроллера
  int t = millis()/1000;
  char time[10];
  String s = String(t);
  s.toCharArray(time, 10);
  client.publish(topic_debug, time);
}


void sendMQTT(String s) {
  char info[35];
  s.toCharArray(info, 35);  // конвертируем строку в символьный массив
  client.publish(topic_data, info);  // публикуем его в основной топик
}


void sendBlynk(String data[]) {
  for (int i = 0; i < 6; i++) {
    Blynk.virtualWrite(V0 + i, data[i]);  // передаем информацию в Blynk
  }
  Blynk.virtualWrite(V6, millis()/1000);
}


void sendTime() {
  timeClient.update();
  Serial.print("$time");
  while (!Serial.available()) {}
  if (Serial.readString() == "$ready"){
    Serial.print(timeClient.getEpochTime() + 60*60*3);
  }
  delay(1000);
}


void sendHTTPS(String data[]) {
  int r=0; //счетчик попыток подключения к сети
  while((!httpsClient.connect(host, httpsPort)) && (r < 50)){
      delay(100);
      DEBUG_PRINT(".");
      r++;
  }
  if(r==50) {
    DEBUG_PRINT("Connection failed");
  }
  else {
    DEBUG_PRINT("Connected to web!");
    // генерируем ссылку и обращаемся по ней
    String address = link + "entry.1689981724=" + String(millis()) + "&entry.78358813=" + data[0] + 
    "&entry.1297584051=" + data[1] + "&entry.645439583=" + data[2] + "&entry.161083686=" + data[3] + 
    "&entry.1743296173=" + data[4] + "&entry.39404176=" + data[5] + "&submit=Submit";
    httpsClient.print(String("GET ") + address + " HTTP/1.1\r\n" + "Host: " + 
                      host + "\r\n" + "Connection: close\r\n\r\n");
    DEBUG_PRINT("\nRequest sent!\n");
  }
}


// функция для получения информации из mqtt, сейчас не используется
/*
void callback(char* topic, byte* payload, int length){
  DEBUG_PRINT(topic);
  DEBUG_PRINT("\n");
  for (int i = 0; i < length; i++) {
    DEBUG_PRINT((char)payload[i]);
  }
  DEBUG_PRINT("\n");

}
*/


void setup() {
  Serial.begin(9600); // инициализация Serial
  Serial.setTimeout(500); // таймаут приёма данных из Serial
  delay(5000); // поуза для того чтобы Arduino полностью запустился
  setupWIFI(); // настройка подключения к WiFi
  
  client.setServer(SERVER_NAME, SERVER_PORT); // задание параметров подключения к MQTT
  Blynk.begin(token, NETWORK_SSID, NETWORK_PASS); // инициализация Blynk
  timeClient.begin(); // инициализация NTP клиента
  setupHTTPS(); // задание параметров подключения к HTTPS
  sendTime(); // передача RTC
  delay(1000);
  Serial.print("done setup.");
  // client.setCallback(callback); // настройка callback'а, сейчас не используется
}


void loop() {
  if (millis() - last_millis > 5000) { // раз в пять секунд запускаем процесс обработки информации
    last_millis = millis();
    bool wifiConnected = setupWIFI(); // проверяем подключение к сети
    if (wifiConnected) {    
      setupMQTT(); // проверяем подключение к MQTT
      sendMillis(); // передаем в топик debug кол-во секунд от запуска
      client.loop(); // для поддержания работы MQTT
      Blynk.run(); // для поддержания работы Blynk
    }
    readFromArduino(wifiConnected); // запрашиваем данные от Arduino, читаем, обрабатываем и передаем их
  }
  if (millis() - last_time_update > 300000) { // раз в пять минут обновляем время на Arduino
    last_time_update = millis();
    sendTime(); // передача RTC
  }
}
