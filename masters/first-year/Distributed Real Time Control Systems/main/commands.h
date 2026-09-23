#ifndef COMMANDS_H
#define COMMANDS_H

#include <Arduino.h>
#include "pid.h"
#include "pico/unique_id.h" 
#include <string.h>
#include "shared.h"

// Pass by reference every single variable we can change 
// Probably not most efficient thing ... but works!
void serial_commands(pid &my_pid, float &duty_cycle, float &r, float y_lux,
                    float v_measured, bool &anti_windup, bool &feedback_on,
                    float d, float G, float total_energy, float total_visibility,
                    float total_flicker, int node_id,
                    float &high_ref, float &low_ref, float &energy_cost,
                    bool &restart_requested);
#endif