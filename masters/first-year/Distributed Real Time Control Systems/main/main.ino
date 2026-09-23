// Include statements 
#include <math.h>
#include "pid.h"
#include "commands.h"
#include "utils.h"
#include "constants.h"
#include "can_bus.h"
#include "dual.h"
#include "pico/unique_id.h"
#include <string.h>
#include "shared.h"

#include "pico/multicore.h" // for can listener on core 1
#include "hardware/sync.h"

// ================= VARIABLES =================
//
// =============================================

// Node identification func 
void add_uid_if_new(uint32_t uid);
float G,d, y_lux, v_measured;
float duty_cycle=0.0;
float r =15.0f; // EVERYONE START WITH THIS REF
bool anti_windup = true;
bool feedback_on= true;
int node_id = 0; // default = not assigned 

// Table 3
float high_ref = 10.0;      // O -> occupancy state references and all 
float low_ref  = 5.0;       // U
float energy_cost = 1.0;    // C
bool restart_requested = false;

//CALIBRATION VARIABLES
bool *nodes_list = nullptr;
int calib_st = 0;
unsigned long cal_time;
int boss_node = 0;
float *Gs = nullptr;        // matriz linearizada
int nodes_count = 0;
int lux_replies = 0;
float *d_terms = nullptr;

// IDENT vars
int num_nodes = 0;
uint32_t my_uid = 0;
uint32_t *uids = nullptr;
int uid_count = 0;

// dynamic allocation for matrices of Gs and u 
#define GIDX(i,j) ((i) * (num_nodes + 1) + (j))
#define UHIDX(node, col) ((node) * MAX_CONV_HIST + (col))

// Necessary for startup 
void run_calibration(unsigned long cur_time);
void setup1();
void loop1();

// for debbugging -> não uso agora 
unsigned long last_print_ms = 0;
const unsigned long print_period_ms = 100;   // imprime de 100 em 100 ms
 
// PID declaration 
pid my_pid{h, kp_init, b_init, ki_init, 00.0, 10.0, kt_init};

// TIME keeping 
unsigned long prev_time = 0;
unsigned long prev_sample_time = 0;
unsigned long init_time = 0;

int sample_count = 0;
float sum_volts = 0.0;

// PERFORMANCE variables 
volatile float total_energy=0.0;
volatile float total_visibility=0.0;
volatile float total_flicker=0.0;

// ---- PROFILING -> to keep track of jitter and process delays -> pede no enunciado 
// Each 10s prints logs 
struct prof_t { unsigned long sum_us; unsigned long max_us; unsigned long count; };

prof_t prof_pid_compute = {0, 0, 0};  // duration of PID math per cycle
prof_t prof_pid_jitter  = {0, 0, 0};  // deviation from target period h

// Performance measure vars 
volatile unsigned long prof_can_sum_us = 0;
volatile unsigned long prof_can_max_us = 0;
volatile unsigned long prof_can_n      = 0;
float sum_visibility=0.0;
float sum_flicker=0.0;
float prev_duty = 0.0; //duty anterior
float prev2_duty = 0.0; //duty ante anterior
float total_n; //numero total de loops
int flicker_n = 0; //flicker tem um N diferente
int transient_count = 0;
float prev_r =-1;

// CALIBRATION
bool printed_gains = false; 
float *u_hist_matrix = nullptr; // flat: [num_nodes+1][MAX_CONV_HIST]
int   *u_hist_col    = nullptr;// write cursor per node
int u_hist_idx = 0;
bool skip_req = false;
int nodes_ack = 0;


// --- Shared state (volatile = no register caching across cores) ---
// These are 32-bit scalars: reads/writes are atomic on Cortex-M0+.
// volatile is sufficient — no spinlock needed for these.
volatile float sv_y_lux      = 0.0f;
volatile float sv_r          = 5;
volatile bool  sv_feedback_on = true;
volatile float sv_duty_cycle = 0.0f;
volatile int   sv_calib_st   = 0;
spin_lock_t   *pid_lock      = nullptr;
spin_lock_t   *can_lock      = nullptr; // !!!
unsigned long prev_time_pid = 0;

