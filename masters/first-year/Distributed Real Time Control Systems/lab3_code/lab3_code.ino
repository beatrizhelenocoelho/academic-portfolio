#include <math.h>

const int LED_PIN = 15;
const int ID_PIN  = 2;
const int LDR_PIN = A0;

const int DAC_RANGE = 4095;
int luminaireID;

float m;
float b;
float backgroundLux = 0.0;
float gain = 1.0;
float measuredLux = 0.0;
float filteredLux = 0.0;
float previousLux = 0.0;
float voltageLDR  = 0.0;
float refLux      = 5.0;
float previousRefLux = 5.0;
float lowRef  = 5.0;
float highRef = 15.0;
char deskState    = 'h';
bool feedbackOn   = true;
bool antiWindupOn = true;
int pwmCommand = 0;
int currentPWM = 0;


// Performance measures
float energyJ = 0.0;
float visibilityErrorAcc = 0.0;
float flickerErrorAcc = 0.0;

unsigned long metricSamples = 0;

float prevDuty1 = 0.0;
float prevDuty2 = 0.0;
const float PMAX = 1.0;

// Perturbações
int disturbanceCounter = 0;
const int DISTURBANCE_HOLD = 400;        // ~4 s
const float DISTURBANCE_DELTA_LUX = 2.0; // limiar
const int DISTURBANCE_MAX_PWM_STEP = 2;  // lento a perturbações

// Temporização do controlo
const float TS = 0.01;
const unsigned long TS_US = 10000UL;
unsigned long lastControlMicros = 0;

// Jitter measurement
bool firstJitterSample = true;
unsigned long previousControlStart = 0;
unsigned long jitterSampleCount = 0;
double jitterSum = 0.0;
double jitterSumSq = 0.0;
unsigned long jitterMin = 1000000UL;
unsigned long jitterMax = 0;
long maxAbsJitterError = 0;

// Filtro exponencial na medição
const float ALPHA_REF_CHANGE = 0.25;   // rápido a mudanças de referência
const float ALPHA_NORMAL     = 0.05;   // normal
const float ALPHA_DISTURB    = 0.02;   // lento a perturbações
const bool PRINT_STARTUP_INFO = false;  // true só para debug humano
const bool PRINT_CSV_HEADER   = false;  // true se quiseres header no terminal

float clampFloat(float x, float xmin, float xmax) {
  if (x < xmin) return xmin;
  if (x > xmax) return xmax;
  return x;
}

int readADCMean(int nSamples = 8) {
  long acc = 0;
  for (int i = 0; i < nSamples; i++) {
    acc += analogRead(LDR_PIN);
  }
  return (int)(acc / nSamples);
}

float readLuxRaw() {
  int adc = readADCMean(8);

  float voltage = adc * 3.3 / 4095.0;
  voltageLDR = voltage;

  if (voltage < 0.02) voltage = 0.02;
  if (voltage > 3.28) voltage = 3.28;

  const float Rfixed = 10000.0;
  float resistance = Rfixed * (3.3 - voltage) / voltage;

  float lux = pow(10.0, (log10(resistance) - b) / m);
  return lux;
}

// Lux útil da LED = lux total - background
float readLux() {
  float lux = readLuxRaw();
  lux -= backgroundLux;
  if (lux < 0.0) lux = 0.0;
  return lux;
}

float averageLuxRaw(int nSamples = 20) {
  float sum = 0.0;
  for (int i = 0; i < nSamples; i++) {
    sum += readLuxRaw();
    delay(50);
  }
  return sum / nSamples;
}

void writePWM(float pwm) {
  pwm = clampFloat(pwm, 0.0, (float)DAC_RANGE);
  int pwmInt = (int)round(pwm);

  pwmCommand = pwmInt;
  currentPWM = pwmInt;
  analogWrite(LED_PIN, pwmInt);
}

class PIDController {
public:
  float kp = 0.0;
  float ki = 0.0;
  float kd = 0.0;

  float beta = 0.2;
  float kaw  = 0.0;
  float tauD = 0.02;

  float Ts   = 0.01;
  float uMin = 0.0;
  float uMax = 1.0;

  float integrator = 0.0;
  float dFilter    = 0.0;
  float yPrev      = 0.0;
  bool firstSample = true;

  void reset(float y0 = 0.0) {
    integrator = 0.0;
    dFilter = 0.0;
    yPrev = y0;
    firstSample = true;
  }

