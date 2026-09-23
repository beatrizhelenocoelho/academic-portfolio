#ifndef CAN_BUS_H
#define CAN_BUS_H

#include "mcp2515.h"
#include "pid.h"

extern MCP2515 can0;

void can_setup();

// =========== Process messages =========== //
void can_handle_messages(int node_id, int num_nodes, uint32_t *uids, int &uid_count,
                         float &r, float y_lux, float v_measured, pid &my_pid, bool *nodes_list,
                         bool &anti_windup, bool &feedback_on, float &duty_cycle, float *Gs,
                         int &lux_replies, int &calib_st, int &boss_node, float *d_terms,
                         float total_energy, float total_visibility, float total_flicker,
                         float &high_ref, float &low_ref, float &energy_cost,
                         bool &restart_requested);

// =========== SEND TO CAN =========== //
void can_send_test(uint8_t src, uint8_t dest);
void can_send_ref(uint8_t src, uint8_t dest, float ref);
void can_send_lux_request(uint8_t src, uint8_t dest);
void can_send_k(uint8_t src, uint8_t dest, float kp, float bpid, float ki);
void can_send_ack(uint8_t src, uint8_t dest);
void can_send_lux_value(uint8_t src, uint8_t dest, float lux);
void can_store_lux_value(uint8_t src);
void can_change_state(uint8_t src, uint8_t state);
void can_send_wakeup(uint32_t uid);
void can_send_calibration_start(uint8_t src);
void can_send_calibration_end(uint8_t src);
void can_send_duty(uint8_t src, float duty);
void can_send_conv_duty(uint8_t src, float duty);
void can_send_control_turn(uint8_t src, uint8_t dest); //NOVO - MARIA
void can_send_changed_ref(uint8_t src); //NOVO -MAria
void can_skip_calib(uint8_t src); // extra
void can_check_errors(int node_id); // periodic CAN error log

// novos helpers
void can_send_get(uint8_t src, uint8_t dest, char what);
void can_send_set_scalar(uint8_t src, uint8_t dest, char type, float val);
void can_send_set_duty(uint8_t src, uint8_t dest, float duty);
void can_send_set_bool(uint8_t src, uint8_t dest, char type, bool on);
void can_send_restart(uint8_t src, uint8_t dest);
void can_send_value_reply(uint8_t src, uint8_t dest, char what, float val);
void can_send_status_reply(uint8_t src, uint8_t dest, char what, bool ok);

//  Dual Decomposition -> STATUS:FAILING
void can_send_lambda(uint8_t src, float lambda_val);             // 'P'
void can_send_dual_turn(uint8_t src, uint8_t dest, uint8_t phase); // 'D' 
void can_send_algo_switch(uint8_t src, bool use_dual);           // 'J' 

#endif