//volatile float sv_duty_cycle_shared[4] = {0.0f, 0.0f, 0.0f, 0.0f};
//volatile bool sv_duty_shared[4] = {true, false, false, false};
//volatile bool sv_update_u_cv = false;

volatile float *sv_duty_cycle_shared = nullptr;
volatile bool  *sv_duty_shared = nullptr;
volatile bool sv_update_u_cv = false;
volatile bool sv_my_turn = false; //added ass maria
volatile bool sv_converged =false; //added ass maria

volatile int sv_cycle_counter = 0; // added. ass maria
float u_ideal_cvg=0.0; //added ass maria

//  Dual Decomposition vars
volatile float *sv_lambda_shared = nullptr; // sv_lambda_shared[j] = lambda of node j
volatile int    sv_dual_phase    = 0;       //0 = u-phase, 1 = lambda-phase
volatile int    sv_dual_iter     = 0;       //iteration counter 


// ========== FUNCTION DECLARATIONS ============
// Ident funcs 
// =============================================

bool uid_exists(uint32_t uid) {
  for (int i = 0; i < uid_count; i++) {
    if (uids[i] == uid) return true;
  }
  return false;
}

void add_uid_if_new(uint32_t uid) {
  if (uid_exists(uid)) return;

  uint32_t *new_uids = new uint32_t[uid_count + 1];
  for (int i = 0; i < uid_count; i++) {
    new_uids[i] = uids[i];
  }
  new_uids[uid_count] = uid;

  delete[] uids;
  uids = new_uids;
  uid_count++;
}

void sort_uids() {
  for (int i = 0; i < uid_count - 1; i++) {
    for (int j = i + 1; j < uid_count; j++) {
      if (uids[j] < uids[i]) {
        uint32_t tmp = uids[i];
        uids[i] = uids[j];
        uids[j] = tmp;
      }
    }
  }
}

int node_id_from_uid(uint32_t uid) {
  for (int i = 0; i < uid_count; i++) {
    if (uids[i] == uid) return i + 1;   // IDs lógicos 1-N
  }
  return 0;
}

void allocate_network_arrays() { // allocate memory para matrizes 
  num_nodes = uid_count;
  nodes_count = uid_count;

  nodes_list = new bool[num_nodes + 1]();
  d_terms = new float[num_nodes + 1]();
  Gs = new float[(num_nodes + 1) * (num_nodes + 1)]();
  // For getting u :D
  u_hist_matrix = new float[(num_nodes + 1) * MAX_CONV_HIST]();
  u_hist_col    = new int  [num_nodes + 1]();

  sv_duty_cycle_shared = new float[num_nodes + 1]();
  sv_duty_shared = new bool[num_nodes + 1]();
  sv_lambda_shared = new float[num_nodes + 1]();  // dual decomp: one lambda per node

  for (int i = 1; i <= num_nodes; i++) {
    nodes_list[i] = true;
  }

  sv_duty_shared[0] = true;
  for (int i = 1; i <= num_nodes; i++) {
    sv_duty_shared[i] = false;
  }
}


// ================= SETUP0 ====================
//
// =============================================

void setup() {

  Serial.begin(115200);

  can_setup();

  pinMode(LED_PIN, OUTPUT_12MA);
  analogReadResolution(12);
  analogWriteFreq(20000); 
  analogWriteRange(4095);

  while (! Serial); // IMPORTANT
  
  my_uid = get_board_uid32();
  load_board_params(my_uid);

  set_led('u', 0.0);

  add_uid_if_new(my_uid);      // cada nó adiciona-se a si próprio
  can_send_wakeup(my_uid);     // wake-up com UID físico

  // Claim spinlock before core 1 starts — must happen on core 0 first
  pid_lock = spin_lock_init(spin_lock_claim_unused(true));
  can_lock = spin_lock_init(spin_lock_claim_unused(true)); // !!!

  init_time = micros();
  cal_time = init_time;

  // Serial.println("CAN ready"); // UNCOMMENT AFTER
  my_pid.bumpless_transfer(kp_init, ki_init, b_init, r, y_lux);

  prev_time = init_time;
  prev_sample_time = init_time;

  // Core 1 starts after setup() returns in Arduino-Pico
}

