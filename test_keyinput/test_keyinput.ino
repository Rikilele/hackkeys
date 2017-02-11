uint8_t buf[8] = { 0 };

void setup() {
  Serial.begin(9600);

  pinMode(7, INPUT);
  digitalWrite(7, 1);

  delay(200);

}

void releaseKey(){
  buf[0] = 0;
  buf[2] = 0;
  Serial.write(buf, 8);
}

void loop() {
  int state = digitalRead(7);
  if(state != 1){
    buf[2] = 4;
    Serial.write(buf, 8);
    releaseKey();
  }

}
