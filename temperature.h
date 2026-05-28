/*
 * temperature.h
 *
 *  Created on: 26 mai 2026
 *      Author: matte
 */

#ifndef TEMPERATURE_H_
#define TEMPERATURE_H_

#include <stdint.h>


//calcul val température en double
double calc_temp(void);

//getter
int16_t get_valTempFinal(void);
int16_t get_valHumFinal(void);

//setter
void set_valTempFinal(int16_t newTemp);
void set_valHumFinal(int16_t newHum);

#endif /* TEMPERATURE_H_ */