// ================== LOOP0 ====================
//
// =============================================

void loop() {
  // Rad lux -> handle CAN msg -> handle user cmds
  unsigned long cur_time = micros();

  // LDR - measure lux 
  if (cur_time - prev_sample_time >= 100) {
    prev_sample_time = cur_time;
    sum_volts += read_volts();
    sample_count++;
  }
  if (sample_count > 0) { // avg over values
    float av_volts = sum_volts / sample_count;
    sv_y_lux       = get_LUX(get_R_LDR(av_volts));
    v_measured     = av_volts;
    sum_volts      = 0.0;
    sample_count   = 0;
  }

  // Local copies to avoid race conditions
  float local_r  = sv_r;
  bool  local_fb = sv_feedback_on;
  float local_dc = sv_duty_cycle;

  {
    unsigned long _t0 = micros();
    can_handle_messages(node_id, num_nodes, uids, uid_count,
                        local_r, sv_y_lux, v_measured, my_pid,
                        nodes_list, anti_windup, local_fb, local_dc,
                        Gs, lux_replies, calib_st, boss_node, d_terms,
                        total_energy, total_visibility, total_flicker,
                        high_ref, low_ref, energy_cost, restart_requested);
    unsigned long _dt = micros() - _t0;
    prof_can_sum_us += _dt;
    if (_dt > prof_can_max_us) prof_can_max_us = _dt;
    prof_can_n++;
  }

  // Serial — pass locals, not sv_* directly
  serial_commands(my_pid, local_dc, local_r, sv_y_lux, v_measured,
                  anti_windup, local_fb, d, G,
                  total_energy, total_visibility, total_flicker, node_id,
                  high_ref, low_ref, energy_cost, restart_requested);
  // Write results back
  sv_r           = local_r;
  sv_feedback_on = local_fb;
  sv_duty_cycle  = local_dc;
  sv_calib_st    = calib_st;

  if (restart_requested) { // RESTART REQUESTED!
    restart_requested = false;

    // reset dos acumuladores
    total_energy = 0.0;
    total_visibility = 0.0;
    total_flicker = 0.0;
    total_n = 0.0;
    flicker_n = 0;
    transient_count = 0;
    sum_visibility = 0.0;
    sum_flicker = 0.0;
    prev_duty = 0.0;
    prev2_duty = 0.0;

    // volta ao estado inicial de controlo
    r = low_ref;
    sv_r = low_ref;
    duty_cycle = 0.0;
    sv_duty_cycle = 0.0;
    feedback_on = true;
    sv_feedback_on = true;
    calib_st = 0;
    sv_calib_st = 0;
    boss_node = 0;
    lux_replies = 0;
    node_id = 0;
    num_nodes = 0;
    nodes_count = 0;

    // libertar estruturas dinamicas -> gemini helped 
    if (nodes_list != nullptr) {
      delete[] nodes_list;
      nodes_list = nullptr;
    }
    if (Gs != nullptr) {
      delete[] Gs;
      Gs = nullptr;
    }
    if (d_terms != nullptr) {
      delete[] d_terms;
      d_terms = nullptr;
    }
    if (u_hist_matrix != nullptr) { 
      delete[] u_hist_matrix; u_hist_matrix = nullptr; 
    }
    if (u_hist_col    != nullptr) { 
      delete[] u_hist_col;    u_hist_col    = nullptr; 
    }
    if (sv_duty_cycle_shared != nullptr) {
      delete[] (float*)sv_duty_cycle_shared;
      sv_duty_cycle_shared = nullptr;
    }
    if (sv_duty_shared != nullptr) {
      delete[] (bool*)sv_duty_shared;
      sv_duty_shared = nullptr;
    }
    if (sv_lambda_shared != nullptr) {
      delete[] (float*)sv_lambda_shared;
      sv_lambda_shared = nullptr;
    }
    sv_dual_iter  = 0;
    sv_dual_phase = 0;
    if (uids != nullptr) {
      delete[] uids;
      uids = nullptr;
    }
    uid_count = 0;
    nodes_ack = 0;
    skip_req  = false;
    printed_gains = false;
    sv_converged  = false;
    sv_my_turn    = false;
    init_time = micros();

    add_uid_if_new(my_uid);
    cal_time = micros();
    can_send_wakeup(my_uid);

    Serial.println("System restarted");
  }
  // Calibration — completely unchanged, runs only on core 0
  if (sv_calib_st < 8){
    run_calibration(cur_time);
  }
}

