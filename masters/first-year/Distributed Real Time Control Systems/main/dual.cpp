#include "dual.h"


// Use with 'J' command to switch 
bool use_dual_decomp = false;



// Primal -> Copied algorithm from lecture slides
float dual_compute_u(int node_id, float c_i, float q_i,
                     volatile float *lambda_shared, float *Gs, int num_nodes) {
    float numerator = -c_i;
    for (int j = 1; j <= num_nodes; j++) {
        numerator += lambda_shared[j] * Gs[node_id * (num_nodes + 1) + j];
    }
    float u = numerator / (2.0f * q_i);
    if (u < 0.0f) u = 0.0f;
    if (u > 1.0f) u = 1.0f;
    return u;
}

// Dual -> Copied algorithm from lecture slides again
float dual_compute_lambda(int node_id, float lambda_own, float L_i, float d_i,
                          volatile float *u_shared, float *Gs, int num_nodes) {
    float violation = L_i - d_i;
    for (int j = 1; j <= num_nodes; j++) {
        violation -= Gs[j * (num_nodes + 1) + node_id] * u_shared[j];
    }
    float lambda_new = lambda_own + DUAL_ALPHA * violation;
    if (lambda_new < 0.0f) lambda_new = 0.0f;
    return lambda_new;
}
