#pragma once

// Dual Decomposition 

#define DUAL_ALPHA      0.005f   // gradient ascent step for lambda update
#define DUAL_MAX_ITER   50       // max iterations before declaring convergence

// Primal
float dual_compute_u(int node_id, float c_i, float q_i,
                     volatile float *lambda_shared, float *Gs, int num_nodes);

// Dual 
float dual_compute_lambda(int node_id, float lambda_own, float L_i, float d_i,
                          volatile float *u_shared, float *Gs, int num_nodes);
