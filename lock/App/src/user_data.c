#include <stdio.h>
#include "user_data.h"
#include "common.h"
#include "user_protocol.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_flash_ex.h"  

void onCmdQueryDeviceStatus(uint8_t *data, uint16_t length)
{
    uint32_t uid0;
    uint32_t uid1;
    uint32_t uid2;
    uint16_t pos = 0;

    if(length < 12){
        printf("[%s]length error!\r\n", __FUNCTION__);
        return;
    }

    uid0 = (data[pos++] << 24);
    uid0 += (data[pos++] << 16);
    uid0 += (data[pos++] << 8);
    uid0 += data[pos++];

    uid1 = (data[pos++] << 24);
    uid1 += (data[pos++] << 16);
    uid1 += (data[pos++] << 8);
    uid1 += data[pos++];

    uid2 = (data[pos++] << 24);
    uid2 += (data[pos++] << 16);
    uid2 += (data[pos++] << 8);
    uid2 += data[pos++];

    if(lock.uid0 != uid0 || lock.uid1 != uid1 || lock.uid2 != uid2){
        printf("[%s]UID is not matched!\r\n", __FUNCTION__);
        return;
    }

    /* send dev status here */
    lock.cmdControl.reportStatus.sendCmdEnable = CMD_ENABLE;
    lock.cmdControl.reportStatus.sendCmdDelay = 0;
}

void onCmdSetDeviceStatus(uint8_t *data, uint16_t length, uint8_t ack)
{
    uint32_t uid0;
    uint32_t uid1;
    uint32_t uid2;
    uint16_t pos = 0;
    uint8_t lockSetState;
    uint16_t cmdLength;

    if(NULL == data){
        printf("[%s]data is null!\r\n", __FUNCTION__);
        return;
    }

    if(ack) cmdLength = 13;
    else    cmdLength = 1;

    if(cmdLength > length){
        printf("[%s]length error!\r\n", __FUNCTION__);
        return;
    }

    lockSetState = data[pos++];

    if(!ack){
        goto out;
    }

    uid0 = (data[pos++] << 24);
    uid0 += (data[pos++] << 16);
    uid0 += (data[pos++] << 8);
    uid0 += data[pos++];

    uid1 = (data[pos++] << 24);
    uid1 += (data[pos++] << 16);
    uid1 += (data[pos++] << 8);
    uid1 += data[pos++];

    uid2 = (data[pos++] << 24);
    uid2 += (data[pos++] << 16);
    uid2 += (data[pos++] << 8);
    uid2 += data[pos++];

    if(lock.uid0 != uid0 || lock.uid1 != uid1 || lock.uid2 != uid2){
        printf("[%s]UID is not matched!\r\n", __FUNCTION__);
        return;
    }
out:
    
    if(lock.lockState != lockSetState){
        /* set dev state here */
        if(lockSetState)    lock.lockTaskState = LOCK_TASK_STATE_BACKWARD;//lock
        else                lock.lockTaskState = LOCK_TASK_STATE_FORWARD;//unlock

        /* set led state here */
        lock.ledTask.state = LED_TASK_STATE_FLASH;
    }
    
    lock.autoLockEnable = 0;

    /* send ack msg here */
    if(ack){
        lock.cmdControl.operateResult.sendCmdEnable = CMD_ENABLE;
        lock.cmdControl.operateResult.sendCmdDelay = lock.lockReplyDelay * DELAY_BASE;
    }
}