// =========================================================
// CHANGE CORE 0 -> 1
// =========================================================

// ---------------- CORE 1: CAN LISTENER -----------------
void setup1() {
  // I am groot 

}

void loop1() {

  // ---- PERIODIC CAN ERROR CHECK ------
  static unsigned long last_error_check = 0;
  if (micros() - last_error_check >= 5000000) { // every 5 seconds
    can_check_errors(node_id);
    last_error_check = micros();
  }

  // ---- PERIODIC TIMING REPORT ------
  static unsigned long last_prof_print = 0;
  if (micros() - last_prof_print >= 10000000) { // every 10 seconds
    last_prof_print = micros();

    // Snapshot and reset CAN profiler (32-bit reads are atomic)
    unsigned long can_sum = prof_can_sum_us, can_max = prof_can_max_us, can_n = prof_can_n;
    prof_can_sum_us = 0; prof_can_max_us = 0; prof_can_n = 0;

    Serial.print("[TIMING node "); Serial.print(node_id); Serial.println("]");
    // printer 
    if (can_n > 0) {
      Serial.print("  CAN handle : avg=");
      Serial.print(can_sum / (float)can_n / 1000.0f, 3);
      Serial.print("ms  max=");
      Serial.print(can_max / 1000.0f, 3);
      Serial.println("ms");
    }
    if (prof_pid_jitter.count > 0) {
      Serial.print("  PID jitter : avg=");
      Serial.print(prof_pid_jitter.sum_us / (float)prof_pid_jitter.count / 1000.0f, 3);
      Serial.print("ms  max=");
      Serial.print(prof_pid_jitter.max_us / 1000.0f, 3);
      Serial.print("ms  (target=");
      Serial.print(h * 1000.0f, 1);
      Serial.println("ms)");
      prof_pid_jitter = {0, 0, 0};
    }
    if (prof_pid_compute.count > 0) {
      Serial.print("  PID compute: avg=");
      Serial.print(prof_pid_compute.sum_us / (float)prof_pid_compute.count / 1000.0f, 3);
      Serial.print("ms  max=");
      Serial.print(prof_pid_compute.max_us / 1000.0f, 3);
      Serial.println("ms");
      prof_pid_compute = {0, 0, 0};
    }
  }

  // ---- PRINTS PARA DEBUG ------
  static unsigned long last_core1_print = 0;
  if (micros() - last_core1_print > 2000000) { // Print every 2 seconds
    // Make atomic copies of shared volatile variables
    float local_r = sv_r;
    float local_lux = sv_y_lux;
    float local_duty = sv_duty_cycle;

    Serial.print("ID: ");
    Serial.print(node_id);
    Serial.print("| cal_st: ");
    Serial.print(sv_calib_st);
    Serial.print(" | conv: ");
    Serial.print(sv_converged);
    Serial.print(" | turn: ");
    Serial.print(sv_my_turn);
    Serial.print(" r="); Serial.print(local_r, 2);
    Serial.print(" y="); Serial.print(local_lux, 2);
    Serial.print(" u="); Serial.println(local_duty, 3);
    last_core1_print = micros();
  }


  if (sv_calib_st < 8) return;

  // -------- CONVERGENCE PHASE ---------
  else if (sv_converged == false){
    if (micros() - last_core1_print > 2000000) {
    //Serial.println("C1");
    }

    if (!use_dual_decomp) {
      // ========================= FEEDFORWARD ALGORITHM =========================
      // 
      // =========================================================================
      if (sv_my_turn == true){
        //Serial.println("C2");
        float local_r = sv_r;
        float local_dc  = sv_duty_cycle;
        float total_dist = 0.0;

        for (int i=1; i<=num_nodes; i++){
          if (i != node_id && nodes_list[i] == true){
            total_dist += Gs[GIDX(i, node_id)] * sv_duty_cycle_shared[i]; //influence of led i in node_id
          }
        }
        float u_ff = -total_dist/Gs[GIDX(node_id, node_id)];
        float u_ideal = (local_r - d_terms[node_id])/Gs[GIDX(node_id, node_id)] + u_ff;

        if (u_ideal < 0.0f) {
          u_ideal = 0.0f;
        }
        if (u_ideal > 1.0f) {
          u_ideal = 1.0f;
        }

        // --- BROADCAST TO OTHER NODES YOUR IDEAL DUTY CYCLE ---
        sv_duty_cycle = u_ideal;
        can_send_conv_duty(node_id, u_ideal);

        // GUARDAR O NOSSO PRÓPRIO VALOR NO HISTÓRICO
        if (u_hist_col[node_id] < MAX_CONV_HIST) {
          u_hist_matrix[UHIDX(node_id, u_hist_col[node_id]++)] = u_ideal;
        }
        delay(5);

        if (sv_cycle_counter > MAX_CONV_HIST){
          sv_converged =true;
          u_ideal_cvg = u_ideal;
          sv_cycle_counter = 0;

          PID_LOCK();
          my_pid.bumpless_transfer(my_pid.get_kp(), my_pid.get_ki(), my_pid.get_b(), local_r, sv_y_lux);
          PID_UNLOCK();
          Serial.println("converged");
        }

        // --- FIND NEXT NODE ---
        int next_turn = 0;

        if (nodes_list != nullptr) {
          for(int i = node_id+1; i<=num_nodes; i++){
            if (nodes_list[i]==true){
              next_turn =i;
              break;
            }
          }
          if (next_turn==0){
            for (int i=1; i<=num_nodes; i++){
              if (nodes_list[i] == true){
                next_turn = i;
                break;
              }
            }
          }
        }
        if (next_turn > 0) {
          can_send_control_turn(node_id, next_turn);
          sv_cycle_counter++;
        }
        else {
          Serial.println("No valid next turn found");
        }
        sv_my_turn = false;
      }

    } else {
      // ========================= DUAL DECOMPOSITION ALGORITHM ========================= // 
      // Failing unfortunately ;-; too many msgs maybe? 
      if (sv_my_turn == true) {

        // find next victim (I mean, node :D to pass turn)
        int  next_turn = 0;
        bool is_last   = false;
        if (nodes_list != nullptr) {
          for (int i = node_id + 1; i <= num_nodes; i++) {
            if (nodes_list[i]) { next_turn = i; break; }
          }
          if (next_turn == 0) {
            is_last = true;
            for (int i = 1; i <= num_nodes; i++) {
              if (nodes_list[i]) { next_turn = i; break; }
            }
          }
        }

        if (sv_dual_phase == 0) {
          // primal update -> get u(k+1) with lambda(k)
          float u_new = dual_compute_u(node_id, energy_cost, P_max,
                                       sv_lambda_shared, Gs, num_nodes);
          // averaging over successive iterates improves convergence
          // No idea why it works, but I trust the slides :D
          u_new = 0.5f * u_new + 0.5f * sv_duty_cycle;

          sv_duty_cycle            = u_new;
          sv_duty_cycle_shared[node_id] = u_new;
          can_send_conv_duty(node_id, u_new);   // reuse 'X' broadcast

          if (u_hist_col[node_id] < MAX_CONV_HIST) {
            u_hist_matrix[UHIDX(node_id, u_hist_col[node_id]++)] = u_new;
          }
          delay(5); // DO NOT SPAMMMM

          // last node in primal flips to dual for the next round
          if (next_turn > 0)
            can_send_dual_turn(node_id, next_turn, is_last ? 1 : 0);
          else
            Serial.println("Dual: no valid next turn (u-phase)");
            // if this happens, I am screwed

        } else {
          // dual update 
          float lam = dual_compute_lambda(node_id,
                                          sv_lambda_shared[node_id],
                                          sv_r, d_terms[node_id],
                                          sv_duty_cycle_shared, Gs, num_nodes);
          sv_lambda_shared[node_id] = lam;
          can_send_lambda(node_id, lam);
          delay(5);

          if (is_last) { // IF end of iter K
            sv_dual_iter++;
            Serial.print("Dual iter: ");
            Serial.println(sv_dual_iter);

            if (sv_dual_iter >= DUAL_MAX_ITER) {
              sv_converged = true;
              u_ideal_cvg  = sv_duty_cycle;
              PID_LOCK();
              my_pid.bumpless_transfer(my_pid.get_kp(), my_pid.get_ki(),
                                       my_pid.get_b(), sv_r, sv_y_lux);
              PID_UNLOCK();
              Serial.println("Dual decomp reached max iteration:");
            } else {
              // restart from u-phase
              can_send_dual_turn(node_id, next_turn, 0);
            }
          } else {
            if (next_turn > 0)
              can_send_dual_turn(node_id, next_turn, 1);
            else
              Serial.println("Dual: no valid next turn (lambda-phase)");
          }
        }

        sv_my_turn = false;
      }
    }
  }

  // ==================================
  // PID ACTION 
  // ==================================

  else if(sv_converged == true){
    //sv_my_turn = false;

    float local_r = sv_r;
    float local_lux = sv_y_lux;
    bool  local_fb  = sv_feedback_on;
    float local_dc  = sv_duty_cycle;
    float total_dist = 0.0;

    unsigned long cur_time = micros(); 
    if (cur_time - prev_time_pid < (unsigned long)(h * 1000000UL)) return; // ERRADO; != MS FORA!!!!
    //Serial.println("PID");
    unsigned long dt = cur_time - prev_time_pid;
    prev_time_pid = cur_time;

    // Record period jitter 
    const unsigned long h_us = (unsigned long)(h * 1000000UL);
    unsigned long _jitter;

    if (dt > h_us) {
        _jitter = dt - h_us;
    } else {
        _jitter = h_us - dt;
    }

    prof_pid_jitter.sum_us += _jitter;
    if (_jitter > prof_pid_jitter.max_us) prof_pid_jitter.max_us = _jitter;
    prof_pid_jitter.count++;

    float u_total = 0.0;
    unsigned long _t0 = micros();
    if(local_fb == true){
      // DEBUG: Show inputs to compute_control

      PID_LOCK();
      float u_fb = my_pid.compute_control(local_r, local_lux);
      float v = u_fb + u_ideal_cvg*DAC_RANGE;
      u_total = my_pid.saturation(v); //PWM
      my_pid.housekeep(local_r, local_lux, v, u_total, anti_windup);
      PID_UNLOCK();

      set_led('p', u_total);
    }
    else{
      set_led('u', local_dc); // use sv_duty_cycle so manual 'u' command takes effect
      float v = local_dc*DAC_RANGE;
      u_total = v;
      PID_LOCK();
      my_pid.housekeep(local_r, local_lux, v, u_total, anti_windup);
      PID_UNLOCK();
    }
    unsigned long _pid_dt = micros() - _t0;
    prof_pid_compute.sum_us += _pid_dt;
    if (_pid_dt > prof_pid_compute.max_us) prof_pid_compute.max_us = _pid_dt;
    prof_pid_compute.count++;

    // =============================================
    // Performance metrics 
    // =============================================

    float cur_duty = u_total / DAC_RANGE;
    //Serial.print("cur_duty: "); Serial.println(cur_duty);
    float expected_lux   = Gs[GIDX(node_id, node_id)] * cur_duty + d_terms[node_id];

    for (int i=1;i<=num_nodes;i++){
      if(i!=node_id && nodes_list[i]==true){
        expected_lux+=Gs[GIDX(i, node_id)]*sv_duty_cycle_shared[i];
      }
    }

    total_n += 1;
    total_energy += energy(prev_duty, dt);
    sum_visibility += visibility(local_r, expected_lux);
    total_visibility = sum_visibility / total_n;

    if (transient_count >= 150){
      sum_flicker += flicker(cur_duty, prev_duty, prev2_duty);
      flicker_n += 1;
      total_flicker = sum_flicker / (flicker_n * h);
    } 
    else{
      transient_count++;
    }

    prev2_duty = prev_duty;
    prev_duty  = cur_duty;
    sv_duty_cycle = cur_duty;

    static unsigned long last_duty_broadcast = 0;
    if (cur_time - last_duty_broadcast > 200000) {  // every 200ms update neighbours with your current duty cycle
      can_send_duty(node_id, cur_duty);
      last_duty_broadcast = cur_time;
    }
    
  }
}
  

