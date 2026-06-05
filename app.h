/***************************************************************************//**
 * @file
 * @brief Application interface provided to main().
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#ifndef APP_H
#define APP_H

#include <stdint.h>
#include "sl_sleeptimer.h"
#include "sl_bgapi.h"
#include "sl_bt_api.h"

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

/**************************************************************************//**
 * Application Init.
 *****************************************************************************/
void app_init(void);

/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
void app_process_action(void);

void app_sendData(sl_bt_msg_t *evt,TypeOfSend funcToCall,dataToSend type);

void sl_sleeptimer_timer_callbackTemp(sl_sleeptimer_timer_handle_t *handle, void *data);

void sl_sleeptimer_timer_callbackLight(sl_sleeptimer_timer_handle_t *handle, void *data);

uint8_t app_RecupDataIO(uint8array *Val);

void app_updateVersionLedIO(sl_bt_msg_t *evt,NeedResponse version);

void app_RespReadReq(sl_bt_msg_t *evt);

void app_updateCharacStatut(sl_bt_msg_t *evt);

#endif // APP_H