void onCmdModifyDeviceSetting(uint8_t *data, uint16_t length, uint8_t ack)
{
    uint32_t uid0;
    uint32_t uid1;
    uint32_t uid2;
    uint16_t pos = 0;
    uint32_t lockDelay;
    uint16_t lockReplyDelay;
    uint8_t  isReport;
    uint8_t  addr;
    uint16_t cmdLength;

    if(NULL == data){
        printf("[%s]data is null!\r\n", __FUNCTION__);
        return;
    }

    if(ack) cmdLength = 18;
    else    cmdLength = 6;

    if(cmdLength > length){
        printf("[%s]length error!\r\n", __FUNCTION__);
        return;
    }

    if(ack) addr = data[pos++];

    lockDelay = data[pos++] << 16;
    lockDelay += data[pos++] << 8;
    lockDelay += data[pos++];
    
    lockReplyDelay = data[pos++] << 8;
    lockReplyDelay += data[pos++];
    
    isReport = data[pos++];

    if(!ack){
        goto out;
    }

    uid0 = (data[pos++] << 24);
    uid0 += (data[pos++] << 16);
    uid0 += (data[pos++] << 8);
    uid0 += data[pos++];

    uid1 = (data[pos++] << 24);
    uid1 += (data[pos++] << 16);
    uid1 += (data[pos++] << 8);
    uid1 += data[pos++];

    uid2 = (data[pos++] << 24);
    uid2 += (data[pos++] << 16);
    uid2 += (data[pos++] << 8);
    uid2 += data[pos++];

    printf("uid: 0x%x%x%x\r\n", uid0, uid1, uid2);

    if(lock.uid0 != uid0 || lock.uid1 != uid1 || lock.uid2 != uid2){
        printf("[%s]UID is not matched!\r\n", __FUNCTION__);
        return;
    }
out:
    /* set dev state here */
    if(ack) lock.address = addr;
    lock.lockDelay = lockDelay;
    lock.lockReplyDelay = lockReplyDelay;
    lock.isReport = isReport;
    user_database_save();
    /* send ack msg here */
    if(ack){
        lock.cmdControl.basicSetting.sendCmdEnable = CMD_ENABLE;
        lock.cmdControl.basicSetting.sendCmdDelay = 0;
    }
  
}

void onCmdSetLedFlash(uint8_t *data, uint16_t length, uint8_t ack)
{
    uint32_t uid0;
    uint32_t uid1;
    uint32_t uid2;
    uint16_t pos = 0;
    uint8_t  isFlash;
    uint16_t cmdLength;

    if(NULL == data){
        printf("[%s]data is null!\r\n", __FUNCTION__);
        return;
    }

    if(ack) cmdLength = 13;
    else    cmdLength = 1;

    if(cmdLength > length){
        printf("[%s]length error!\r\n", __FUNCTION__);
        return;
    }   

    isFlash = data[pos++];

    if(!ack){
        goto out;
    }

    uid0 = (data[pos++] << 24);
    uid0 += (data[pos++] << 16);
    uid0 += (data[pos++] << 8);
    uid0 += data[pos++];

    uid1 = (data[pos++] << 24);
    uid1 += (data[pos++] << 16);
    uid1 += (data[pos++] << 8);
    uid1 += data[pos++];

    uid2 = (data[pos++] << 24);
    uid2 += (data[pos++] << 16);
    uid2 += (data[pos++] << 8);
    uid2 += data[pos++];

    if(lock.uid0 != uid0 || lock.uid1 != uid1 || lock.uid2 != uid2){
        printf("[%s]UID is not matched!\r\n", __FUNCTION__);
        return;
    }
out:
    /* set led flash here */
    lock.ledFlashStatus = isFlash;
    user_database_save();
    /* send ack msg here */
    if(ack){
        lock.cmdControl.ledFlashSetting.sendCmdEnable = CMD_ENABLE;
        lock.cmdControl.ledFlashSetting.sendCmdDelay = 0;
    }
}

