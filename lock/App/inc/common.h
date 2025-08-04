#ifndef __COMMON_H__
#define __COMMON_H__
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h> 
// #include <math.h>
#include "main.h"
#include "flash_if.h"

#define DATABASE_START_ADDR         (0x0800F000)
#define DATABASE_MAGIC              (0xaaaa)

#define DEFAULT_LOCK_DELAY          (30)
#define DEFAULT_LOCK_REPLY_DELAY    (0)
#define DEFAULT_LOCK_LED_FLASH      (0)
#define DEFAULT_LOCK_REPORT         (1)
#define DEFAULT_LOCK_ALARM_STATUS   (0)
#define DEFAULT_AUTO_LOCK_FLAG      (1)
#define DEFAULT_SENSOR_LOCK_DELAY   (1)
#define DEFAULT_BAUD_RATE_INDEX     (4)

#define DELAY_BASE                  (10)//100ms*10 = 1s
#define FLASH_FREQ                  (1)
#define FAULT_DECT_TIME             (2)
#define SW_VERSION					(0x0701)//版本号v7.1  20250804

#define BROADCAST_ADDR              (0xFF)
#define CHECK_ADDR_INVALID(addr)    (BROADCAST_ADDR != addr && addr != lock.address)
#define CHECK_ACK(addr)             (addr == lock.address)
#define IS_ADDR_INVALID(addr)        (addr != lock.address)
#define IS_UID_INVALID(uid0, uid1, uid2)        (uid0 != lock.uid0 || uid1 != lock.uid1 || uid2 != lock.uid2)

enum {
    STATUS_REQUEST_UPGRADE = 1,
    STATUS_UPGRADE_GOING,
    STATUS_UPGRADE_SUCCESS,
};
#define PACKET_SIZE                 (32)

enum {
    CMD_DISABLE = 0,
    CMD_ENABLE,
};

typedef struct {
    uint8_t sendCmdEnable;
    uint16_t sendCmdDelay;
}cmd_setting_t;

typedef struct {
    uint8_t faultState;
    uint8_t faultDectEnable;
    uint16_t faultDectLatency;
}fault_control_t;

typedef struct {
    cmd_setting_t reportStatus;
    cmd_setting_t operateResult;
    cmd_setting_t basicSetting;
    cmd_setting_t ledFlashSetting;
    cmd_setting_t alarmSetting;
    cmd_setting_t reportOperateStatus;
    cmd_setting_t reportAutoLockAlarm;
    cmd_setting_t reportLockFaultAlarm;
    cmd_setting_t reportCheckSensorLockAlarm;
    cmd_setting_t singleModifyBaudRate;
	cmd_setting_t reportGunState;
    cmd_setting_t setAddrByUid;  
    cmd_setting_t getInfoByAddr;
    cmd_setting_t setAddrByAddr;
    cmd_setting_t clearUartBuffer;
    cmd_setting_t factoryCmd;
    cmd_setting_t upgrade;
}cmd_control_t;

typedef struct {
    uint8_t state;
    uint8_t flashOn;
    uint16_t flashCnt;
}led_task_ctrl_t;

typedef struct {
    uint8_t gunState;
    uint8_t lockDetectState1;
    uint8_t lockDetectState2;
    uint8_t lockState;
    uint8_t lockTaskState;
    uint16_t lockReplyDelay;
    uint8_t  ledFlashStatus;
    uint8_t  alarmStatus;
    uint8_t  isReport;
    uint8_t  address;
    uint8_t oldAddr;
    uint8_t autoLockFlag;
    uint8_t sensorLockDelay;//检查枪舌状态，自动关锁延时
    uint16_t sensorLockCnt;
    uint8_t HoldOnDetectEnable;
    uint16_t HoldOnLatencyCnt;
    uint8_t autoLockEnable;
    uint32_t lockDelay;
    uint32_t uid0;
    uint32_t uid1;
    uint32_t uid2;
    uint16_t baudRateIndex;
    cmd_control_t cmdControl;
    led_task_ctrl_t ledTask;
    fault_control_t faultControl;
}lock_ctrl_t;

enum {
    LOCK_TASK_STATE_IDLE = 0,
    LOCK_TASK_STATE_FORWARD,
    LOCK_TASK_STATE_BACKWARD,
    LOCK_TASK_STATE_STOP,
};

enum {
    LED_TASK_STATE_IDLE = 0,
    LED_TASK_STATE_FLASH,
};

enum {
    LOCK_STATE_UNLOCK = 0,
    LOCK_STATE_LOCK,
};

enum {
    LOCK_ALARM_NONE = 0,
    LOCK_ALARM_UNLOCK,
    LOCK_ALARM_LOCK,
};

typedef struct {
    uint16_t magic;
    uint16_t address;
    uint16_t isReport;
    uint16_t ledFlash;
    uint16_t lockDelayLow;
    uint16_t lockDelayHigh;
    uint16_t lockReplyDelay;
    uint16_t alarmStatus;
    uint16_t autoLockFlag;
    uint16_t sensorLockDelay;
    uint16_t baudRateIndex;
}database_t;

typedef struct {
    uint16_t magic;
    uint16_t address;
    uint16_t deviceCmd;
    uint16_t baudIndex;
    uint32_t upgradeFlag;
    uint32_t packetIndex;
    uint32_t packetSize;
    uint8_t packetData[PACKET_SIZE];
    uint16_t upgradeStatus;
}upgrade_t;


extern lock_ctrl_t lock;

#endif
