#ifndef __USER_DATA_H__
#define __USER_DATA_H__
#include <stdint.h>

typedef struct {
    uint8_t opt;
    void (*func)(uint8_t *data, uint8_t length);
}cmd_query_t;

typedef struct {
    uint8_t  *cmdEnable;
    uint16_t *cmdDelay;
    void (*func)(void);
}cmd_report_t;

extern cmd_query_t query_cmd[];
extern cmd_query_t factory_query_cmd[];
extern cmd_report_t report_cmd[];

void onCmdQueryDeviceStatus(uint8_t *data, uint8_t length);

void onCmdSetDeviceStatus(uint8_t *data, uint8_t length);

void onCmdModifyDeviceSetting(uint8_t *data, uint8_t length);

void onCmdSetLedFlash(uint8_t *data, uint8_t length);

void onCmdClrDevAlarmSetting(uint8_t *data, uint8_t length);

// void onCmdSetDeviceStatusByAddr(uint8_t *data, uint16_t length);

// void onCmdSetLedFlashByAddr(uint8_t *data, uint16_t length);

// void onCmdClrDevAlarmSettingByAddr(uint8_t *data, uint16_t length);

void onCmdModifyBaudRate(uint8_t *data, uint8_t length);

void onCmdSetAddrByUid(uint8_t *data, uint8_t length);

void onCmdGetInfoByAddr(uint8_t *data, uint8_t length);

void onCmdSetAddrByAddr(uint8_t *data, uint8_t length);

void onCmdClearUartBuffer(uint8_t *data, uint8_t length);

void onCmdFactoryQuery(uint8_t *data, uint8_t length);

void onCmdRequestUpgrade(uint8_t *data, uint8_t length);

void onReportDeviceStatus(void);

void onReportDeviceOptResult(void);

void onReportSetDeviceResult(void);

void onReportSetLedFlashStatus(void);

void onReportClearDevAlarmSetting(void);

void onReportDevAlarm(void);

void onReportAutoLockAlarm(void);

void onReportLockFaultAlarm(void);

void onReportSingleModifyBaudRate(void);

void onReportGunStatus(void);

void onReportAutoLockAlarmByGunState(void);

void onReportSetAddrByUid(void);

void onReportGetInfoByAddr(void);

void onReportSetAddrByAddr(void);

void onReportClearUartBuffer(void);

void onReportFactoryCmd(void);

void onReportRequestUpgrade(void);

void user_database_init(void);

void user_database_save(void);

void user_reply_handle(void);

void user_check_report_delay(void);

void printSetting(void);

int write_upgrade_flag(void);

void sync_boot_env(void);

#endif