void onCmdClrDevAlarmSetting(uint8_t *data, uint16_t length, uint8_t ack)
{
    uint32_t uid0;
    uint32_t uid1;
    uint32_t uid2;
    uint16_t pos = 0;
    uint16_t cmdLength; 

    if(!ack){
        goto out;
    } 

    cmdLength = 12;
    if(cmdLength > length){
       printf("[%s]length error!\r\n", __FUNCTION__);
        return;
    }

    uid0 = (data[pos++] << 24);
    uid0 += (data[pos++] << 16);
    uid0 += (data[pos++] << 8);
    uid0 += data[pos++];

    uid1 = (data[pos++] << 24);
    uid1 += (data[pos++] << 16);
    uid1 += (data[pos++] << 8);
    uid1 += data[pos++];

    uid2 = (data[pos++] << 24);
    uid2 += (data[pos++] << 16);
    uid2 += (data[pos++] << 8);
    uid2 += data[pos++];

    if(lock.uid0 != uid0 || lock.uid1 != uid1 || lock.uid2 != uid2){
        printf("[%s]UID is not matched!\r\n", __FUNCTION__);
        return;
    }
out:
    /* send ack msg here */
    if(ack){
        /* clear device alarm setting here */
        lock.alarmStatus = LOCK_ALARM_NONE;
        lock.cmdControl.alarmSetting.sendCmdEnable = CMD_ENABLE;
        lock.cmdControl.alarmSetting.sendCmdDelay = 0;
    }else{
        lock.alarmStatus = LOCK_ALARM_NONE;
    }

    user_database_save();
}

void onReportDeviceStatus(void)
{
    uint8_t buffer[23];
    uint8_t pos = 0;
    buffer[pos++] = lock.address;
    buffer[pos++] = lock.lockState;
    buffer[pos++] = lock.gunState;
    buffer[pos++] = (lock.lockDelay >> 16) & 0xff;
    buffer[pos++] = (lock.lockDelay >> 8) & 0xff;
    buffer[pos++] = lock.lockDelay & 0xff;
    buffer[pos++] = (lock.lockReplyDelay >> 8) & 0xff;
    buffer[pos++] = lock.lockReplyDelay & 0xff;
    buffer[pos++] = lock.ledFlashStatus;
    buffer[pos++] = lock.alarmStatus;
    buffer[pos++] = lock.isReport;
    buffer[pos++] = (lock.uid0 >> 24)& 0xff;
    buffer[pos++] = (lock.uid0 >> 16) & 0xff;
    buffer[pos++] = (lock.uid0 >> 8) & 0xff;
    buffer[pos++] = lock.uid0 & 0xff;
    buffer[pos++] = (lock.uid1 >> 24)& 0xff;
    buffer[pos++] = (lock.uid1 >> 16) & 0xff;
    buffer[pos++] = (lock.uid1 >> 8) & 0xff;
    buffer[pos++] = lock.uid1 & 0xff;
    buffer[pos++] = (lock.uid2 >> 24)& 0xff;
    buffer[pos++] = (lock.uid2 >> 16) & 0xff;
    buffer[pos++] = (lock.uid2 >> 8) & 0xff;
    buffer[pos++] = lock.uid2 & 0xff;
    
    user_protocol_send_data(CMD_ACK, OPTION_QUERY_SINGLE_LOCK_STATUS, buffer, sizeof(buffer));       
}

void onReportDeviceOptResult(void)
{
    uint8_t buffer[14];
    uint8_t pos = 0;
    buffer[pos++] = lock.address;
    buffer[pos++] = lock.lockState;
    buffer[pos++] = (lock.uid0 >> 24)& 0xff;
    buffer[pos++] = (lock.uid0 >> 16) & 0xff;
    buffer[pos++] = (lock.uid0 >> 8) & 0xff;
    buffer[pos++] = lock.uid0 & 0xff;
    buffer[pos++] = (lock.uid1 >> 24)& 0xff;
    buffer[pos++] = (lock.uid1 >> 16) & 0xff;
    buffer[pos++] = (lock.uid1 >> 8) & 0xff;
    buffer[pos++] = lock.uid1 & 0xff;
    buffer[pos++] = (lock.uid2 >> 24)& 0xff;
    buffer[pos++] = (lock.uid2 >> 16) & 0xff;
    buffer[pos++] = (lock.uid2 >> 8) & 0xff;
    buffer[pos++] = lock.uid2 & 0xff;

    user_protocol_send_data(CMD_ACK, OPTION_LOCK_SINGLE_DEVICE, buffer, pos);    
}

