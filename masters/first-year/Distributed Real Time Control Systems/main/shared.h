// shared.h — included by main.ino AND can_bus.cpp
#pragma once // to include only once by compiler 
#include "pid.h"
#include "hardware/sync.h"

// Shared volatile scalars (32-bit -> atomic on Cortex-M0+) 
extern volatile float sv_y_lux;
extern volatile float sv_r;
extern volatile bool  sv_feedback_on; 
extern volatile float sv_duty_cycle;   // local duty 
extern volatile int   sv_calib_st; // calib state

// Feedforward from accessible disturbance
extern volatile float *sv_duty_cycle_shared;
extern volatile bool  *sv_duty_shared;
extern int num_nodes; // to count active nodes 
extern volatile bool sv_update_u_cv;
extern volatile bool sv_my_turn; // Baton to pass between nodes
extern volatile bool sv_converged; // Convergence flag 
extern volatile int sv_cycle_counter;

// CONVERGENCE
#define MAX_CONV_HIST 6 // Convergence cycles -> USER DEFINEEE
#define REGISTERED_MCU 3 // MAX REGISTERED NODES
extern int nodes_ack; 

// Dual Decomposition shared state
extern bool           use_dual_decomp;      // false = feedforward, true = dual decomp
extern volatile float *sv_lambda_shared;    // sv_lambda_shared[j] = lambda of node j
extern volatile int    sv_dual_phase;       // 0 = primal, 1 = dual
extern volatile int    sv_dual_iter;        // cur iteration count 

// Spinlocks
extern spin_lock_t *pid_lock;
extern pid         my_pid;   // defined in main.ino, locked before use in can_bus.cpp
extern spin_lock_t *can_lock; // SUPER IMPORTANT for can.send

#define CAN_LOCK()   uint32_t _irq_save = spin_lock_blocking(can_lock)
#define CAN_UNLOCK() spin_unlock(can_lock, _irq_save)

#define PID_LOCK()   uint32_t _irq_save = spin_lock_blocking(pid_lock)
#define PID_UNLOCK() spin_unlock(pid_lock, _irq_save)