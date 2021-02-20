#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char auth[] = "YOUR BLYNK TOKEN"; // Blynk Auth Token
char ssid[] = "YOUR NETWORK NAME"; // Network name
char pass[] = "YOUR NETWORK PASSWORD"; // Network password

String parsedData[7]; // массив для передаваемых данных

// данные поступают в виде x,y,z,a,b,c (шесть переменных, разделенных запятой)
// температура, влажность, CO2, движение, освещенность, шум
void readFromArduino() { // функция парсинга и передачи данных на сервер
  String s = Serial.readString(); // читаем строку из Serial
  int l = s.length() - 2; // длина строки минус "\n"
  int n = 0; // счётчик индекса массива
  for (int i = 0; i < l; i++) {
    if (s[i] == ',') {
      n++; // если обнаружен разделитель - переходим в следующую ячейку
    }
    else {
      parsedData[n] += s[i]; // иначе продолжаем писать в нынешнюю
    }
  }
  parsedData[6] = (String)(millis()/1000); // в конце записываем время от запуска (а почему бы и нет)
  for (int i = 0; i < 7; i++) {
    Blynk.virtualWrite(V0 + i, parsedData[i]); // передаем каждое значение по отдельности
    parsedData[i] = ""; // опустошаем ячейку массива
  }
}

void setup()
{
  Serial.begin(9600); // инициализация Serial
  Serial.setTimeout(500); // таймаут приёма данных из Serial
  Blynk.begin(auth, ssid, pass); // инициализация Blynk
}

void loop()
{
  Blynk.run(); // необходимо для работы Blynk
  if (Serial.available()) readFromArduino(); // если в Serial есть информация -
                                             // читаем и передаем ее
}
