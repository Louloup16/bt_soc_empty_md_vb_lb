/*
 * sensor.c
 *
 *  Created on: 26 mai 2026
 *      Author: matte
 */
#include "sensor.h"
#include <math.h>
#include <stdint.h>
#include "sl_sensor_rht.h"
#include "app_log.h"


//conversion des données capteurs selon GATT Specification Supplement val=rawdata*M*10^D*2^B
double conv_val_sensor_toBLE(int32_t rawData, int32_t paramM,int32_t paramD, int32_t paramB){
  double paramWithD= (double) pow(10,paramD);
  double paramWithB= (double) pow(2,paramB);
  return (double)rawData*((double)paramM)*paramWithD*paramWithB;
}
