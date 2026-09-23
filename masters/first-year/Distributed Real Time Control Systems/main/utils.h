#ifndef UTILS_H
#define UTILS_H

// Include statements
#include <Arduino.h>
#include <math.h>
#include "constants.h"
#include "pico/unique_id.h" 
#include <string.h>

// Calculate lux
float get_R_LDR(float V_measured);
float get_LUX(float R_LDR);
float read_LUX_calib();
void set_led(char format, float value);
void gain_and_background(float &d, float &G);

// Performance metrics
float energy(float prev_duty, unsigned long dt);
float visibility(float r, float expected_lux);
float flicker(float cur_duty, float prev_duty, float prev2_duty);
float automatic_references(unsigned long elapsed_time);

// Extra helpers 
float read_LUX_always();
float read_volts();
//void ident_board(int &node_id); // Sets the ID of the boards as well as the weights of the boards 
uint32_t get_board_uid32();
void load_board_params(uint32_t uid); // substitui pelo ident_board, continuo a ter parâmetros locais diferentes por placa mas o node_id deixa de depender da board física

#endif
