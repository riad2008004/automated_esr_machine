#include <Arduino.h>
#include <DHT.h>
#include <Arduino_FreeRTOS.h>

#define DHTPIN A0
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define FAN 38
#define BUZZER 39

float temperature = 30.3;
float humidity = 75.5;
float esr[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Sensor from 1 : A1,A6,A8,A5,A7,A3,A9,A4,A10,A2
// LED From 1: 23,22,21,20,19,18,17,16,15,14
int esr_sensor[10] = {A1, A6, A8, A5, A7, A3, A9, A4, A10, A2};
int indicator[10] = {23, 22, 21, 20, 19, 18, 17, 16, 15, 14};

// Task: Read DHT11
void readDHTTask(void *parameter)
{
  while (true)
  {
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t) && !isnan(h))
    {
      temperature = t;
      humidity = h;

      if (temperature >= 30)
      {
        digitalWrite(FAN, LOW);
      }
      else
        digitalWrite(FAN, HIGH);
    }

    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}

// Task 2: Print Data
void printTask(void *parameter)
{
  while (true)
  {
    Serial.print("TEMP=");
    Serial.print(temperature, 1);

    Serial.print(" & HUM=");
    Serial.print(humidity, 1);

    for (int i = 0; i < 10; i++)
    {
      Serial.print(" & ESR");
      Serial.print(i + 1);
      Serial.print("=");
      Serial.print(esr[i], 1);
    }
    Serial.println();

    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}

// Task: Monitor all sensors
void esrTask(void *parameter)
{
  while (true)
  {

    for (int i = 0; i < 10; i++)
    {
      int sensorValue = digitalRead(esr_sensor[i]);

      // Assuming LOW = test tube detected
      if (sensorValue == LOW)
      {
        digitalWrite(indicator[i], HIGH);
        vTaskDelay(pdMS_TO_TICKS(300));
        digitalWrite(indicator[i], LOW);
      }
      else
      {
        digitalWrite(indicator[i], LOW); // LED OFF
      }
    }
    vTaskDelay(pdMS_TO_TICKS(300)); // small delay for stability
  }
}

void buzzerTask(void *parameter)
{
  for (int i = 0; i < 6; i++) // 6 × 500ms = ~3 seconds
  {
    digitalWrite(BUZZER, LOW);
    vTaskDelay(pdMS_TO_TICKS(250));

    digitalWrite(BUZZER, HIGH);
    vTaskDelay(pdMS_TO_TICKS(250));
  }

  digitalWrite(BUZZER, HIGH);

  vTaskDelete(NULL);
}

void setup()
{
  Serial.begin(9600);
  dht.begin();
  // Initialize pins
  for (int i = 0; i < 10; i++)
  {
    pinMode(esr_sensor[i], INPUT);
    pinMode(indicator[i], OUTPUT);
  }
  pinMode(BUZZER, OUTPUT);
  pinMode(FAN, OUTPUT);
  digitalWrite(BUZZER, HIGH);
  digitalWrite(FAN, HIGH);

  xTaskCreate(readDHTTask, "ReadDHT", 512, NULL, 1, NULL);
  xTaskCreate(printTask, "Print", 512, NULL, 1, NULL);
  xTaskCreate(esrTask, "ESR Task", 256, NULL, 1, NULL);
  xTaskCreate(buzzerTask, "Buzzer", 128, NULL, 1, NULL);
}

void loop()
{
  // Running by RTOS
}