  float update(float r, float y, float uff, bool fbEnable, bool awEnable) {
    if (firstSample) {
      yPrev = y;
      firstSample = false;
    }

    float dy = (y - yPrev) / Ts;

    float a = tauD / (tauD + Ts);
    dFilter = a * dFilter + (1.0 - a) * dy;

    float e  = r - y;
    float ep = beta * r - y;

    float ufb = 0.0;
    if (fbEnable) {
      ufb = kp * ep + integrator - kd * dFilter;
    }

    float uUnsat = uff + ufb;
    float uSat   = clampFloat(uUnsat, uMin, uMax);

    if (fbEnable) {
      float awTerm = 0.0;
      if (awEnable) {
        awTerm = kaw * (uSat - uUnsat);
      }
      integrator += Ts * (ki * e + awTerm);
    }

    yPrev = y;
    return uSat;
  }
};

PIDController pid;

// Calibração automática 
void calibrateSystem() {
  writePWM(0.0);
  delay(4000);
  backgroundLux = averageLuxRaw(20);

  writePWM(DAC_RANGE);
  delay(4000);
  float maxLux = averageLuxRaw(20);

  gain = maxLux - backgroundLux;
  if (gain < 0.1) gain = 0.1;

  writePWM(0.0);
}

// Inicialização automática dos ganhos a partir do ganho estático
void configureControllerFromGain() {
  float invGain = 1.0 / max(gain, 1.0f);

  pid.kp   = 0.35 * invGain;
  pid.ki   = 0.4 * invGain;
  pid.kd   = 0.0;
  pid.beta = 1;
  pid.kaw  = 3.0 * pid.ki;
  pid.tauD = 0.03;
  pid.Ts   = TS;
  pid.uMin = 0.0;
  pid.uMax = 1.0;

  pid.reset(0.0);
}

float computeFeedforwardDuty(float refLuxValue) {
  if (refLuxValue <= 0.0) return 0.0;

  float dutyFF = refLuxValue / max(gain, 1.0f);

  // suavizar um pouco referências baixas
  if (refLuxValue <= 3.5) {
    dutyFF *= 0.85;
  }

  return clampFloat(dutyFF, 0.0, 1.0);
}

