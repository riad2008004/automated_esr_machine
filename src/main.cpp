#include <Arduino.h>
#include <DHT.h>
#include <Stepper.h>
#include <Arduino_FreeRTOS.h>

#define DHTPIN A0
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define FAN 38
#define BUZZER 39

#define STEPS 2048

float temperature = 30.3;
float humidity = 75.5;
float esr[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Sensor from 1 : A1,A6,A8,A5,A7,A3,A9,A4,A10,A2
// LED From 1: 23,22,21,20,19,18,17,16,15,14
int esr_sensor[10] = {A1, A6, A8, A5, A7, A3, A9, A4, A10, A2};
int indicator[10] = {23, 22, 21, 20, 19, 18, 17, 16, 15, 14};

// Motor 1
Stepper motor1(STEPS, 29, 31, 30, 49);
// Motor 2
Stepper motor2(STEPS, 48, 50, 51, 52);

void stepperTask(void *pvParameters)
{
  vTaskDelay(3000 / portTICK_PERIOD_MS);

  const int maxSteps = 5000;
  int stepCount = 0;
  int direction = 1;
  int temp_testtube = 0;

  while (1)
  {
    temp_testtube = 0;
    for (int i = 0; i < 10; i++)
    {
      if (digitalRead(esr_sensor[i]))
        temp_testtube++;
    }
    if (temp_testtube != 0)
    {
      // -------- FORWARD 5000 STEPS --------
      stepCount = 0;
      direction = 1;

      while (stepCount < maxSteps)
      {
        motor1.step(direction);
        motor2.step(direction);

        stepCount++;
        taskYIELD();
      }

      // -------- BACKWARD 5000 STEPS --------
      stepCount = 0;
      direction = -1;

      while (stepCount < maxSteps)
      {
        motor1.step(direction);
        motor2.step(direction);

        stepCount++;
        taskYIELD();
      }
      // make rest the motor
      digitalWrite(29, LOW);
      digitalWrite(30, LOW);
      digitalWrite(31, LOW);
      digitalWrite(49, LOW);

      digitalWrite(48, LOW);
      digitalWrite(51, LOW);
      digitalWrite(50, LOW);
      digitalWrite(52, LOW);
      // -------- REST FOR 1 MINUTE --------
      vTaskDelay(60000 / portTICK_PERIOD_MS); // 60,000 ms = 1 min
    }
  }
}

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

  motor1.setSpeed(10);
  motor2.setSpeed(10);

  xTaskCreate(readDHTTask, "ReadDHT", 1024, NULL, 1, NULL);
  xTaskCreate(printTask, "Print", 1024, NULL, 1, NULL);
  xTaskCreate(esrTask, "ESR Task", 128, NULL, 1, NULL);
  xTaskCreate(buzzerTask, "Buzzer", 128, NULL, 1, NULL);
  xTaskCreate(stepperTask, "Stepper Task", 1024, NULL, 1, NULL);

  vTaskStartScheduler();
}

void loop()
{
  // Running by RTOS
}