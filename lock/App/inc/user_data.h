#ifndef __USER_DATA_H__
#define __USER_DATA_H__
#include <stdint.h>

void onCmdQueryDeviceStatus(uint8_t *data, uint16_t length);

void onCmdSetDeviceStatus(uint8_t *data, uint16_t length, uint8_t ack);

void onCmdModifyDeviceSetting(uint8_t *data, uint16_t length, uint8_t ack);

void onCmdSetLedFlash(uint8_t *data, uint16_t length, uint8_t ack);

void onCmdClrDevAlarmSetting(uint8_t *data, uint16_t length, uint8_t ack);

void onCmdSetDeviceStatusByAddr(uint8_t *data, uint16_t length);

void onCmdSetLedFlashByAddr(uint8_t *data, uint16_t length);

void onCmdClrDevAlarmSettingByAddr(uint8_t *data, uint16_t length);

void onCmdModifyBaudRate(uint8_t *data, uint16_t length, uint8_t ack);

void onReportDeviceStatus(void);

void onReportDeviceOptResult(void);

void onReportSetDeviceResult(void);

void onReportSetLedFlashStatus(void);

void onReportClearDevAlarmSetting(void);

void onReportDevAlarm(uint8_t alarmType);

void onReportAutoLockAlarm(void);

void onReportLockFaultAlarm(void);

void onReportSingleModifyBaudRate(void);

void onReportGunStatus(void);

void user_database_init(void);

void user_database_save(void);

void user_reply_handle(void);

void printSetting(void);

#endif
