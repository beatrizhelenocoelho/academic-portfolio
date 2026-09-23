#include "can_bus.h"
#include "utils.h"
#include "shared.h"
#include "constants.h"
#include <string.h>

// Variable declaration for CAN COMMUNICATIONS
extern void add_uid_if_new(uint32_t uid);
extern float *u_hist_matrix;
extern int   *u_hist_col;
extern int    num_nodes;
extern int nodes_ack;
extern unsigned long init_time;

// INIT THE CAN (INIT PIN is on 20)
MCP2515 can0(spi0, 17, 19, 16, 18, 1000000);

// BINARY ENCODING -> at least I tried 
static void pack_float(uint8_t *buf, float value) {
  memcpy(buf, &value, sizeof(float));
}

static float unpack_float(uint8_t *buf) {
  float value;
  memcpy(&value, buf, sizeof(float));
  return value;
}

static void pack_u32(uint8_t *buf, uint32_t value) {
  memcpy(buf, &value, sizeof(uint32_t));
}

static uint32_t unpack_u32(uint8_t *buf) {
  uint32_t value;
  memcpy(&value, buf, sizeof(uint32_t));
  return value;
}

// SETUP
void can_setup() {
  can0.reset();
  can0.setBitrate(CAN_1000KBPS);
  can0.setNormalMode();
}

// ================== MESSAGES ================== // 
// All messages that we can send from one node to 
// others are here -> kinda messy but works 
// ============================================== // 

void can_send_test(uint8_t src, uint8_t dest) {
  struct can_frame tx;
  tx.can_id = 0x100 + dest;
  tx.can_dlc = 3;
  tx.data[0] = 'T';
  tx.data[1] = src;
  tx.data[2] = dest;

  CAN_LOCK();
  can0.sendMessage(&tx);
  CAN_UNLOCK();
}

void can_send_ref(uint8_t src, uint8_t dest, float ref) {
  struct can_frame tx;
  tx.can_id = 0x100 + dest;
  tx.can_dlc = 7;
  tx.data[0] = 'r';
  tx.data[1] = src;
  tx.data[2] = dest;
  pack_float(&tx.data[3], ref);

  CAN_LOCK();
  can0.sendMessage(&tx);
  CAN_UNLOCK();
}

void can_send_lux_request(uint8_t src, uint8_t dest) {
  struct can_frame tx;
  tx.can_id = 0x100 + dest;
  tx.can_dlc = 3;
  tx.data[0] = 'l';
  tx.data[1] = src;
  tx.data[2] = dest;

  CAN_LOCK();
  can0.sendMessage(&tx);
  CAN_UNLOCK();
}

void can_send_k(uint8_t src, uint8_t dest, float kp, float bpid, float ki) {
  struct can_frame tx;
  uint32_t _irq_save;

  tx.can_id = 0x100 + dest;
  tx.can_dlc = 7;
  tx.data[0] = 'p';
  tx.data[1] = src;
  tx.data[2] = dest;
  pack_float(&tx.data[3], kp);

  _irq_save = spin_lock_blocking(can_lock);
  can0.sendMessage(&tx);
  spin_unlock(can_lock, _irq_save);

  delay(2);

  tx.can_id = 0x100 + dest;
  tx.can_dlc = 7;
  tx.data[0] = 'b';
  tx.data[1] = src;
  tx.data[2] = dest;
  pack_float(&tx.data[3], bpid);
  _irq_save = spin_lock_blocking(can_lock);
  can0.sendMessage(&tx);
  spin_unlock(can_lock, _irq_save);

  delay(2);

  tx.can_id = 0x100 + dest;
  tx.can_dlc = 7;
  tx.data[0] = 'i';
  tx.data[1] = src;
  tx.data[2] = dest;
  pack_float(&tx.data[3], ki);
  _irq_save = spin_lock_blocking(can_lock);
  can0.sendMessage(&tx);
  spin_unlock(can_lock, _irq_save);
}

