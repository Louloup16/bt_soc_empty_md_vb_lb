/*
 * gen_blt.c
 *
 *  Created on: 5 juin 2026
 *      Author: matte
 */

#include "gen_blt.h"
#include "app_log.h"


static sl_sleeptimer_timer_handle_t handleTimerTemp;
static sl_sleeptimer_timer_handle_t handleTimerLight;
static uint8_t connection_id=0;
static sl_bt_msg_t evtNotifTemp;
static sl_bt_msg_t evtNotifLum;
static uint16_t characteristic =0;
static uint8_t statutsFlags=0;
static uint16_t statutsFlagConfig=0;

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

void sl_sleeptimer_timer_callbackNotif(sl_sleeptimer_timer_handle_t *handle, void *data){
  (void)handle;
  sl_bt_msg_t *myEvt=(sl_bt_msg_t*) data;
  uint16_t myCharacteristic =myEvt->data.evt_gatt_server_characteristic_status.characteristic;
  app_log_info(" id caracteristique lu callBack = %d \n",myCharacteristic);

  switch(myCharacteristic){
    case gattdb_temperature:
      //mise à jour du masque des signaux sur 32 bits. 1 signal= 1 bit
      sl_bt_external_signal(TEMPERATURE_TIMER_SIGNAL);
    break;

    case gattdb_irradiance_0:
       //mise à jour du masque des signaux sur 32 bits. 1 signal= 1 bit
      sl_bt_external_signal(LUMINOSITE_TIMER_SIGNAL);
    break;
  }
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
               sl_bt_msg_t evtNotifTemp2;
               memcpy(&evtNotifTemp2, evt, sizeof(evtNotifTemp2));
               app_log_info("valComp data = %d\n",memcmp(&evtNotifTemp2,&evtNotifTemp,sizeof(evtNotifTemp2)));
               sl_sleeptimer_start_periodic_timer_ms(&handleTimerTemp,1000,sl_sleeptimer_timer_callbackNotif,&evtNotifTemp,1,0);
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
               sl_sleeptimer_start_periodic_timer_ms(&handleTimerLight,1000,sl_sleeptimer_timer_callbackNotif,&evtNotifLum,1,0);
            }
            else{
               sl_sleeptimer_stop_timer(&handleTimerLight);
            }
        }
      break;
    }
}

sl_sleeptimer_timer_handle_t get_handleTimerTemp(void) {
    return handleTimerTemp; // Retrait du '&'
}

sl_sleeptimer_timer_handle_t get_handleTimerLight(void) {
    return handleTimerLight; // Retrait du '&'
}

uint8_t get_connection_id(void) {
    return connection_id;
}

sl_bt_msg_t get_evtNotifTemp(void) {
    return evtNotifTemp;
}

sl_bt_msg_t get_evtNotifLum(void) {
    return evtNotifLum;
}

uint16_t get_characteristic(void) {
    return characteristic;
}

uint8_t get_statutsFlags(void) {
    return statutsFlags;
}

uint16_t get_statutsFlagConfig(void) {
    return statutsFlagConfig;
}

void set_handleTimerTemp(sl_sleeptimer_timer_handle_t handle) {
    // Plus de vérification NULL ni d'étoile car c'est une copie par valeur
    handleTimerTemp = handle;
}

void set_handleTimerLight(sl_sleeptimer_timer_handle_t handle) {
    handleTimerLight = handle;
}

void set_connection_id(uint8_t id) {
    connection_id = id;
}

void set_evtNotifTemp(sl_bt_msg_t evt) {
    evtNotifTemp = evt;
}

void set_evtNotifLum(sl_bt_msg_t evt) {
    evtNotifLum = evt;
}

void set_characteristic(uint16_t charac) {
    characteristic = charac;
}

void set_statutsFlags(uint8_t flags) {
    statutsFlags = flags;
}

void set_statutsFlagConfig(uint16_t config) {
    statutsFlagConfig = config;
}
