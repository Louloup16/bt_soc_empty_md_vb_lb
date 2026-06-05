/*
 * temperature.c
 *
 *  Created on: 26 mai 2026
 *      Author: matte
 */

#include "temperature.h"
#include "sensor.h"
#include <stdint.h>
#include "sl_sensor_rht.h"
#include "app_log.h"



static int16_t valTempFinal;
static int16_t valHumFinal;


double calc_temp(void){
  uint32_t valHumRaw;
  int32_t valTempRaw;
  sl_sensor_rht_get(&valHumRaw, &valTempRaw);
  //pour calcul une résolution de 0.01 est attendu donc conversion des mili celsius vers centi celsius => /10
  double valTemp= conv_val_sensor_toBLE(valTempRaw, 1,-2,0)/10; // resultat en deg celsius
  valTempFinal=(int16_t) valTemp;
  return valTemp;
}

int16_t get_valTempFinal(void){
    return valTempFinal;
}

int16_t get_valHumFinal(void){
    return valHumFinal;
}

void set_valTempFinal(int16_t newTemp){
  valTempFinal = newTemp;
}

void set_valHumFinal(int16_t newHum){
  valHumFinal = newHum;
}