// ---------------- CALIBRATION SEQUENCE (runs on core 0, called from loop()) -----------------

void run_calibration(unsigned long cur_time){

  // State 0 - wait 15 s for all nodes to wake up, elect lowest-ID as first boss
  // State 1 - Boss broadcasts 'c' (stop all PIDs/LEDs) then advances to state 2
  // State 2 - Boss stabilizes dark then sets duty = 1
  // State 3 - Boss holds LED on for 4 s, then snapshots self + broadcasts 's' 
  // State 5 - Boss duty=0 ; waits 300 ms, then hands over to next boss
  // State 6 - All LEDs off — wait 2 s for dark, boss sends 'f' + 'S'=7 to finish
  // State 7 - Every node computes net gains (subtract dark term) and prints the result
  // State 8 - Normal PID control (calib_st >= 8 exits run_calibration entirely)

  if (calib_st == 0) {
  static unsigned long last_w_time = 0;

  if (cur_time - last_w_time >= 1000000) {
    can_send_wakeup(my_uid);
    last_w_time = cur_time;

    if (uid_count == REGISTERED_MCU && skip_req == false) {
      nodes_ack++;
      delay(5);
      can_skip_calib(100); // beacuse ID's not defined yet!!!! -> 100 is random val to fill func  
      skip_req = true;
      
    }
  }

    if (cur_time - cal_time >= 10000000 || (nodes_ack >= REGISTERED_MCU)) { // WAIT 10 s or skip if max nodes found 
      sort_uids();
      allocate_network_arrays();

      node_id = node_id_from_uid(my_uid);
      boss_node = 1;

      Serial.print("num_nodes = ");
      Serial.println(num_nodes);
      Serial.print("my node_id = ");
      Serial.println(node_id);
      Serial.print("nodes_ack = ");
      Serial.println(nodes_ack);

      cal_time = cur_time;
      calib_st = 1;
    }
  }

  else if (calib_st == 1){ 
    if (node_id == boss_node){
      can_send_calibration_start(boss_node); // 'c' tells others to dim
      set_led('u', 0.0);
      feedback_on = false;
      duty_cycle = 0.0;
      delay(10);
      cal_time = cur_time;
      calib_st = 2;
      can_change_state(boss_node, 2);
    }
    // others wait here for 'S'=2 via CAN.
  }

  else if (calib_st == 2){ 
    if (cur_time - cal_time >= 4000000){
      if (node_id == boss_node){
        duty_cycle = 1.0;
        set_led('u', 1.0);
        
        cal_time = cur_time;
        calib_st = 3;
        can_change_state(boss_node, 3);
      }
    }
  }

  else if (calib_st == 3) { 
    if (cur_time - cal_time >= 2000000) {
      if (node_id == boss_node) {
        Gs[GIDX(boss_node, boss_node)] = (float)sv_y_lux;
        can_store_lux_value(boss_node); // 's' others snapshot their y_lux now

        cal_time = cur_time;
        calib_st = 5;
        can_change_state(boss_node, 5);
      }
    }
  }

  // --- STATE 4 was deleted xD -> not useful 

  else if (calib_st == 5) { 
    if (node_id == boss_node) {


      //  300 ms stabilisation window to make sure I dont skip stages
      // delay the MCP2515 RX buffers overflow 
      if (cur_time - cal_time < 300000) return;

      duty_cycle = 0.0; 
      set_led('u', 0.0); 
      Serial.println("Change boss...");
    
      bool found_next = false; 
      int next_boss = 0; 

      for (int i = boss_node + 1; i <= num_nodes; i++) { 
        if (nodes_list[i] == true) {
          next_boss = i; 
          found_next = true; 
          break; 
        }
      }

      if (found_next) { 
        boss_node = next_boss; 
        cal_time = cur_time; 
        calib_st = 1;
        can_change_state(boss_node, 1); // src field carries the NEW boss ID
      } else {
        cal_time = cur_time; 
        calib_st = 6; 
        can_change_state(boss_node, 6); 
      }
    }
  }

  else if (calib_st == 6) { 
    if (cur_time - cal_time >= 4000000) { 
      if (node_id == boss_node) {
        d_terms[node_id] = (float)sv_y_lux;
        
        // 'f' sets calib_st=7 on others
        can_send_calibration_end(node_id); 
        delay(10);
        feedback_on = true; 
        calib_st = 7;
        can_change_state(boss_node, 7);
      }
    }
  }

  else if (calib_st == 7 && printed_gains == false) { 
    Serial.println("Calculating local true Gains...");

    for (int b = 1; b <= num_nodes; b++) {
      if (!nodes_list[b]) continue;
      Gs[GIDX(b, node_id)] = Gs[GIDX(b, node_id)] - d_terms[node_id];
      if (Gs[GIDX(b, node_id)] < 0.0) Gs[GIDX(b, node_id)] = 0.0;
    }

    Serial.println("Calibration Math Complete!");

    // Print only the column relevant to THIS node
    Serial.print("Node ");
    Serial.print(node_id);
    Serial.println(" — Local Gs Column (Sensor specific):");

    for (int row = 1; row <= num_nodes; row++) {
      if (!nodes_list[row]) continue; // Skip if this boss node doesn't exist
      
      Serial.print("  Boss ");
      Serial.print(row);
      Serial.print(": Gs[");
      Serial.print(row);
      Serial.print("][");
      Serial.print(node_id);
      Serial.print("] = ");
      // We use node_id as the fixed column index
      Serial.println(Gs[GIDX(row, node_id)], 4); 
    }

    Serial.print("  d_term[");
    Serial.print(node_id);
    Serial.print("] = ");
    Serial.println(d_terms[node_id], 4);

    // to avoid printing multiple stuff while waiting for can message in non boss nodes 
    printed_gains = true; 

    if (node_id == boss_node){
      //Serial.println("initializing control");
      //can_send_control_turn(boss_node, 1); //assim o node 1 é o primeiro
      sv_my_turn = true; // assim o node 3 é o primeiro
      can_change_state(boss_node, 8);
      calib_st = 8;
    }

  }  
}