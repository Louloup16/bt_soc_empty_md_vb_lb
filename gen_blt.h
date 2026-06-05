/*
 * gen_blt.h
 *
 *  Created on: 5 juin 2026
 *      Author: matte
 */

#ifndef GEN_BLT_H_
#define GEN_BLT_H_

#define TEMPERATURE_TIMER_SIGNAL (1<<0)
#define LUMINOSITE_TIMER_SIGNAL (1<<1)

#include <stdint.h>
#include "sl_sleeptimer.h"
#include "sl_bgapi.h"
#include "sl_bt_api.h"
#include "em_common.h"
#include "app_assert.h"
#include "sl_bluetooth.h"
#include "app_log.h"
#include "sl_sensor_rht.h"
#include "temperature.h"
#include "gatt_db.h"
#include "sl_simple_led_instances.h"
#include "sl_sensor_light.h"
#include "light.h"
#include "gen_blt.h"

typedef enum{
  TEMPERATURE=0,
  LUMINOSITE,
  IO
}dataToSend;

typedef enum{
  READ=0,
  NOTIFY
}TypeOfSend;

typedef enum{
  WAIT_RESPONSE=0,
  NO_RESPONSE
}NeedResponse;

void app_sendData(sl_bt_msg_t *evt,TypeOfSend funcToCall,dataToSend type);

void sl_sleeptimer_timer_callbackNotif(sl_sleeptimer_timer_handle_t *handle, void *data);

uint8_t app_RecupDataIO(uint8array *Val);

void app_updateVersionLedIO(sl_bt_msg_t *evt,NeedResponse version);

void app_RespReadReq(sl_bt_msg_t *evt);

void app_updateCharacStatut(sl_bt_msg_t *evt);

// --- GETTERS ---
sl_sleeptimer_timer_handle_t get_handleTimerTemp(void);
sl_sleeptimer_timer_handle_t get_handleTimerLight(void);
uint8_t get_connection_id(void);
sl_bt_msg_t get_evtNotifTemp(void);
sl_bt_msg_t get_evtNotifLum(void);
uint16_t get_characteristic(void);
uint8_t get_statutsFlags(void);
uint16_t get_statutsFlagConfig(void);

// --- SETTERS ---
void set_handleTimerTemp(sl_sleeptimer_timer_handle_t handle);
void set_handleTimerLight(sl_sleeptimer_timer_handle_t handle);
void set_connection_id(uint8_t id);
void set_evtNotifTemp(sl_bt_msg_t evt);
void set_evtNotifLum(sl_bt_msg_t evt);
void set_characteristic(uint16_t charac);
void set_statutsFlags(uint8_t flags);
void set_statutsFlagConfig(uint16_t config);

#endif /* GEN_BLT_H_ */