void can_send_ack(uint8_t src, uint8_t dest) {
  struct can_frame tx;
  tx.can_id = 0x100 + dest;
  tx.can_dlc = 3;
  tx.data[0] = 'a';
  tx.data[1] = src;
  tx.data[2] = dest;
  CAN_LOCK();
  can0.sendMessage(&tx);
  CAN_UNLOCK();
}

void can_change_state(uint8_t src, uint8_t state) {
  struct can_frame tx;
  tx.can_id = 0x100;
  tx.can_dlc = 4;
  tx.data[0] = 'S';
  tx.data[1] = src;
  tx.data[2] = 0;
  tx.data[3] = state;
  CAN_LOCK();
  can0.sendMessage(&tx);
  CAN_UNLOCK();
}

void can_send_lux_value(uint8_t src, uint8_t dest, float lux) {
  struct can_frame tx;
  tx.can_id = 0x100 + dest;
  tx.can_dlc = 7;
  tx.data[0] = 'y';
  tx.data[1] = src;
  tx.data[2] = dest;
  pack_float(&tx.data[3], lux);
  CAN_LOCK();
  can0.sendMessage(&tx);
  CAN_UNLOCK();
}

void can_store_lux_value(uint8_t src) {
  struct can_frame tx;
  tx.can_id = 0x100;
  tx.can_dlc = 3;
  tx.data[0] = 's';
  tx.data[1] = src;
  tx.data[2] = 0;
  CAN_LOCK();
  can0.sendMessage(&tx);
  CAN_UNLOCK();
}

void can_send_wakeup(uint32_t uid) { // Calibration
  struct can_frame tx;
  tx.can_id = 0x100;
  tx.can_dlc = 5;
  tx.data[0] = 'w';
  pack_u32(&tx.data[1], uid);

  CAN_LOCK();
  can0.sendMessage(&tx);
  CAN_UNLOCK();
}

void can_send_calibration_start(uint8_t src) {
  struct can_frame tx;
  tx.can_id = 0x100;
  tx.can_dlc = 3;
  tx.data[0] = 'c';
  tx.data[1] = src;
  tx.data[2] = 0;

  CAN_LOCK();
  can0.sendMessage(&tx);
  CAN_UNLOCK();
}

void can_send_calibration_end(uint8_t src) {
  struct can_frame tx;
  tx.can_id = 0x100;
  tx.can_dlc = 3;
  tx.data[0] = 'f';
  tx.data[1] = src;
  tx.data[2] = 0;

  CAN_LOCK();
  can0.sendMessage(&tx);
  CAN_UNLOCK();
}

void can_send_duty(uint8_t src, float duty) {
  struct can_frame tx;
  tx.can_id = 0x100;
  tx.can_dlc = 7;
  tx.data[0] = 'U';
  tx.data[1] = src;
  tx.data[2] = 0;
  pack_float(&tx.data[3], duty);

  CAN_LOCK();
  can0.sendMessage(&tx);
  CAN_UNLOCK();
}

void can_send_conv_duty(uint8_t src, float duty) {
  struct can_frame tx;
  tx.can_id = 0x100;          // broadcast, same as can_send_duty
  tx.can_dlc = 7;
  tx.data[0] = 'X';           // exclusive for convergence 
  tx.data[1] = src;
  tx.data[2] = 0;
  pack_float(&tx.data[3], duty);

  CAN_LOCK();
  can0.sendMessage(&tx);
  CAN_UNLOCK();
}

void can_send_get(uint8_t src, uint8_t dest, char what) {
  struct can_frame tx;
  tx.can_id = 0x100 + dest;
  tx.can_dlc = 4;
  tx.data[0] = 'g';
  tx.data[1] = src;
  tx.data[2] = dest;
  tx.data[3] = (uint8_t)what;
  CAN_LOCK();
  can0.sendMessage(&tx);
  CAN_UNLOCK();
}

void can_send_set_scalar(uint8_t src, uint8_t dest, char type, float val) {
  struct can_frame tx;
  tx.can_id = 0x100 + dest;
  tx.can_dlc = 7;
  tx.data[0] = (uint8_t)type;
  tx.data[1] = src;
  tx.data[2] = dest;
  pack_float(&tx.data[3], val);
  CAN_LOCK();
  can0.sendMessage(&tx);
  CAN_UNLOCK();
}

