#include <Arduino.h>

float temperature = 30.3;
float humidity = 75.5;
float esr[10] = {5, 5, 5, 5, 5, 5, 5, 5, 5, 5};

void setup()
{
  Serial.begin(9600);
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

  Serial.println(); // end line

  delay(1000); // send every 1 second
}