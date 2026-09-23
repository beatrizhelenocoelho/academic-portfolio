#include "commands.h"
#include "can_bus.h"
#include "constants.h"

// Command variable declaration 
extern float *u_hist_matrix;
extern int   *u_hist_col;
extern int    num_nodes;
extern unsigned long init_time;

// Process all commands -> MONSTER FUNCTION
void serial_commands(pid &my_pid, float &duty_cycle, float &r, float y_lux,
                    float v_measured, bool &anti_windup, bool &feedback_on,
                    float d, float G, float total_energy, float total_visibility,
                    float total_flicker, int node_id,
                    float &high_ref, float &low_ref, float &energy_cost,
                    bool &restart_requested) {

  if (Serial.available() > 0) {
    char cmd = Serial.read();

    // 'g <sub> <i>' -> GET commands 
    if (cmd == 'g') {
      while (Serial.peek() == ' ') Serial.read();
      char sub = Serial.read();
      while (Serial.peek() == ' ') Serial.read();
      int desk_n = Serial.parseInt();

      if (desk_n <= 0) { Serial.println("err"); goto flush; }

      // ============= TABLE 1 =============
      if (sub == 'u') {          // Get duty cycle
        if (desk_n == node_id) {
          Serial.print("u "); Serial.print(desk_n); Serial.print(" "); Serial.println(duty_cycle, 4);
        } else { can_send_get(node_id, desk_n, 'u'); }
      }
      else if (sub == 'r') {     // Get illuminance reference
        if (desk_n == node_id) {
          Serial.print("r "); Serial.print(desk_n); Serial.print(" "); Serial.println(r, 4);
        } else { can_send_get(node_id, desk_n, 'r'); }
      }
      else if (sub == 'y') {     // Measure actual illuminance (LUX sensor)
        if (desk_n == node_id) {
          Serial.print("y "); Serial.print(desk_n); Serial.print(" "); Serial.println(y_lux, 4);
        } else { can_send_get(node_id, desk_n, 'y'); }
      }
      else if (sub == 'v') {     // Measure voltage at LDR
        if (desk_n == node_id) {
          Serial.print("v "); Serial.print(desk_n); Serial.print(" "); Serial.println(v_measured, 4);
        } else { can_send_get(node_id, desk_n, 'v'); }
      }
      else if (sub == 'o') {     // Get occupancy state
        if (desk_n == node_id) {
          char occ = !feedback_on ? 'o' : (r >= high_ref ? 'h' : 'l');
          Serial.print("o "); Serial.print(desk_n); Serial.print(" "); Serial.println(occ);
        } else { can_send_get(node_id, desk_n, 'o'); }
      }
      else if (sub == 'a') {     // Get anti-windup state
        if (desk_n == node_id) {
          Serial.print("a "); Serial.print(desk_n); Serial.print(" "); Serial.println(anti_windup ? 1 : 0);
        } else { can_send_get(node_id, desk_n, 'a'); }
      }
      else if (sub == 'f') {     // Get feedback control state
        if (desk_n == node_id) {
          Serial.print("f "); Serial.print(desk_n); Serial.print(" "); Serial.println(feedback_on ? 1 : 0);
        } else { can_send_get(node_id, desk_n, 'f'); }
      }
      else if (sub == 'd') {     // Get external illuminance (disturbance)
        if (desk_n == node_id) {
          Serial.print("d "); Serial.print(desk_n); Serial.print(" "); Serial.println(d, 4);
        } else { can_send_get(node_id, desk_n, 'd'); }
      }
      else if (sub == 'p') {     // Get instantaneous power
        if (desk_n == node_id) {
          Serial.print("p "); Serial.print(desk_n); Serial.print(" "); Serial.println(duty_cycle * P_max, 4);
        } else { can_send_get(node_id, desk_n, 'p'); }
      }
      else if (sub == 't') {     // Get elapsed time since last restart
        if (desk_n == node_id) {
          float elapsed = (millis() - init_time) / 1000.0f;
          Serial.print("t "); Serial.print(desk_n); Serial.print(" "); Serial.println(elapsed, 2);
        } else { can_send_get(node_id, desk_n, 't'); }
      }

      // ============= TABLE 2 =============
      else if (sub == 'E') {
        if (desk_n == node_id) {
          Serial.print("E "); Serial.print(desk_n); Serial.print(" "); Serial.println(total_energy, 4);
        } else { can_send_get(node_id, desk_n, 'E'); }
      }
      else if (sub == 'V') {
        if (desk_n == node_id) {
          Serial.print("V "); Serial.print(desk_n); Serial.print(" "); Serial.println(total_visibility, 4);
        } else { can_send_get(node_id, desk_n, 'V'); }
      }
      else if (sub == 'F') {
        if (desk_n == node_id) {
          Serial.print("F "); Serial.print(desk_n); Serial.print(" "); Serial.println(total_flicker, 4);
        } else { can_send_get(node_id, desk_n, 'F'); }
      }
      
      // ============= TABLE 3 =============
      else if (sub == 'O') {
        if (desk_n == node_id) {
          Serial.print("O "); Serial.print(desk_n); Serial.print(" "); Serial.println(high_ref, 4);
        } else { can_send_get(node_id, desk_n, 'O'); }
      }
      else if (sub == 'U') {
        if (desk_n == node_id) {
          Serial.print("U "); Serial.print(desk_n); Serial.print(" "); Serial.println(low_ref, 4);
        } else { can_send_get(node_id, desk_n, 'U'); }
      }
      else if (sub == 'L') {
        if (desk_n == node_id) {
          Serial.print("L "); Serial.print(desk_n); Serial.print(" "); Serial.println(r, 4);
        } else { can_send_get(node_id, desk_n, 'L'); }
      }
      else if (sub == 'C') {
        if (desk_n == node_id) {
          Serial.print("C "); Serial.print(desk_n); Serial.print(" "); Serial.println(energy_cost, 4);
        } else { can_send_get(node_id, desk_n, 'C'); }
      }
      else { Serial.println("err"); }
    }

    // ============= SET COMMANDS =============

    else if (cmd == 'u') {       // Set duty cycle (Table 1)
      int desk_n = Serial.parseInt();
      float duty = Serial.parseFloat();
      if (desk_n <= 0) { Serial.println("err"); goto flush; } 
      // goto flush to kill command and clear serial pipe -> saw on gemini, works, not bad!
      if (desk_n == node_id) {
        if (duty >= 0.0f && duty <= 1.0f) {
          duty_cycle = duty;
          feedback_on = false;
          Serial.println("ack");
        } else { Serial.println("err"); }
      } else {
        can_send_set_duty(node_id, desk_n, duty);
      }
    }

    else if (cmd == 'r') {       // Set illuminance reference (Table 1)
      int desk_n = Serial.parseInt();
      float lux = Serial.parseFloat();
      if (lux >= 0.0f) {
        if (desk_n == node_id || desk_n == 0) {
          r = lux;
          feedback_on = true;
          Serial.println("ack");
        }
        if (desk_n != node_id || desk_n == 0) {
          can_send_ref(node_id, desk_n, lux);
        }
        can_send_changed_ref(node_id);
        sv_converged = false;
        sv_cycle_counter = 0;
        sv_my_turn = true;
        memset(u_hist_col, 0, sizeof(int) * (num_nodes + 1));
        sv_dual_iter  = 0;
        sv_dual_phase = 0;
        if (sv_lambda_shared != nullptr) {
          for (int i = 0; i <= num_nodes; i++) sv_lambda_shared[i] = 0.0f;
        }
      } else { Serial.println("err"); }
    }

    else if (cmd == 'o') {       // Set occupancy state (Table 1): o=off, l=low, h=high
      int desk_n = Serial.parseInt();
      while (Serial.peek() == ' ') Serial.read();
      char occ = Serial.read();
      if (desk_n <= 0) { Serial.println("err"); goto flush; }
      if (occ != 'o' && occ != 'l' && occ != 'h') { Serial.println("err"); goto flush; }
      if (desk_n == node_id) {
        if (occ == 'o') {
          feedback_on = false;
        } else if (occ == 'l') {
          r = low_ref;
          feedback_on = true;
        } else {
          r = high_ref;
          feedback_on = true;
        }
        Serial.println("ack");
      } else {
        float occ_code;

        switch (occ) {
            case 'o': occ_code = 0.0f; break;
            case 'l': occ_code = 1.0f; break;
            default:  occ_code = 2.0f; break;
        }
        can_send_set_scalar(node_id, desk_n, 'N', occ_code);
      }
    }

    else if (cmd == 'a') {       // Set anti-windup (Table 1)
      int desk_n = Serial.parseInt();
      int on = Serial.parseInt();
      if (desk_n <= 0) { Serial.println("err"); goto flush; }
      if (on != 0 && on != 1) { Serial.println("err"); goto flush; }
      if (desk_n == node_id) {
        anti_windup = (on == 1);
        Serial.println("ack");
      } else {
        can_send_set_bool(node_id, desk_n, 'A', on == 1);
      }
    }

    else if (cmd == 'f') {       // Set feedback control (Table 1)
      int desk_n = Serial.parseInt();
      int on = Serial.parseInt();
      if (desk_n <= 0) { Serial.println("err"); goto flush; }
      if (on != 0 && on != 1) { Serial.println("err"); goto flush; }
      if (desk_n == node_id) {
        feedback_on = (on == 1);
        Serial.println("ack");
      } else {
        can_send_set_bool(node_id, desk_n, 'F', on == 1);
      }
    }

    // ── Table 3 SET commands ───────────────────────────────────────────

    else if (cmd == 'O') {
      int desk_n = Serial.parseInt();
      float val = Serial.parseFloat();
      if (desk_n <= 0) { Serial.println("err"); goto flush; }
      if (desk_n == node_id) {
        if (val >= 0.0f) { high_ref = val; Serial.println("ack"); }
        else             { Serial.println("err"); }
      } else { can_send_set_scalar(node_id, desk_n, 'O', val); }
    }

    else if (cmd == 'U') {
      int desk_n = Serial.parseInt();
      float val = Serial.parseFloat();
      if (desk_n <= 0) { Serial.println("err"); goto flush; }
      if (desk_n == node_id) {
        if (val >= 0.0f) { low_ref = val; Serial.println("ack"); }
        else             { Serial.println("err"); }
      } else { can_send_set_scalar(node_id, desk_n, 'W', val); }
    }

    else if (cmd == 'C') {
      int desk_n = Serial.parseInt();
      float val = Serial.parseFloat();
      if (desk_n <= 0) { Serial.println("err"); goto flush; }
      if (desk_n == node_id) {
        if (val >= 0.0f) { energy_cost = val; Serial.println("ack"); }
        else             { Serial.println("err"); }
      } else { can_send_set_scalar(node_id, desk_n, 'C', val); }
    }

    else if (cmd == 'R') {
      restart_requested = true;
      can_send_restart(node_id, 0);
      Serial.println("ack");
    }

    // ============= DEBUG =================== // Not used for now IG 

    else if (cmd == 'k') {
      int desk_n = Serial.parseInt();
      float new_kp = Serial.parseFloat();
      float new_b  = Serial.parseFloat();
      float new_ki = Serial.parseFloat();
      if (desk_n == node_id) {
        my_pid.bumpless_transfer(new_kp, new_ki, new_b, r, y_lux);
        Serial.println("ack");
      } else {
        can_send_k(node_id, desk_n, new_kp, new_b, new_ki);
        Serial.println("ack");
      }
    }

    else if (cmd == 'J') {
      use_dual_decomp = !use_dual_decomp;
      sv_converged    = false;
      sv_dual_iter    = 0;
      sv_dual_phase   = 0;
      sv_cycle_counter = 0;
      if (sv_lambda_shared != nullptr) {
        for (int i = 0; i <= num_nodes; i++) sv_lambda_shared[i] = 0.0f;
      }
      sv_my_turn = true;
      can_send_algo_switch(node_id, use_dual_decomp);
      Serial.print("Algorithm switched to: ");
      if (use_dual_decomp) {
        Serial.println("Algorithm: DUAL DECOMPOSITION");
      } else {
          Serial.println("Algorithm: FEEDFORWARD");
      }
    }

    else if (cmd == 'D') {  // debug: print last convergence u history
      if (u_hist_matrix == nullptr || u_hist_col == nullptr) {
        Serial.println("err");
      } else {
        Serial.println("Last convergence u history:");
        for (int n = 1; n <= num_nodes; n++) {
          Serial.print("Node "); Serial.print(n); Serial.print(": ");
          int entries = u_hist_col[n];
          if (entries == 0) { Serial.println("(no data)"); continue; }
          for (int k = 0; k < entries; k++) {
            Serial.print(u_hist_matrix[n * MAX_CONV_HIST + k], 4);
            if (k < entries - 1) Serial.print(" ");
          }
          Serial.println();
        }
      }
    }

    else { Serial.println("err"); }

    flush:
    while (Serial.available() > 0) Serial.read();
  }
}
