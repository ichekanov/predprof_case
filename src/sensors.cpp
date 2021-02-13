#include <Arduino.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>

/*
Датчики: 
  + Датчик температуры
  + Датчик влажности
  + Датчик освещенности
  + Датчик движения (проблема - обновляется довольно редко)
  + Датчик шума (проблема - нет явного верхнего и нижнего пределов)
  + Датчик газов (проблема - не ясны крайние значения, непонятно 
    как интерпретировать значения)
*/

int light_pin = A0; // пин фоторезистора
int DHT_pin = 3; // пин датчика температуры и влажности
int motion_pin = 2; // пин датчика движения
int noise_pin = A1; // пин датчика шума
int smoke_pin = A2; // пин датчика газов

int light = 0; // raw значение с АЦП для фоторезистора
int light_percent = 0; // перевод этого значния в проценты

int noise = 0; // raw значение с АЦП для микрофона
int noise_percent = 0; // перевод этого значения проценты

float temperature = 0.0; // значение температуры
float humidity = 0.0; // значние влажности
DHT dht(DHT_pin, DHT11); // инициализация датчика температуры

bool motion = false; // значение на датчике движения

int smoke = 0; // значение на датчике газов

// работа с фоторезистором
int read_light() {
    light = analogRead(light_pin); // чтение с фоторезистора
    light_percent = map(light, 1023, 0, 0, 100); // преобразование данных
    return light_percent;
}

// работа с микрофоном
int read_noise() {
    noise = analogRead(noise_pin); // чтение с микрофона
    noise_percent = map(noise, 0, 700, 0, 100); // преобразование данных
    return noise_percent;
}

void setup() {
    Serial.begin(9600); // установка связи с компьютером
    pinMode(light_pin, INPUT); // инициализация фоторезистора
    pinMode(noise_pin, INPUT); // инициализация микрофона
    pinMode(smoke_pin, INPUT); // инициализация датчика дыма
    dht.begin(); // инициализация датчика температуры/влажности
    pinMode(13, OUTPUT); // работа со внутренним светодиодом
    digitalWrite(13, LOW);
}

void loop() {
    digitalWrite(13, HIGH);

    Serial.print("Light intensity: "); Serial.print(read_light()); Serial.println("%");
    Serial.print("Noise level: "); Serial.print(read_noise()); Serial.println("%");
    Serial.print("Temperature: "); Serial.print(dht.readTemperature()); Serial.println("°C");
    Serial.print("Humidity: "); Serial.print(dht.readHumidity()); Serial.println("%");
    Serial.print("Smoke level: "); Serial.println(analogRead(smoke_pin));
    if (digitalRead(motion_pin)) {Serial.println("Motion detected!");}
    else {Serial.println("No motion detected.");} 
    Serial.println();
    
    digitalWrite(13, LOW);
    delay(2000);
}
