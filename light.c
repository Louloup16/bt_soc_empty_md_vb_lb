/*
 * light.c
 *
 *  Created on: 4 juin 2026
 *      Author: matte
 */

#include "light.h"
#include "sensor.h"
#include <stdint.h>
#include "sl_sensor_light.h"
#include "app_log.h"



static uint16_t valLightFinal;


double calc_light(void){
  float lux ;
  float uvi;
  sl_sensor_light_get(&lux, &uvi);
  // pour calcul une résolution de 0.1 est attendu donc conversion des lux vers deci lux => *10
  double valLum= conv_val_sensor_toBLE(lux, 1,-1,0)*10; // resultat en lux
  valLightFinal=(uint16_t) valLum;
  return valLum;
}

uint16_t get_valLightFinal(void){
    return valLightFinal;
}

void set_valLightFinal(uint16_t newTemp){
  valLightFinal = newTemp;
}

