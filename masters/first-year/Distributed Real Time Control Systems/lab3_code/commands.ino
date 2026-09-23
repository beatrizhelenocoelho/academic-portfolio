void readCommand() {

  if (!Serial.available()) return;
  String cmd = Serial.readStringUntil('\n');
  cmd.trim();

  if(cmd.startsWith("u ")){ // function set pwm

    int i;
    int val;

    sscanf(cmd.c_str(),"u %d %d",&i,&val);

    if(i == luminaireID){

      if(val < 0) val = 0;
      if(val > DAC_RANGE) val = DAC_RANGE;

      pwmCommand = val;
      currentPWM = val;

      analogWrite(LED_PIN, pwmCommand);

      // Serial.println("ack");

    } else Serial.println("err");
  }

  else if(cmd.startsWith("g u ")){ // function get pwm

    int i;
    sscanf(cmd.c_str(),"g u %d",&i);

    if(i == luminaireID){

      Serial.print("u ");
      Serial.print(i);
      Serial.print(" ");
      Serial.println(pwmCommand);

    } else Serial.println("err");
  }

  else if(cmd.startsWith("r ")){ // function set reference

    int i;
    float val;

    sscanf(cmd.c_str(),"r %d %f",&i,&val);

    if(i == luminaireID){

      refLux = val;
      // Serial.println("ack");

    } else Serial.println("err");
  }

  else if(cmd.startsWith("g r ")){ // function get reference

    int i;
    sscanf(cmd.c_str(),"g r %d",&i);

    if(i == luminaireID){

      Serial.print("r ");
      Serial.print(i);
      Serial.print(" ");
      Serial.println(refLux);

    } else Serial.println("err");
  }

  else if(cmd.startsWith("g y ")){ // function get lux

    int i;
    sscanf(cmd.c_str(),"g y %d",&i);

    if(i == luminaireID){

      Serial.print("y ");
      Serial.print(i);
      Serial.print(" ");
      Serial.println(measuredLux);

    } else Serial.println("err");
  }

  else if(cmd.startsWith("g v ")){ // function get LDR voltage

    int i;
    sscanf(cmd.c_str(),"g v %d",&i);

    if(i == luminaireID){

      Serial.print("v ");
      Serial.print(i);
      Serial.print(" ");
      Serial.println(voltageLDR);

    } else Serial.println("err");
  }

  else if(cmd.startsWith("o ")){ // function set occupancy

    int i;
    char val;

    sscanf(cmd.c_str(),"o %d %c",&i,&val);

    if(i == luminaireID){

      if(val == 'o' || val == 'l' || val == 'h'){
        deskState = val;

        if(val == 'o') refLux = 0.0;
        if(val == 'l') refLux = lowRef;
        if(val == 'h') refLux = highRef;

        // Serial.println("ack");
      }
      else{
        Serial.println("err");
      }

    } else Serial.println("err");
  }

  else if(cmd.startsWith("g o ")){ // function get occupancy

    int i;
    sscanf(cmd.c_str(),"g o %d",&i);

    if(i == luminaireID){

      Serial.print("o ");
      Serial.print(i);
      Serial.print(" ");
      Serial.println(deskState);

    } else Serial.println("err");
  }

  else if(cmd.startsWith("f ")){ // function set feedback

    int i;
    int val;

    sscanf(cmd.c_str(),"f %d %d",&i,&val);

    if(i == luminaireID){

      feedbackOn = (val != 0);
      // Serial.println("ack");

    } else Serial.println("err");
  }

  else if(cmd.startsWith("g f ")){ // function get feedback

    int i;
    sscanf(cmd.c_str(),"g f %d",&i);

    if(i == luminaireID){

      Serial.print("f ");
      Serial.print(i);
      Serial.print(" ");
      Serial.println(feedbackOn);

    } else Serial.println("err");
  }

  else if(cmd.startsWith("a ")){ // function set anti-windup

    int i;
    int val;

    sscanf(cmd.c_str(),"a %d %d",&i,&val);

    if(i == luminaireID){

      antiWindupOn = (val != 0);
      // Serial.println("ack");

    } else Serial.println("err");
  }

  else if(cmd.startsWith("g a ")){ // function get anti-windup

    int i;
    sscanf(cmd.c_str(),"g a %d",&i);

    if(i == luminaireID){

      Serial.print("a ");
      Serial.print(i);
      Serial.print(" ");
      Serial.println(antiWindupOn);

    } else Serial.println("err");
  }

  else if(cmd.startsWith("g E ")){ // get accumulated energy

    int i;
    sscanf(cmd.c_str(),"g E %d",&i);

    if(i == luminaireID){
      Serial.print("E ");
      Serial.print(i);
      Serial.print(" ");
      Serial.println(energyJ, 6);
    } else Serial.println("err");
  }

  else if(cmd.startsWith("g V ")){ // get average visibility error

    int i;
    sscanf(cmd.c_str(),"g V %d",&i);

    if(i == luminaireID){
      float avgV = 0.0;
      if(metricSamples > 0) avgV = visibilityErrorAcc / metricSamples;

      Serial.print("V ");
      Serial.print(i);
      Serial.print(" ");
      Serial.println(avgV, 6);
    } else Serial.println("err");
  }

  else if(cmd.startsWith("g F ")){ // get average flicker error

    int i;
    sscanf(cmd.c_str(),"g F %d",&i);

    if(i == luminaireID){
      float avgF = 0.0;
      if(metricSamples > 0) avgF = flickerErrorAcc / metricSamples;

      Serial.print("F ");
      Serial.print(i);
      Serial.print(" ");
      Serial.println(avgF, 6);
    } else Serial.println("err");
  }

  else if(cmd.startsWith("g d ")){ // get external illuminance

    int i;
    sscanf(cmd.c_str(),"g d %d",&i);

    if(i == luminaireID){
      float externalLux = readLuxRaw() - measuredLux;
      if(externalLux < 0.0) externalLux = 0.0;

      Serial.print("d ");
      Serial.print(i);
      Serial.print(" ");
      Serial.println(externalLux, 6);
    } else Serial.println("err");
  }

  else if(cmd.startsWith("g p ")){ // get instantaneous power

    int i;
    sscanf(cmd.c_str(),"g p %d",&i);

    if(i == luminaireID){
      float duty = currentPWM / (float)DAC_RANGE;
      float powerW = PMAX * duty;

      Serial.print("p ");
      Serial.print(i);
      Serial.print(" ");
      Serial.println(powerW, 6);
    } else Serial.println("err");
  }

  else{
    Serial.println("err");
  }
}