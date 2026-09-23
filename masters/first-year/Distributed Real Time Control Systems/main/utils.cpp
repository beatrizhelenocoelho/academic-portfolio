#include "utils.h"

// Base PID constants 
float m;
float b;
float kp_init;
float ki_init;
float b_init;
float kt_init;

// ---------- PHASE 1 FUNCS ---------- // 

// LUX calculation funcs
float get_R_LDR(float V_measured) {
  float R_LDR;  //ohm
  R_LDR = R_fixed * (Vcc - V_measured) / V_measured;
  return R_LDR;
}

float get_LUX(float R_LDR) {  //calculate lux
  float lux;
  lux = pow(10, (log10(R_LDR) - b) / m);
  return lux;
}

float read_LUX_calib() {
  float sum_volts = 0.0;
  int n_samples = 100;

  for (int i = 0; i < n_samples; i++) {
    sum_volts += analogRead(LDR_PIN) * Vcc / DAC_RANGE;
    delay(2);
  }
  float volts = sum_volts/100.0;
  float R_LDR = get_R_LDR(volts);
  float lux = get_LUX(R_LDR);
  return lux;
}

/*
float read_LUX_always() { // Removed for now, useless 
  
  float volts = analogRead(LDR_PIN) * Vcc / DAC_RANGE;
  float R_LDR_av = get_R_LDR(volts);
  float lux_av = get_LUX(R_LDR_av);

  return lux_av;
}
*/

float read_volts(){ // Faster because does less math 
  float volts = analogRead(LDR_PIN) * Vcc / DAC_RANGE;
  return volts;
}

void set_led(char format, float value) {
  int pwm = 0;
  if (format == 'p') {
    pwm= (int) value;
    if (pwm >= 0 && pwm <= DAC_RANGE) {
      analogWrite(LED_PIN, pwm);
    }
  } 
  else if (format == 'u') {
    if (value >= 0.0 && value <= 1.0) {
        pwm = value*DAC_RANGE;
        analogWrite(LED_PIN, pwm);
    }
  }
  else if (format == 'g'){ //percentaGGGe
    if (value >= 0.0 && value <= 100.0){
      pwm= value*DAC_RANGE/100.0;
      analogWrite(LED_PIN,pwm);
    }
  }
}

void gain_and_background(float &d, float &G){
  set_led('p', 0);
  delay(3000);
  d = read_LUX_calib();
  Serial.print("d: "); Serial.println(d);
  set_led('p', DAC_RANGE);
  delay(3000);
  G = read_LUX_calib()-d;
  Serial.print("G: "); Serial.println(G);

}

// Performance measures helpers 
float energy(float prev_duty, unsigned long dt){
  return P_max * prev_duty * (dt)/1000000.0;
}

float visibility(float r, float expected_lux){
  float vis = 0.0;
  if (r > expected_lux){
    vis = r-expected_lux;
  }
  return vis;
}

float flicker(float cur_duty, float prev_duty, float prev2_duty){
  float fli = 0.0;
  if ((cur_duty-prev_duty)*(prev_duty-prev2_duty) <0.0){
    fli = fabs(cur_duty-prev_duty) + fabs(prev_duty-prev2_duty); 
  }
  return fli;
}

// ---------- PHASE 2 FUNCS ---------- // 

uint32_t get_board_uid32() {
  pico_unique_board_id_t id;
  pico_get_unique_board_id(&id);

  uint32_t h = 2166136261u;   // FNV-1a simples
  for (int i = 0; i < PICO_UNIQUE_BOARD_ID_SIZE_BYTES; i++) {
    h ^= id.id[i];
    h *= 16777619u;
  }
  return h;
}

// ======== BOARD IDENTIFIER ========
// Set the known boards here 
// ==================================

void load_board_params(uint32_t uid) {
  (void)uid;  // não usamos o hash aqui para comparar parâmetros locais

  char id_str[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1];
  pico_get_unique_board_id_string(id_str, sizeof(id_str));

  if (strcmp(id_str, "E660C0D1C7864824") == 0) {
    kp_init = 20.0;
    ki_init = 90.0;
    m = -0.80;
    b = 6.32;
    b_init = 7.0;
    //kt_init = 2.0;
    kt_init = 0.0; // 0 to use clamping 
    
  }
  else if (strcmp(id_str, "E66118604B4A4B21") == 0) {
    kp_init = 20.0;
    ki_init = 90.0;
    m = -0.87;
    b = 6.30;
    b_init = 7.0;
    //kt_init = 2.0;
    kt_init = 0.0;

  }
  else if (strcmp(id_str, "E66118604B3B2221") == 0) {
    kp_init = 10.0;
    ki_init = 90.0;
    m = -0.80;
    b = 6.24;
    b_init = 4.5;
    //kt_init = 2.0;
    kt_init = 0.0;
  }
  else {
    kp_init = 10.0;
    ki_init = 80.0;
    m = -0.80;
    b = 6.24;
    b_init = 4.5;
    kt_init = 0.8;
    Serial.println("Unknown board ID, using default parameters.");
  }
}