void can_send_set_duty(uint8_t src, uint8_t dest, float duty) {
  struct can_frame tx;
  tx.can_id = 0x100 + dest;
  tx.can_dlc = 7;
  tx.data[0] = 'd';
  tx.data[1] = src;
  tx.data[2] = dest;
  pack_float(&tx.data[3], duty);
  CAN_LOCK();
  can0.sendMessage(&tx);
  CAN_UNLOCK();
}

void can_send_set_bool(uint8_t src, uint8_t dest, char type, bool on) {
  struct can_frame tx;
  tx.can_id = 0x100 + dest;
  tx.can_dlc = 4;
  tx.data[0] = (uint8_t)type;   // 'F' ou 'A'
  tx.data[1] = src;
  tx.data[2] = dest;
  tx.data[3] = on ? 1 : 0;
  CAN_LOCK();
  can0.sendMessage(&tx);
  CAN_UNLOCK();
}

void can_send_restart(uint8_t src, uint8_t dest) {
  struct can_frame tx;
  tx.can_id = (dest == 0) ? 0x100 : (0x100 + dest);
  tx.can_dlc = 3;
  tx.data[0] = 'R';
  tx.data[1] = src;
  tx.data[2] = dest;
  CAN_LOCK();
  can0.sendMessage(&tx);
  CAN_UNLOCK();
}

void can_send_value_reply(uint8_t src, uint8_t dest, char what, float val) {
  struct can_frame tx;
  tx.can_id = 0x100 + dest;
  tx.can_dlc = 8;
  tx.data[0] = 'v';
  tx.data[1] = src;
  tx.data[2] = dest;
  tx.data[3] = (uint8_t)what;
  pack_float(&tx.data[4], val);
  CAN_LOCK();
  can0.sendMessage(&tx);
  CAN_UNLOCK();
}

void can_send_status_reply(uint8_t src, uint8_t dest, char what, bool ok) {
  struct can_frame tx;
  tx.can_id = 0x100 + dest;
  tx.can_dlc = 5;
  tx.data[0] = 'm';
  tx.data[1] = src;
  tx.data[2] = dest;
  tx.data[3] = (uint8_t)what;
  tx.data[4] = ok ? 1 : 0;
  CAN_LOCK();
  can0.sendMessage(&tx);
  CAN_UNLOCK();
}

void can_send_control_turn(uint8_t src, uint8_t dest){ //NOVO ASS MARIA
  struct can_frame tx;
  tx.can_id = 0x100+dest;
  tx.can_dlc =3;
  tx.data[0] = 't';
  tx.data[1] =src;
  tx.data[2]=dest;
  CAN_LOCK();
  can0.sendMessage(&tx);
  CAN_UNLOCK();
}

void can_send_changed_ref(uint8_t src){
  struct can_frame tx;
  tx.can_id = 0x100; // 0x100 means Broadcast to everyone
  tx.can_dlc = 3;
  tx.data[0] = 'M';
  tx.data[1] = src;
  tx.data[2] = 0;
  CAN_LOCK();
  can0.sendMessage(&tx);
  CAN_UNLOCK();
}

// ================== CAN ERRORS ================== // 

void can_check_errors(int node_id) {
  CAN_LOCK();
  uint8_t tec    = can0.errorCountTX();
  uint8_t rec    = can0.errorCountRX();
  uint8_t eflags = can0.getErrorFlags();
  CAN_UNLOCK();

  // Only print if there is something to report
  if (tec == 0 && rec == 0 && eflags == 0) return;

  Serial.print("[CAN ERR node ");
  Serial.print(node_id);
  Serial.print("] TEC=");
  Serial.print(tec);
  Serial.print(" REC=");
  Serial.println(rec);

  if (eflags & EFLG_TXBO)  Serial.println("  ERROR: Bus-Off");
  if (eflags & EFLG_TXEP)  Serial.println("  ERROR: TX Error-Passive");
  if (eflags & EFLG_RXEP)  Serial.println("  ERROR: RX Error-Passive");
  if (eflags & EFLG_TXWAR) Serial.println("  WARNING: TX Error Warning (TEC>=96)");
  if (eflags & EFLG_RXWAR) Serial.println("  WARNING: RX Error Warning (REC>=96)");
  if (eflags & EFLG_EWARN) Serial.println("  WARNING: Error Warning flag set");

  if (eflags & (EFLG_RX0OVR | EFLG_RX1OVR)) {
    if (eflags & EFLG_RX0OVR) Serial.println("  ERROR: RX Buffer 0 Overflow");
    if (eflags & EFLG_RX1OVR) Serial.println("  ERROR: RX Buffer 1 Overflow");
    // Must clear overflow flags so the chip can accept new messages
    CAN_LOCK();
    can0.clearRXnOVRFlags();
    CAN_UNLOCK();
  }
}

