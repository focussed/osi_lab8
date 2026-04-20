
// TMP36 Temperature Sensor on A0
// Responds to 'T' over Serial with temperature in Celsius
// ATU Sligo - Operating Systems Interfacing Lab 8

const int sensorPin = A0;
const float voltagePerDegree = 0.01;  // 10mV per degree C
const float zeroDegreeVoltage = 0.5;   // 500mV at 0°C

void setup() {
  Serial.begin(9600);
}

void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == 'T' || cmd == 't') {
      int sensorValue = analogRead(sensorPin);
      float voltage = (sensorValue / 1023.0) * 5.0;
      float temperatureC = (voltage - zeroDegreeVoltage) / voltagePerDegree;
      
      Serial.print("TEMP: ");
      Serial.println(temperatureC, 1);
    }
  }
  delay(100);
}