void onReportSetDeviceResult(void)
{
    uint8_t buffer[23];
    uint8_t pos = 0;
    buffer[pos++] = lock.address;
    buffer[pos++] = (lock.lockDelay >> 8) & 0xff;
    buffer[pos++] = (lock.lockDelay >> 16) & 0xff;
    buffer[pos++] = lock.lockReplyDelay & 0xff;
    buffer[pos++] = (lock.lockReplyDelay >> 8) & 0xff;
    buffer[pos++] = lock.isReport;
    buffer[pos++] = (lock.uid0 >> 24)& 0xff;
    buffer[pos++] = (lock.uid0 >> 16) & 0xff;
    buffer[pos++] = (lock.uid0 >> 8) & 0xff;
    buffer[pos++] = lock.uid0 & 0xff;
    buffer[pos++] = (lock.uid1 >> 24)& 0xff;
    buffer[pos++] = (lock.uid1 >> 16) & 0xff;
    buffer[pos++] = (lock.uid1 >> 8) & 0xff;
    buffer[pos++] = lock.uid1 & 0xff;
    buffer[pos++] = (lock.uid2 >> 24)& 0xff;
    buffer[pos++] = (lock.uid2 >> 16) & 0xff;
    buffer[pos++] = (lock.uid2 >> 8) & 0xff;
    buffer[pos++] = lock.uid2 & 0xff;

    user_protocol_send_data(CMD_ACK, OPTION_SET_SINGLE_DEVICE, buffer, pos);     
}

void onReportSetLedFlashStatus(void)
{
    uint8_t buffer[23];
    uint8_t pos = 0;
    buffer[pos++] = lock.address;
    buffer[pos++] = lock.ledFlashStatus;
    buffer[pos++] = (lock.uid0 >> 24)& 0xff;
    buffer[pos++] = (lock.uid0 >> 16) & 0xff;
    buffer[pos++] = (lock.uid0 >> 8) & 0xff;
    buffer[pos++] = lock.uid0 & 0xff;
    buffer[pos++] = (lock.uid1 >> 24)& 0xff;
    buffer[pos++] = (lock.uid1 >> 16) & 0xff;
    buffer[pos++] = (lock.uid1 >> 8) & 0xff;
    buffer[pos++] = lock.uid1 & 0xff;
    buffer[pos++] = (lock.uid2 >> 24)& 0xff;
    buffer[pos++] = (lock.uid2 >> 16) & 0xff;
    buffer[pos++] = (lock.uid2 >> 8) & 0xff;
    buffer[pos++] = lock.uid2 & 0xff;

    user_protocol_send_data(CMD_ACK, OPTION_SET_SINGLE_DEVICE_LED, buffer, pos);     
}

void onReportClearDevAlarmSetting(void)
{
    uint8_t buffer[23];
    uint8_t pos = 0;
    buffer[pos++] = lock.address;
    buffer[pos++] = lock.alarmStatus;
    buffer[pos++] = (lock.uid0 >> 24)& 0xff;
    buffer[pos++] = (lock.uid0 >> 16) & 0xff;
    buffer[pos++] = (lock.uid0 >> 8) & 0xff;
    buffer[pos++] = lock.uid0 & 0xff;
    buffer[pos++] = (lock.uid1 >> 24)& 0xff;
    buffer[pos++] = (lock.uid1 >> 16) & 0xff;
    buffer[pos++] = (lock.uid1 >> 8) & 0xff;
    buffer[pos++] = lock.uid1 & 0xff;
    buffer[pos++] = (lock.uid2 >> 24)& 0xff;
    buffer[pos++] = (lock.uid2 >> 16) & 0xff;
    buffer[pos++] = (lock.uid2 >> 8) & 0xff;
    buffer[pos++] = lock.uid2 & 0xff;

    user_protocol_send_data(CMD_ACK, OPTION_CLR_SINGLE_DEVICE_ALARM_SETTING, buffer, pos);     
}

