#include <SerialParser.h>

SerialParser serialParser;
String mapConfig[7] = {
  "data1",
  "data2",
  "data3",
  "data4",
  "data5",
  "data6",
  "data7"
};

String serialData = "";

void setup() {
  Serial.begin(9600);
  serialParser.init(mapConfig, 7);
}

void loop() {
  while(Serial.available() > 0){
    serialData = Serial.readStringUntil('#');
    serialParser.parse(serialData, ';');
    Serial.println("===========================================");
    Serial.print("Data1: "); Serial.println(serialParser.getValue("data1"));
    Serial.print("Data2: "); Serial.println(serialParser.getValue("data2"));
    Serial.print("Data3: "); Serial.println(serialParser.getValue("data3"));
    Serial.print("Data4: "); Serial.println(serialParser.getValue("data4"));
    Serial.print("Data5: "); Serial.println(serialParser.getValue("data5"));
    Serial.print("Data6: "); Serial.println(serialParser.getValue("data6"));
    Serial.print("Data7: "); Serial.println(serialParser.getValue("data7"));
    Serial.println("==========================================="); Serial.println();
  }
}

void printConfig(String *config) {
  Serial.println("===========================================");
  for(int idx = 0; idx < 7; idx++){
    Serial.println(config[idx]);
  }
  Serial.println("==========================================="); Serial.println();
}
