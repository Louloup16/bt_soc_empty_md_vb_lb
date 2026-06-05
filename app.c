/***************************************************************************//**
 * @file
 * @brief Core application logic.
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
#include "em_common.h"
#include "app_assert.h"
#include "sl_bluetooth.h"
#include "app.h"
#include "app_log.h"
#include "sl_sensor_rht.h"
#include "temperature.h"
#include "gatt_db.h"
#include <stdint.h>
#include "sl_sleeptimer.h"
#include "sl_simple_led_instances.h"
#include "sl_bgapi.h"
#include "sl_sensor_light.h"
#include "light.h"


#define TEMPERATURE_TIMER_SIGNAL (1<<0)
#define LUMINOSITE_TIMER_SIGNAL (1<<1)



// The advertising set handle allocated from Bluetooth stack.
static uint8_t advertising_set_handle = 0xff;

static sl_sleeptimer_timer_handle_t handleTimerTemp;
static int timerDataTemp=0;
static sl_sleeptimer_timer_handle_t handleTimerLight;
static int timerDataLight=0;
static uint8_t connection_id=0;
static sl_bt_msg_t evtNotifTemp;
static sl_bt_msg_t evtNotifLum;
static uint16_t characteristic =0;
static uint8_t statutsFlags=0;
static uint16_t statutsFlagConfig=0;

/**************************************************************************//**
 * Application Init.
 *****************************************************************************/
SL_WEAK void app_init(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application init code here!                         //
  // This is called once during start-up.
  /////////////////////////////////////////////////////////////////////////////
  app_log_info("%s\n", __FUNCTION__);
}

/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
SL_WEAK void app_process_action(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application code here!                              //
  // This is called infinitely.                                              //
  // Do not call blocking functions from here!                               //
  /////////////////////////////////////////////////////////////////////////////
}

/**************************************************************************//**
 * Bluetooth stack event handler.
 * This overrides the dummy weak implementation.
 *
 * @param[in] evt Event coming from the Bluetooth stack.
 *****************************************************************************/
void sl_bt_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;

  switch (SL_BT_MSG_ID(evt->header)) {
    // -------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack command before receiving this boot event!
    case sl_bt_evt_system_boot_id:
      // Create an advertising set.
      sc = sl_bt_advertiser_create_set(&advertising_set_handle);
      app_assert_status(sc);

      // Generate data for advertising
      sc = sl_bt_legacy_advertiser_generate_data(advertising_set_handle,
                                                 sl_bt_advertiser_general_discoverable);
      app_assert_status(sc);

      // Set advertising interval to 100ms.
      sc = sl_bt_advertiser_set_timing(
        advertising_set_handle,
        160, // min. adv. interval (milliseconds * 1.6)
        160, // max. adv. interval (milliseconds * 1.6)
        0,   // adv. duration
        0);  // max. num. adv. events
      app_assert_status(sc);
      // Start advertising and enable connections.
      sc = sl_bt_legacy_advertiser_start(advertising_set_handle,
                                         sl_bt_legacy_advertiser_connectable);
      app_assert_status(sc);
      break;

    // -------------------------------
    // This event indicates that a new connection was opened.
    case sl_bt_evt_connection_opened_id:
      app_log_info("%s: connection_opened!\n", __FUNCTION__);
      sl_simple_led_init_instances();
      sl_sensor_rht_init();
      sl_sensor_light_init();
      app_log_info("Init capteur temperature\n");
      app_log_info(" valeur de la temperature : %.2f \n",calc_temp());
      break;

    // -------------------------------
    // This event indicates that a connection was closed.
    case sl_bt_evt_connection_closed_id:
      // Generate data for advertising
      app_log_info("%s: connection_closed!\n", __FUNCTION__);
      sc = sl_bt_legacy_advertiser_generate_data(advertising_set_handle,
                                                 sl_bt_advertiser_general_discoverable);
      app_assert_status(sc);

      // Restart advertising after client has disconnected.
      sc = sl_bt_legacy_advertiser_start(advertising_set_handle,
                                         sl_bt_legacy_advertiser_connectable);
      app_assert_status(sc);

      sl_sensor_rht_deinit();
      sl_sensor_light_deinit();
      app_log_info("DeInit capteur temperature et luminosite\n");

      break;

    ///////////////////////////////////////////////////////////////////////////
    case sl_bt_evt_gatt_server_user_read_request_id:
      app_RespReadReq(evt);
      break;

    case sl_bt_evt_gatt_server_characteristic_status_id:
      app_updateCharacStatut(evt);
      break;

    case sl_bt_evt_system_external_signal_id:
      if((evt->data.evt_system_external_signal.extsignals&LUMINOSITE_TIMER_SIGNAL)!=0){
          app_sendData(&evtNotifLum,NOTIFY,LUMINOSITE);
      }
      if((evt->data.evt_system_external_signal.extsignals&TEMPERATURE_TIMER_SIGNAL)!=0){
           app_sendData(&evtNotifTemp,NOTIFY,TEMPERATURE);
      }
      break;

    case sl_bt_evt_gatt_server_user_write_request_id :
      if(evt->data.evt_gatt_server_user_write_request.characteristic== gattdb_digital_0){
          app_updateVersionLedIO(evt,WAIT_RESPONSE);
      }
      break;

    case sl_bt_cmd_gatt_write_characteristic_value_without_response_id:
      if(evt->data.evt_gatt_server_user_write_request.characteristic== gattdb_digital_0){
          app_updateVersionLedIO(evt,NO_RESPONSE);
      }
      break;
    ///////////////////////////////////////////////////////////////////////////

    // -------------------------------
    // Default event handler.
    default:
      break;
  }
}