// Tarefa de controlo
void controlTask() {
  unsigned long now = micros();
  if ((now - lastControlMicros) < 10000UL) return;
  lastControlMicros += 10000UL;
  if (firstJitterSample) {
    previousControlStart = now;
    firstJitterSample = false;
  } else {
    unsigned long dt = now - previousControlStart;
    previousControlStart = now;

    jitterSampleCount++;
    jitterSum += dt;
    jitterSumSq += (double)dt * (double)dt;

    if (dt < jitterMin) jitterMin = dt;
    if (dt > jitterMax) jitterMax = dt;

    long err = (long)dt - (long)TS_US;
    if (labs(err) > maxAbsJitterError) {
      maxAbsJitterError = labs(err);
    }
  }   
  measuredLux = readLux();

  // detetar perturbação: referência igual, lux muda muito
  float deltaRef = fabs(refLux - previousRefLux);
  float deltaLux = fabs(measuredLux - previousLux);

  if (deltaRef < 0.1 && deltaLux > DISTURBANCE_DELTA_LUX) {
    disturbanceCounter = DISTURBANCE_HOLD;
  }

  if (disturbanceCounter > 0) {
    disturbanceCounter--;
  }

  // se já caiu para zero, a perturbação deixou de fazer sentido
  if (measuredLux < 0.2) {
    disturbanceCounter = 0;
  }

  bool disturbance = disturbanceCounter > 0;

  // filtro simples, mas com 3 velocidades
  float alpha;
  if (deltaRef > 0.2) {
    alpha = ALPHA_REF_CHANGE;
  } else if (disturbance) {
    alpha = ALPHA_DISTURB;
  } else {
    alpha = ALPHA_NORMAL;
  }

  filteredLux = (1.0 - alpha) * filteredLux + alpha * measuredLux;

  int targetPWM;

  if (refLux <= 0.0) {
    pid.reset(filteredLux);
    targetPWM = 0;
  } else {
    float uff = computeFeedforwardDuty(refLux);
    float dutyCmd = pid.update(refLux, filteredLux, uff, feedbackOn, antiWindupOn);
    targetPWM = (int)round(dutyCmd * DAC_RANGE);
  }

  // lento a perturbações também na saída
  if (disturbance) {
    if (targetPWM > currentPWM + DISTURBANCE_MAX_PWM_STEP) {
      targetPWM = currentPWM + DISTURBANCE_MAX_PWM_STEP;
    }
    else if (targetPWM < currentPWM - DISTURBANCE_MAX_PWM_STEP) {
      targetPWM = currentPWM - DISTURBANCE_MAX_PWM_STEP;
    }
  }

  writePWM(targetPWM);

  // Performance measures
  float duty = currentPWM / (float)DAC_RANGE;   // duty cycle em [0,1]

  // energia acumulada
  energyJ += PMAX * duty * TS;

  // visibility error (estimado a partir do comando LED)
  float estimatedLux = duty * gain;
  visibilityErrorAcc += max(0.0f, refLux - estimatedLux);

  // flicker error
  float dd1 = duty - prevDuty1;
  float dd2 = prevDuty1 - prevDuty2;
  float fk = 0.0f;

  // excluir transições explícitas de referência
  if ((dd1 * dd2) < 0.0f && fabs(refLux - previousRefLux) < 0.1f) {
    fk = fabs(dd1) + fabs(dd2);
  }

  flickerErrorAcc += fk;

  prevDuty2 = prevDuty1;
  prevDuty1 = duty;

  metricSamples++;

  previousLux = measuredLux;
  previousRefLux = refLux;
}
void printJitterStats() {
  if (jitterSampleCount == 0) {
    Serial.println("No jitter samples collected yet.");
    return;
  }

  double mean = jitterSum / jitterSampleCount;
  double variance = (jitterSumSq / jitterSampleCount) - (mean * mean);
  if (variance < 0.0) variance = 0.0;
  double stddev = sqrt(variance);

  Serial.println("=== Jitter Statistics ===");
  Serial.print("Samples: ");
  Serial.println(jitterSampleCount);

  Serial.print("Mean dt (us): ");
  Serial.println(mean, 3);

  Serial.print("Std dev (us): ");
  Serial.println(stddev, 3);

  Serial.print("Min dt (us): ");
  Serial.println(jitterMin);

  Serial.print("Max dt (us): ");
  Serial.println(jitterMax);

  Serial.print("Max abs error from 10000 us: ");
  Serial.println(maxAbsJitterError);
}

void printStatusEvery10ms() {
  static unsigned long lastPrintMicros = 0;
  static unsigned long t0 = micros();
  unsigned long now = micros();

  if (now - lastPrintMicros < 10000UL) return;
  lastPrintMicros += 10000UL;

  float t = (now - t0) * 1e-6;

  Serial.print(t, 4);
  Serial.print(",");
  Serial.print(refLux, 2);
  Serial.print(",");
  Serial.print(filteredLux, 2);
  Serial.print(",");
  Serial.println(currentPWM);
}

// setup / loop
void setup() {
  Serial.begin(115200);
  while (!Serial);

  pinMode(LED_PIN, OUTPUT_12MA);
  pinMode(ID_PIN, INPUT_PULLUP);

  analogReadResolution(12);
  analogWriteFreq(60000);
  analogWriteRange(DAC_RANGE);

  luminaireID = digitalRead(ID_PIN);

  if (luminaireID == 0) {
    m = -0.8;
    b = 6.02;
  } else {
    m = -0.8;
    b = 5.72;
  }

  calibrateSystem();
  configureControllerFromGain();

  measuredLux = readLux();
  filteredLux = measuredLux;
  previousLux = measuredLux;

  writePWM(0.0);

  lastControlMicros = micros();

  if (PRINT_STARTUP_INFO) {
    Serial.print("Luminaire ID: ");
    Serial.println(luminaireID);

    Serial.print("backgroundLux = ");
    Serial.println(backgroundLux, 4);

    Serial.print("gain = ");
    Serial.println(gain, 4);
  }

  if (PRINT_CSV_HEADER) {
    Serial.println("time,ref,lux,pwm,u_percent,power,energy,avgVisibility,avgFlicker");
  }
}

void loop() {
  readCommand();
  controlTask();
  printStatusEvery10ms();

  static unsigned long lastReport = millis();
  /*if (millis() - lastReport > 10000) {   // a cada 10 s
    printJitterStats();
    lastReport = millis();
  }*/
}