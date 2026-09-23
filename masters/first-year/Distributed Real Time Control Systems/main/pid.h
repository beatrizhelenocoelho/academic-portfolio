#ifndef PID_H
#define PID_H

class pid{
  float i, d, kp, ki, kd, b, h, y_old, NN, kt;
  public:
  explicit pid(float _h, float _kp=1.0, float _b=1.0, float _ki=1.0, float _kd=0.0, float _NN=10.0, float _kt=1.0); //adicionei default para kt
  ~pid(){};

  float compute_control(float r, float y);
  void housekeep(float r, float y, float v, float u, bool anti_wind);
  float saturation(float v);
  void bumpless_transfer(float new_kp, float new_ki, float new_b, float r, float y);
  float get_kp();
  float get_ki();
  float get_b();

};

inline void pid::housekeep(float r, float y, float v, float u, bool anti_wind){
  float e = r - y;

  // Anti-windup logic (ignore anti_wind flag, always apply)
  if (anti_wind == false) {
    i += ki*h*e;  // integrate error
  }
  else{
    if (kt > 0) {
      // Back-calculation: subtract kt * (v - u) which is the saturation error
      i += ki*h*e - kt*h*(v - u);
    }
    else {
      // Clamping: stop integrating when saturated
      bool sat_H = (u >= 4095 && e > 0);  // saturated high and error positive
      bool sat_L = (u <= 0 && e < 0);     // saturated low and error negative
      
      if (!sat_H && !sat_L) {
        i += ki*h*e;
      }
      // else does not integrate when saturated
    }
  }

  y_old = y;
}

#endif //PID_H