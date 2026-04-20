#include <Arduino.h>
#include <DHT.h>
#include <Arduino_FreeRTOS.h>

#define DHTPIN A0
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

float temperature = 30.3;
float humidity = 75.5;
float esr[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

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

    vTaskDelay(3000 / portTICK_PERIOD_MS);
  }
}

void setup()
{
  Serial.begin(9600);
  dht.begin();

  xTaskCreate(readDHTTask, "ReadDHT", 2048, NULL, 1, NULL);
  xTaskCreate(printTask, "Print", 2048, NULL, 1, NULL);
}

void loop()
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

  delay(1000); // send every 1 second
}