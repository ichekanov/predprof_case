#include <Arduino.h>          // основная библиотека Arduino
#include <DHT.h>              // библиотека для работы с DHT11
#include <Adafruit_Sensor.h>  // библиотека для работы с MQ-135
#include <SoftwareSerial.h>   // библиотека для передачи данных в esp
#include <MQ135.h>            // библиотека для работы с MQ-135

/*
Датчики: 
  + Датчик температуры
  + Датчик влажности
  + Датчик освещенности
  + Датчик движения
  + Датчик шума
  + Датчик газов
*/

#define LIGHT_PIN A0 // пин фоторезистора
#define DHT_PIN 3    // пин датчика температуры и влажности
#define MOTION_PIN 2 // пин датчика движения
#define NOISE_PIN A1 // пин датчика шума
#define CO2_PIN A2   // пин датчика газов
SoftwareSerial espSerial(10, 11); // инициализация второго Serial порта для esp (recieve, transmit)
DHT dht(DHT_PIN, DHT11);          // инициализация датчика температуры
MQ135 mq(CO2_PIN);

float rawData0[6]; // три массива с сырыми данными, обновляющимися по очереди
float rawData1[6];
float rawData2[6];
float data[6];     // массив с отфильтрованными данными
byte n = 0;        // счетчик для чтения в массив с сырыми данными
unsigned long last_read = 0;

float readData(int sensor);     // функция универсального чтения для всех датчиков
void sendToSerial(float data[]);// передача данных на компьютер
void sendToEsp(float data[]);   // передача данных на esp
void medianFilter(float *refRawData0, float *refRawData1, float *refRawData2, float* refData); // медианный фильтр для защиты от некорректных данных
void readToRaw(float *refRawData0, float *refRawData1, float *refRawData2, byte &iter); // чтение в массив с сырыми данными


void setup() {
    Serial.begin(9600);         // установка связи с компьютером
    espSerial.begin(9600);
    espSerial.setTimeout(50);   // таймаут для получения данных от esp
    pinMode(LIGHT_PIN, INPUT);  // инициализация фоторезистора
    pinMode(NOISE_PIN, INPUT);  // инициализация микрофона
    pinMode(CO2_PIN, INPUT);    // инициализация датчика CO2
    dht.begin();                // инициализация датчика температуры/влажности
    pinMode(13, OUTPUT);        // работа со внутренним светодиодом
    digitalWrite(13, LOW);
    for (int i = 0; i < 3; i++) {
        readToRaw(rawData0, rawData1, rawData2, n); // трижды читаем данные с датчиков, чтобы "прогреть" их
                                                    // и для корректной работы медианного фильтра
        n++;
        delay(1000);
    }
    for (int i = 0; i < 5; i++) { // сигнализируем об окончании инициализации
        digitalWrite(13, HIGH);
        delay(75);
        digitalWrite(13, LOW);
        delay(75);
    }
}


void loop() {
    if (last_read - millis() > 1000) {
        last_read = millis();
        readToRaw(rawData0, rawData1, rawData2, n);
    }
    if (espSerial.available()) {
        String message = espSerial.readString();
        Serial.print("Recieved from ESP: ");
        Serial.println(message);
        if (message == "$get") {
            medianFilter(rawData0, rawData1, rawData2, data); // прогоняем прочитанные данные через фильтр
            sendToEsp(data); // отправляем их в esp
            sendToSerial(data);  // и на ПК
        }
    }
}


float readData(int sensor) {
    if (sensor == 0) {
        return dht.readTemperature(); // чтение с DHT
    }
    if (sensor == 1) {
        return dht.readHumidity(); // чтение с DHT
    }
    if (sensor == 2) {
        return mq.getPPM(); // чтение с MQ-135
    }
    if (sensor == 3) {
        if (digitalRead(MOTION_PIN)) return 100; // чтение с датчика движения
        else return 0; 
    }
    if (sensor == 4) {
        int light = analogRead(LIGHT_PIN); // чтение с фоторезистора
        int light_percent = map(light, 1023, 0, 0, 100); // преобразование данных
        return light_percent;
    }
    if (sensor == 5) {
        int noise = analogRead(NOISE_PIN); // чтение с микрофона
        int noise_percent = map(noise, 0, 1023, 0, 100); // преобразование данных
        return noise_percent;
    }
    return -1;
}


void sendToSerial(float data[]) {
    digitalWrite(13, HIGH);
    Serial.print("Temperature: ");     Serial.print(data[0]); Serial.println("°C");
    Serial.print("Humidity: ");        Serial.print(data[1]); Serial.println("%");
    Serial.print("CO2 level: ");       Serial.print(data[2]); Serial.println("%");
    if (data[3]) {Serial.println("Motion detected!");}
    else {Serial.println("No motion detected.");}
    Serial.print("Light intensity: "); Serial.print(data[4]); Serial.println("%");
    Serial.print("Noise level: ");     Serial.print(data[5]); Serial.println("%");
    Serial.println();
    digitalWrite(13, LOW);
}


void sendToEsp(float data[]) {
    // данные отправляются в виде x,y,z,a,b,c (шесть переменных, разделенных запятой)
    // температура, влажность, CO2, движение, освещенность, шум
    for (int k = 0; k < 5; k++) {
        espSerial.print(data[k]);
        espSerial.print(',');
    }
    espSerial.print(data[5]);
}


void medianFilter(float *refRawData0, float *refRawData1, float *refRawData2, float* refData) {
    // для каждого из шести элементов массива rawData нужно выбрать медианный и записать его в data
    for (int k = 0; k < 6; k++) {
        float m = refRawData0[k] + refRawData1[k] + refRawData2[k];
        m -= min(refRawData0[k], min(refRawData1[k],  refRawData2[k]));
        m -= max(refRawData0[k], max(refRawData1[k],  refRawData2[k]));
        // теперь в переменной m лежит медианное значение
        refData[k] = m;
    }
}

void readToRaw(float *refRawData0, float *refRawData1, float *refRawData2, byte &iter) {
    for (int k = 0; k < 6; k++) { // к сожалению, на данном этапе не удалось реализовать 
                                  // работу с двумерным массивом
        // идея проста: данные пишутся "по кругу", то есть при каждом вызове функции 
        // запись идет в разный массив, но ячейки каждого массива перезаписываются каждую третью итерацию
        if (iter == 0)
            refRawData0[k] = readData(k);
        if (iter == 1)
            refRawData1[k] = readData(k);
        if (iter == 2)
            refRawData2[k] = readData(k);
        delay(10);
    }
    iter++;
    if (iter >= 3) iter = 0;
}