void onReportDevAlarm(uint8_t alarmType)
{
    uint8_t buffer[23];
    uint8_t pos = 0;
    buffer[pos++] = lock.address;
    buffer[pos++] = alarmType;
    buffer[pos++] = (lock.uid0 >> 24)& 0xff;
    buffer[pos++] = (lock.uid0 >> 16) & 0xff;
    buffer[pos++] = (lock.uid0 >> 8) & 0xff;
    buffer[pos++] = lock.uid0 & 0xff;
    buffer[pos++] = (lock.uid1 >> 24)& 0xff;
    buffer[pos++] = (lock.uid1 >> 16) & 0xff;
    buffer[pos++] = (lock.uid1 >> 8) & 0xff;
    buffer[pos++] = lock.uid1 & 0xff;
    buffer[pos++] = (lock.uid2 >> 24)& 0xff;
    buffer[pos++] = (lock.uid2 >> 16) & 0xff;
    buffer[pos++] = (lock.uid2 >> 8) & 0xff;
    buffer[pos++] = lock.uid2 & 0xff;

    user_protocol_send_data(CMD_QUERY, OPTION_MANUAL_ALARM, buffer, pos);     
}

void onReportAutoLockAlarm(void)
{
    uint8_t buffer[23];
    uint8_t pos = 0;
    buffer[pos++] = lock.address;
    buffer[pos++] = (lock.uid0 >> 24)& 0xff;
    buffer[pos++] = (lock.uid0 >> 16) & 0xff;
    buffer[pos++] = (lock.uid0 >> 8) & 0xff;
    buffer[pos++] = lock.uid0 & 0xff;
    buffer[pos++] = (lock.uid1 >> 24)& 0xff;
    buffer[pos++] = (lock.uid1 >> 16) & 0xff;
    buffer[pos++] = (lock.uid1 >> 8) & 0xff;
    buffer[pos++] = lock.uid1 & 0xff;
    buffer[pos++] = (lock.uid2 >> 24)& 0xff;
    buffer[pos++] = (lock.uid2 >> 16) & 0xff;
    buffer[pos++] = (lock.uid2 >> 8) & 0xff;
    buffer[pos++] = lock.uid2 & 0xff;

    user_protocol_send_data(CMD_QUERY, OPTION_AUTO_LOCK_ALARM, buffer, pos);    
}

uint16_t user_read_flash(uint32_t address)
{
    return *(__IO uint16_t*)address;
}

void user_database_init(void)
{
    uint16_t i;
    uint16_t lenOfDataBase = sizeof(database_t) / sizeof(uint16_t);
    database_t readDataBase;
    uint16_t *pData = (uint16_t *)&readDataBase;

    memset((uint8_t *)&lock, 0, sizeof(lock));

    printf("\r\n******************** App Start ********************\r\n");

    lock.uid0 = HAL_GetUIDw0();
    lock.uid1 = HAL_GetUIDw1();
    lock.uid2 = HAL_GetUIDw2();

    for(i=0;i<lenOfDataBase;i++){
        pData[i] = user_read_flash(DATABASE_START_ADDR + 2U*i);
    }

    if(DATABASE_MAGIC != readDataBase.magic){
        printf("Init Database!!!\r\n");
        lock.lockDelay = DEFAULT_LOCK_DELAY;
        lock.lockReplyDelay = DEFAULT_LOCK_REPLY_DELAY;
        lock.isReport = DEFAULT_LOCK_REPORT;
        lock.ledFlashStatus = DEFAULT_LOCK_LED_FLASH;
        lock.alarmStatus = DEFAULT_LOCK_ALARM_STATUS;
        user_database_save();
    }else{
        printf("Read database from flash!!!\r\n");
        lock.address = (uint8_t )readDataBase.address;
        lock.lockDelay = readDataBase.lockDelayLow;
        lock.lockDelay += (readDataBase.lockDelayHigh << 16);
        lock.lockReplyDelay = readDataBase.lockReplyDelay;
        lock.isReport = (uint8_t)readDataBase.isReport;
        lock.ledFlashStatus = (uint8_t)readDataBase.ledFlash;
        lock.alarmStatus = (uint8_t)readDataBase.alarmStatus;
    }

    printf("Chip uuid: 0x%x%x%x\r\n", lock.uid0, lock.uid1, lock.uid2);
    printf("address: 0x%X\r\n", lock.address);
    printf("isReport: 0x%X\r\n", lock.isReport);
    printf("ledFlash: 0x%X\r\n", lock.ledFlashStatus);
    printf("lockDelay: 0x%X\r\n", lock.lockDelay);
    printf("lockReplyDelay: 0x%X\r\n", lock.lockReplyDelay);
    printf("alarm status: 0x%X\r\n", lock.alarmStatus);
}