void app_sendData(sl_bt_msg_t *evt,TypeOfSend funcToCall,dataToSend type){
  connection_id=evt->data.evt_gatt_server_user_read_request.connection;
  characteristic=evt->data.evt_gatt_server_user_read_request.characteristic;
  uint8_t valDataToSend[2];
  size_t lenVal= sizeof(valDataToSend);

  switch(type){
    case TEMPERATURE:
      app_log_info("val Temp = %2f C\n",calc_temp());
      int16_t valTemp=get_valTempFinal()*100;//mutiplier par 100 -> standard BLE pour temp
      memcpy(valDataToSend, &valTemp,lenVal);
      break;

    case LUMINOSITE:
      app_log_info("val Temp = %2f W/m2\n",calc_light());
      uint16_t valLum=get_valLightFinal()*10;//mutiplier par 10 -> standard BLE pour lum
      memcpy(valDataToSend, &valLum,lenVal);
      break;

    case IO :
      uint8_t ledstate= sl_simple_led_get_state(sl_led_led0.context);
       //pour communique la valeur d'une IO il faut envoyer 1 ou 0 en caractère
      if(ledstate==1){
          valDataToSend[0]='1';
          valDataToSend[1]=0;
       }
       else{
           valDataToSend[0]='0';
           valDataToSend[1]=0;
       }
       app_log_info("val Led state %d \n",valDataToSend[0]);
       break;
  }

  switch(funcToCall){
    case READ:
      sl_bt_gatt_server_send_user_read_response(connection_id,
                                                characteristic,
                                            0,
                                            lenVal,
                                            valDataToSend,
                                            (uint16_t*)&lenVal
                                            );
      break;

    case NOTIFY :
      sl_bt_gatt_server_send_notification(connection_id,
                                          characteristic,
                                                lenVal,
                                                valDataToSend
                                                );
      break;
  }
}

void sl_sleeptimer_timer_callbackTemp(sl_sleeptimer_timer_handle_t *handle, void *data){
  (void)handle;
  int* DataVal=(int*) data;
  //mise à jour du masque des signaux sur 32 bits. 1 signal= 1 bit
  sl_bt_external_signal(TEMPERATURE_TIMER_SIGNAL);
  (*DataVal)++;
  app_log_info("TimerTemp step %d \n",*DataVal);
}

void sl_sleeptimer_timer_callbackLight(sl_sleeptimer_timer_handle_t *handle, void *data){
  (void)handle;
    int* DataVal=(int*) data;
  //mise à jour du masque des signaux sur 32 bits. 1 signal= 1 bit
  sl_bt_external_signal(LUMINOSITE_TIMER_SIGNAL);
  (*DataVal)++;
   app_log_info("TimerLight step %d \n",*DataVal);

}

uint8_t app_RecupDataIO(uint8array *Val){
  return *(Val->data+(Val->len)-1);
}

void app_updateVersionLedIO(sl_bt_msg_t *evt,NeedResponse version){
   connection_id=evt->data.evt_gatt_server_user_write_request.connection;
   characteristic=evt->data.evt_gatt_server_user_write_request.characteristic;
   uint8_t valIO=app_RecupDataIO(&(evt->data.evt_gatt_server_user_write_request.value));
   if(valIO==49){
      sl_simple_led_turn_on(sl_led_led0.context);
   }
   else{
      sl_simple_led_turn_off(sl_led_led0.context);
   }
   if(version==WAIT_RESPONSE){
      app_log_info(" val to write affichage ASCII = %c \n",valIO);
      app_log_info(" val to write affichage decimal = %u \n",valIO);
      sl_bt_gatt_server_send_user_write_response(connection_id,characteristic,0);
   }
}

void app_RespReadReq(sl_bt_msg_t *evt){
  connection_id=evt->data.evt_gatt_server_user_read_request.connection;
  characteristic=evt->data.evt_gatt_server_user_read_request.characteristic;

  switch(characteristic){
    case gattdb_temperature:
      app_log_info(" id caracteristique temperature = %d \n",characteristic);
      app_sendData(evt,READ,TEMPERATURE);
      break;

    case gattdb_digital_0:
      app_log_info("read IO \n");
      app_sendData(evt,READ,IO);
      break;

    case gattdb_irradiance_0:
      app_sendData(evt,READ,LUMINOSITE);

  }
}

void app_updateCharacStatut(sl_bt_msg_t *evt){
  characteristic =evt->data.evt_gatt_server_characteristic_status.characteristic;
  statutsFlags=evt->data. evt_gatt_server_characteristic_status.status_flags;
  statutsFlagConfig=evt->data. evt_gatt_server_characteristic_status.client_config_flags;

  switch(characteristic){
      case gattdb_temperature:
        if(statutsFlags==sl_bt_gatt_server_client_config){
           app_log_info("client_config_flag = %d\n",statutsFlags);
           if(statutsFlagConfig==1 || statutsFlagConfig==3){
               evtNotifTemp=*evt;
               sl_sleeptimer_start_periodic_timer_ms(&handleTimerTemp,1000,sl_sleeptimer_timer_callbackTemp,&timerDataTemp,1,0);
           }
           else{
               sl_sleeptimer_stop_timer(&handleTimerTemp);
           }
       }
       break;

      case gattdb_irradiance_0:
        if(statutsFlags==sl_bt_gatt_server_client_config){
           app_log_info("client_config_flag = %d\n",statutsFlags);
            if(statutsFlagConfig==1 || statutsFlagConfig==3){
                evtNotifLum=*evt;
               sl_sleeptimer_start_periodic_timer_ms(&handleTimerLight,1000,sl_sleeptimer_timer_callbackLight,&timerDataLight,1,0);
            }
            else{
               sl_sleeptimer_stop_timer(&handleTimerLight);
            }
        }
      break;
    }

}
