#include "pid.h"

pid::pid(float _h, float _kp, float _b, float _ki, float _kd, float _NN, float _kt)
  //member variable initialization list
  : h{_h}, kp{_kp}, b{_b}, ki{_ki}, kd{_kd}, NN{_NN}, kt{_kt}, i{0.0}, d{0.0}, y_old{0.0}
  {} //should check arguments validity

float pid::compute_control(float r, float y){
  float p = kp*(b*r-y); //o b tem a ver com aquela contante q vimos nas aulas
  float ad = kd/(kd+NN*h);
  float bd = kd*NN/(kd+NN*h);
  d = ad*d - bd*(y-y_old);
  y_old = y;  // FIX: Update y_old so next call has correct previous value

  float v = p+i+d;
  
  // CANNOT SATURATE HERE -> will use pid::saturation
  return v;

}

float pid::saturation(float v){
  float u=v;
  if (v<0) u=0;
  if (v>4095) u=4095;
  return u;
}

void pid::bumpless_transfer(float new_kp, float new_ki, float new_b, float r, float y){
  float old_term = kp*(b*r-y);
  float new_term = new_kp*(new_b*r-y);
  i = i + old_term - new_term;

  kp = new_kp;
  ki = new_ki;
  //kd = new_kd; -> who cares about the derivative 
  b = new_b;

}

// Getters -> not used for now 
float pid::get_kp(){ 
  return kp;
}

float pid::get_ki(){
  return ki;
}

float pid::get_b(){
  return b;
}
