/*
 * sensor.h
 *
 *  Created on: 26 mai 2026
 *      Author: matte
 */

#ifndef SENSOR_H_
#define SENSOR_H_
#include <stdint.h>


// permet de convertir une donnee de capteur au format BLE
double conv_val_sensor_toBLE(int32_t rawData, int32_t paramM,int32_t paramD, int32_t paramB);

#endif /* SENSOR_H_ */