// ================== DUAL DECOMPOSITION ================== // 
// EXTRA -> Not finished fixing 

void can_send_lambda(uint8_t src, float lambda_val) {
  struct can_frame tx;
  tx.can_id = 0x100;          // broadcast
  tx.can_dlc = 7;
  tx.data[0] = 'P';
  tx.data[1] = src;
  tx.data[2] = 0;
  pack_float(&tx.data[3], lambda_val);
  CAN_LOCK();
  can0.sendMessage(&tx);
  CAN_UNLOCK();
}

void can_send_dual_turn(uint8_t src, uint8_t dest, uint8_t phase) {
  struct can_frame tx;
  tx.can_id = 0x100 + dest;   // unicast to next node
  tx.can_dlc = 4;
  tx.data[0] = 'D';
  tx.data[1] = src;
  tx.data[2] = dest;
  tx.data[3] = phase;         // 0 = u-phase, 1 = lambda-phase
  CAN_LOCK();
  can0.sendMessage(&tx);
  CAN_UNLOCK();
}

void can_send_algo_switch(uint8_t src, bool use_dual) {
  struct can_frame tx;
  tx.can_id = 0x100;          // broadcast
  tx.can_dlc = 4;
  tx.data[0] = 'J';
  tx.data[1] = src;
  tx.data[2] = 0;
  tx.data[3] = use_dual ? 1 : 0;
  CAN_LOCK();
  can0.sendMessage(&tx);
  CAN_UNLOCK();
}

void can_skip_calib(uint8_t src){ // send a ping to inform others that it's showtime
  struct can_frame tx;
  tx.can_id = 0x100; // 0x100 means Broadcast to everyone
  tx.can_dlc = 3;
  tx.data[0] = 'z';
  tx.data[1] = src;
  tx.data[2] = 0;
  CAN_LOCK();
  can0.sendMessage(&tx);
  CAN_UNLOCK();
}

// ================== MESSAGE HANDLER ================== // 
// All messages that can be processed sit here!
// ===================================================== // 


