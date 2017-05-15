#include <Bridge.h>
#include <YunServer.h>
#include <YunClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
int dac = 0;
int DigitalPin[] = {2, 12, 13};
int DacPin = 3;

#define ONE_WIRE_BUS 2  // Data wire is pin 2 on the Arduino
OneWire oneWire(ONE_WIRE_BUS);  // Setup a oneWire instance to communicate with any OneWire device
DallasTemperature sensors(&oneWire);  // Pass the oneWire reference to Dallas Temperature. 
YunServer server;

void setup() {
  pinMode(A0, OUTPUT);
  pinMode(2,INPUT);
  pinMode(4,INPUT);
  pinMode(12,OUTPUT);
  pinMode(13,OUTPUT);
  digitalWrite(13, HIGH); //on, bridge to A0 somehow?
  Bridge.begin();
  digitalWrite(13, LOW); //off
  server.listenOnLocalhost();
  server.begin();
}

void loop() {
  sensors.requestTemperatures(); // Send the command to get temperatures
  YunClient client = server.accept();
  
  if (client) {
    process(client);
    client.stop();
  }
    delay(50);
  }
  
void process(YunClient client) {
  String command = client.readStringUntil('/');
  
  if (command == "digital") {
    digitalCommand(client);
  }
  
  if (command == "dac") {
    dacCommand(client);
  }
  
  if (command == "status") {
    statusCommand(client);
  }
}
  
void digitalCommand(YunClient client) {
  int pin, value;
  pin = client.parseInt();
  
  if (client.read() == '/') {
    value = client.parseInt();
    digitalWrite(pin, value);
  } else {
    value = digitalRead(pin);
    }
    
  client.print(F("digital,"));
  client.print(pin);
  client.print(F(","));
  client.println(value);
}

void dacCommand(YunClient client) {
  int pin, value;
  pin = client.parseInt();
  
  if (client.read() == '/') {
    value = client.parseInt();
    dac = value;
    analogWrite(pin, value);
  } else {
    value = dac;
    }
    
  client.print(F("dac,"));
  client.print(pin);
  client.print(F(","));
  client.println(value);
}

void statusCommand(YunClient client) {
  int pin, value;
  client.print(F("status"));
  
  for (int thisPin = 0; thisPin < 3; thisPin++) { //cycle through digital pins, starting at 0 ending at 2
    pin = DigitalPin[thisPin];
    value = digitalRead(pin);
    client.print(F("#"));
    client.print(pin);
    client.print(F("="));
    client.print(value);
  } {
    pin = DacPin;
    value = dac;
    client.print(F("#")); 
    client.print(pin); 
    client.print(F("="));
    client.print(value);

    int temp = (sensors.getTempFByIndex(0));
    int correctedTemp = (temp +15); //calibration
    
    if (dac <= correctedTemp) {   //Control loop for heating element, dac is goal temp
      digitalWrite(A0, HIGH);
      Console.println("HIGH");
      } else {
      digitalWrite(A0, LOW);
      Console.println("LOW");
      }
   } {
    //display temp on website
    value = analogRead(0);
    int temp = (sensors.getTempFByIndex(0));
    int correctedTemp = (temp +15); //calibration
    client.print(F("#A0")); //was A0
    client.print(F("="));
    client.print(correctedTemp);
  }
  client.println("");
}