void user_database_save(void)
{
    uint16_t i;
    uint16_t lenOfDataBase = sizeof(database_t) / sizeof(uint16_t);
    database_t writeDataBase;
    uint16_t *pData = (uint16_t *)&writeDataBase;
    HAL_StatusTypeDef status;
    FLASH_EraseInitTypeDef flashEraseInitType;
    uint32_t PageError;

    writeDataBase.magic = DATABASE_MAGIC;
    writeDataBase.address = lock.address;
    writeDataBase.isReport = lock.isReport;
    writeDataBase.ledFlash = lock.ledFlashStatus;
    writeDataBase.lockDelayLow = lock.lockDelay & 0xffff;
    writeDataBase.lockDelayHigh = (lock.lockDelay >> 16) & 0xffff;
    writeDataBase.lockReplyDelay = lock.lockReplyDelay;
    writeDataBase.alarmStatus = lock.alarmStatus;

    HAL_FLASH_Unlock();

    flashEraseInitType.TypeErase = FLASH_TYPEERASE_PAGES;
    flashEraseInitType.PageAddress = DATABASE_START_ADDR;
    flashEraseInitType.NbPages = 1;
    status = HAL_FLASHEx_Erase(&flashEraseInitType, &PageError);
    
    if(HAL_OK != status){
        HAL_FLASH_Lock();
        printf("Flash erase error: %d\r\n", status);
        return;
    }

    for(i=0;i<lenOfDataBase;i++){
       if(HAL_OK != HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, DATABASE_START_ADDR + 2U*i, pData[i])){
            printf("[%s]write data[%d] fail!\r\n", __FUNCTION__, i);
       } 
    }

    HAL_FLASH_Lock();
}

void user_reply_handle(void)
{
    if(lock.cmdControl.reportStatus.sendCmdEnable && !lock.cmdControl.reportStatus.sendCmdDelay){
        lock.cmdControl.reportStatus.sendCmdEnable = CMD_DISABLE;
        onReportDeviceStatus();
    }

    if(lock.cmdControl.operateResult.sendCmdEnable && !lock.cmdControl.operateResult.sendCmdDelay){
        lock.cmdControl.operateResult.sendCmdEnable = CMD_DISABLE;
        onReportDeviceOptResult();
    }

    if(lock.cmdControl.basicSetting.sendCmdEnable && !lock.cmdControl.basicSetting.sendCmdDelay){
        lock.cmdControl.basicSetting.sendCmdEnable = CMD_DISABLE;
        onReportSetDeviceResult();
    }

    if(lock.cmdControl.ledFlashSetting.sendCmdEnable && !lock.cmdControl.ledFlashSetting.sendCmdDelay){
        lock.cmdControl.ledFlashSetting.sendCmdEnable = CMD_DISABLE;
        onReportSetLedFlashStatus();
    }

    if(lock.cmdControl.alarmSetting.sendCmdEnable && !lock.cmdControl.alarmSetting.sendCmdDelay){
        lock.cmdControl.alarmSetting.sendCmdEnable = CMD_DISABLE;
        onReportClearDevAlarmSetting();
    }

    if(lock.cmdControl.reportOperateStatus.sendCmdEnable && !lock.cmdControl.reportOperateStatus.sendCmdDelay){
        lock.cmdControl.reportOperateStatus.sendCmdEnable = CMD_DISABLE;
        onReportDevAlarm(lock.lockState);
    }

    if(lock.cmdControl.reportAutoLockAlarm.sendCmdEnable && !lock.cmdControl.reportAutoLockAlarm.sendCmdDelay){
        lock.cmdControl.reportAutoLockAlarm.sendCmdEnable = CMD_DISABLE;
        onReportAutoLockAlarm();
    }
}