void can_handle_messages(int node_id, int num_nodes, uint32_t *uids, int &uid_count,
                         float &r, float y_lux, float v_measured, pid &my_pid, bool *nodes_list,
                         bool &anti_windup, bool &feedback_on, float &duty_cycle, float *Gs,
                         int &lux_replies, int &calib_st, int &boss_node, float *d_terms,
                         float total_energy, float total_visibility, float total_flicker,
                         float &high_ref, float &low_ref, float &energy_cost,
                         bool &restart_requested) {

  (void)uids;
  (void)uid_count;
  (void)nodes_list;
  (void)lux_replies;

  struct can_frame rx;
  
  while (true) {

    //  Lock ->read the hardware and unlock
    CAN_LOCK();
    MCP2515::ERROR err = can0.readMessage(&rx);
    CAN_UNLOCK();

    // If no message, break 
    if (err != MCP2515::ERROR_OK) break;

    // Process the message safely
    char type = (char)rx.data[0];

    if (type == 'w') {
      uint32_t uid = unpack_u32(&rx.data[1]);
      add_uid_if_new(uid);
      continue;
    }
    else if (type == 'z'){
      nodes_ack++; // ackonledge other guys are ready 
      continue;
    }

    uint8_t src = rx.data[1];
    uint8_t dest = rx.data[2];

    if (dest != node_id && dest != 0) continue;

    if (type == 'T') {
      Serial.print("RX test from node ");
      Serial.println(src);
    }

    else if (type == 'r') { //CHANGED ASS MARIA
      float new_ref = unpack_float(&rx.data[3]);
      r = new_ref;
      sv_r = new_ref; // Shared variable also
      feedback_on = true;
      sv_converged = false;
      sv_cycle_counter = 0;
      memset(u_hist_col, 0, sizeof(int) * (num_nodes + 1));
      //can_send_ack(node_id, src);
      Serial.print("Received");
      Serial.println(r);
      Serial.print(" src = ");
      Serial.println(src);

      // GET sv_converged false t oeveryone 
      //delayMicroseconds(200); // REMOVE AFTER PLSSSSS
      //can_send_changed_ref(src);
    }

    else if (type == 'l') {
      can_send_lux_value(node_id, src, y_lux);
    }

    else if (type == 'y') {
      float lux = unpack_float(&rx.data[3]);
      Serial.print("Measured LUX: ");
      Serial.println(lux, 4);
    }

    else if (type == 'p') {
      float new_kp = unpack_float(&rx.data[3]);
      PID_LOCK();
      my_pid.bumpless_transfer(new_kp, my_pid.get_ki(), my_pid.get_b(), r, y_lux);
      PID_UNLOCK();
      can_send_ack(node_id, src);
    }

    else if (type == 'b') {
      float new_b = unpack_float(&rx.data[3]);
      PID_LOCK();
      my_pid.bumpless_transfer(my_pid.get_kp(), my_pid.get_ki(), new_b, r, y_lux);
      PID_UNLOCK();
      can_send_ack(node_id, src);
    }

    else if (type == 'i') {
      float new_ki = unpack_float(&rx.data[3]);
      PID_LOCK();
      my_pid.bumpless_transfer(my_pid.get_kp(), new_ki, my_pid.get_b(), r, y_lux);
      PID_UNLOCK();
      can_send_ack(node_id, src);
    }

    else if (type == 'a') {
      Serial.print("ACK from node ");
      Serial.println(src);
    }

    // -------- NOVOS/ANTIGOS COMANDOS REMOTOS --------

    else if (type == 'd') {
      float new_duty = unpack_float(&rx.data[3]);
      if (new_duty >= 0.0f && new_duty <= 1.0f) {
        duty_cycle = new_duty;
        feedback_on = false;
        can_send_status_reply(node_id, src, 'd', true);
      } else {
        can_send_status_reply(node_id, src, 'd', false);
      }
    }

    else if (type == 'F') {
      uint8_t on = rx.data[3];
      if (on == 0 || on == 1) {
        feedback_on = (on == 1);
        can_send_status_reply(node_id, src, 'F', true);
      } else {
        can_send_status_reply(node_id, src, 'F', false);
      }
    }

    else if (type == 'A') {
      uint8_t on = rx.data[3];
      if (on == 0 || on == 1) {
        anti_windup = (on == 1);
        can_send_status_reply(node_id, src, 'A', true);
      } else {
        can_send_status_reply(node_id, src, 'A', false);
      }
    }

    else if (type == 'g') {
      char what = (char)rx.data[3];

      // I squished this part with Gemini to save space 
      // Table 3
      if      (what == 'O') can_send_value_reply(node_id, src, 'O', high_ref);
      else if (what == 'U') can_send_value_reply(node_id, src, 'U', low_ref);
      else if (what == 'L') can_send_value_reply(node_id, src, 'L', r);
      else if (what == 'C') can_send_value_reply(node_id, src, 'C', energy_cost);
      // Table 2
      else if (what == 'E') can_send_value_reply(node_id, src, 'E', total_energy);
      else if (what == 'V') can_send_value_reply(node_id, src, 'V', total_visibility);
      else if (what == 'F') can_send_value_reply(node_id, src, 'F', total_flicker);
      // Table 1 - duty, reference, lux, voltage, anti-windup, feedback, disturbance, power, time, occupancy
      else if (what == 'u') can_send_value_reply(node_id, src, 'u', duty_cycle);
      else if (what == 'r') can_send_value_reply(node_id, src, 'r', r);
      else if (what == 'y') can_send_value_reply(node_id, src, 'y', y_lux);
      else if (what == 'v') can_send_value_reply(node_id, src, 'v', v_measured);
      else if (what == 'a') can_send_value_reply(node_id, src, 'a', anti_windup ? 1.0f : 0.0f);
      else if (what == 'f') can_send_value_reply(node_id, src, 'f', feedback_on ? 1.0f : 0.0f);
      else if (what == 'd') can_send_value_reply(node_id, src, 'd', d_terms[node_id]);
      else if (what == 'p') can_send_value_reply(node_id, src, 'p', duty_cycle * P_max);
      else if (what == 't') can_send_value_reply(node_id, src, 't', (millis() - init_time) / 1000.0f);
      else if (what == 'o') {
        // derive occupancy: 0=off, 1=low, 2=high
        float occ = !feedback_on ? 0.0f : (r >= high_ref ? 2.0f : 1.0f);
        can_send_value_reply(node_id, src, 'o', occ);
      }
      // legacy internal sub-commands (kept for backward compat)
      else if (what == 'T') can_send_value_reply(node_id, src, 'T', r);
      else if (what == 'H') can_send_value_reply(node_id, src, 'H', anti_windup ? 1.0f : 0.0f);
      else if (what == 'B') can_send_value_reply(node_id, src, 'B', feedback_on ? 1.0f : 0.0f);
      else                  can_send_status_reply(node_id, src, what, false);
    }

    else if (type == 'O') {
      float val = unpack_float(&rx.data[3]);
      if (val >= 0.0f) {
        high_ref = val;
        can_send_status_reply(node_id, src, 'O', true);
      } else {
        can_send_status_reply(node_id, src, 'O', false);
      }
    }

    else if (type == 'W') {
      float val = unpack_float(&rx.data[3]);
      if (val >= 0.0f) {
        low_ref = val;
        can_send_status_reply(node_id, src, 'U', true);
      } else {
        can_send_status_reply(node_id, src, 'U', false);
      }
    }

    else if (type == 'C') {
      float val = unpack_float(&rx.data[3]);
      if (val >= 0.0f) {
        energy_cost = val;
        can_send_status_reply(node_id, src, 'C', true);
      } else {
        can_send_status_reply(node_id, src, 'C', false);
      }
    }

    else if (type == 'R') {
      restart_requested = true;
      if (dest != 0) {
        can_send_status_reply(node_id, src, 'R', true);
      }
    }

    // 'N' — occupancy SET: 0=off, 1=low, 2=high
    else if (type == 'N') {
      int occ = (int)roundf(unpack_float(&rx.data[3]));
      if (occ == 0) {
        feedback_on = false;
        can_send_status_reply(node_id, src, 'N', true);
      } else if (occ == 1 || occ == 2) {
        float new_ref = (occ == 1) ? low_ref : high_ref;
        r = new_ref;
        sv_r = new_ref;
        feedback_on = true;
        sv_converged = false;
        sv_cycle_counter = 0;
        memset(u_hist_col, 0, sizeof(int) * (num_nodes + 1));
        can_send_status_reply(node_id, src, 'N', true);
      } else {
        can_send_status_reply(node_id, src, 'N', false);
      }
    }

    else if (type == 'v') {
      char what = (char)rx.data[3];
      float val = unpack_float(&rx.data[4]);

      if (what == 'o') {
        // occupancy reply: 0.0=off, 1.0=low, 2.0=high
        Serial.print("o ");
        Serial.print(src);
        Serial.print(" ");
        if      (val < 0.5f) Serial.println("o");
        else if (val < 1.5f) Serial.println("l");
        else                 Serial.println("h");
      }
      else if (what == 'O' || what == 'U' || what == 'L' || what == 'C') {
        Serial.print(what);
        Serial.print(" ");
        Serial.print(src);
        Serial.print(" ");
        Serial.println(val, 4);
      }
      else if (what == 'E') {
        Serial.print("Total Energy: ");
        Serial.println(val, 4);
      }
      else if (what == 'V') {
        Serial.print("Visibility Error: ");
        Serial.println(val, 4);
      }
      else if (what == 'Q') {
        Serial.print("Flicker: ");
        Serial.println(val, 4);
      }
      else if (what == 'T') {
        Serial.print("Current reference: ");
        Serial.println(val, 4);
      }
      else if (what == 'H') {
        if (val >= 0.5f) Serial.println("Anti-windup ON");
        else             Serial.println("Anti-windup OFF");
      }
      else if (what == 'B') {
        if (val >= 0.5f) Serial.println("Feedback ON");
        else             Serial.println("Feedback OFF");
      }
      else {
        Serial.print(what);
        Serial.print(" ");
        Serial.print(src);
        Serial.print(" ");
        Serial.println(val, 4);
      }
    }

    else if (type == 'm') {
      bool ok = (rx.data[4] != 0);
      if (ok) Serial.println("ack");
      else    Serial.println("err");
    }

    // -------- CALIBRAÇÃO / FEEDFORWARD --------

    else if (type == 'S') {
      calib_st = rx.data[3];
      boss_node = src;
    }

    else if (type == 's') {
      uint8_t current_boss = rx.data[1];
      Gs[current_boss * (num_nodes + 1) + node_id] = y_lux;
      Serial.print("Recorded cross-coupling light from Boss ");
      Serial.println(current_boss);
    }

    else if (type == 'c') {
      //Serial.println("Calibration cmd received. Turning LED off");
      feedback_on = false;
      duty_cycle = 0.0f;
      set_led('u', 0.0);
    }

    else if (type == 'f') {
      d_terms[node_id] = y_lux;
      calib_st = 7;
      feedback_on = true;
      Serial.println("Calibration Finished. Resuming normal control.");
    }

    else if (type == 'U') {
      float new_duty = unpack_float(&rx.data[3]);
      sv_duty_cycle_shared[src] = new_duty;
      sv_duty_shared[src] = true;
      sv_update_u_cv = true;
    }

    else if (type == 'X') {
      float new_duty = unpack_float(&rx.data[3]);
      sv_duty_cycle_shared[src] = new_duty;
      sv_duty_shared[src] = true;
      sv_update_u_cv = true;

      if (u_hist_col[src] < MAX_CONV_HIST) {
          u_hist_matrix[src * MAX_CONV_HIST + u_hist_col[src]++] = new_duty;
      }
    }

    else if (type == 't'){
      sv_my_turn = true;
      //Serial.print("Received control turn from node ");
      //Serial.println(src);
    }

    else if (type == 'M'){
      sv_converged = false;
      //sv_my_turn = false; //REMOVED
      sv_cycle_counter = 0;
      memset(u_hist_col, 0, sizeof(int) * (num_nodes + 1));
      Serial.println("sv_converged = false");
      // reset dual decomp state so whichever algorithm runs starts fresh
      sv_dual_iter  = 0;
      sv_dual_phase = 0;
      if (sv_lambda_shared != nullptr) {
        for (int i = 0; i <= num_nodes; i++) sv_lambda_shared[i] = 0.0f;
      }
    }

    // ── Dual Decomposition message handlers ──────────────────────────

    else if (type == 'P') {
      // Lambda broadcast: store sender's lambda value
      sv_lambda_shared[src] = unpack_float(&rx.data[3]);
    }

    else if (type == 'D') {
      // Dual token: update phase and signal this node's turn
      sv_dual_phase = rx.data[3];
      sv_my_turn    = true;
    }

    else if (type == 'J') {
      // Algorithm switch broadcast: adopt the new algorithm and reset state
      use_dual_decomp = (rx.data[3] != 0);
      sv_converged    = false;
      sv_dual_iter    = 0;
      sv_dual_phase   = 0;
      sv_cycle_counter = 0;
      if (sv_lambda_shared != nullptr) {
        for (int i = 0; i <= num_nodes; i++) sv_lambda_shared[i] = 0.0f;
      }
      Serial.print("Algorithm switched to: ");
      Serial.println(use_dual_decomp ? "DUAL DECOMP" : "FEEDFORWARD");
    }

    // The end ... FINALLY

  }
}