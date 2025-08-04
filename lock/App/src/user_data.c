#include <stdio.h>
#include <string.h>
#include "user_data.h"
#include "common.h"
#include "user_protocol.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_flash_ex.h"  

static uint32_t packetIndex;
static uint32_t packetSize;
static uint8_t upgradeStatus;
static uint8_t packetData[PACKET_SIZE];
/**
 * @brief 添加监听指令步骤
 * （1）在user_protocol.h中定义opt
 * （2）编写监听解析函数
 * （3）在query_cmd增加对应接口
 */

cmd_query_t query_cmd[] = {
    {OPTION_QUERY_SINGLE_LOCK_STATUS,           onCmdQueryDeviceStatus},
    {OPTION_LOCK_SINGLE_DEVICE,                 onCmdSetDeviceStatus},
    {OPTION_SET_SINGLE_DEVICE,                  onCmdModifyDeviceSetting},
    {OPTION_SET_SINGLE_DEVICE_LED,              onCmdSetLedFlash},
    {OPTION_CLR_SINGLE_DEVICE_ALARM_SETTING,    onCmdClrDevAlarmSetting},
    {OPT_CODE_SINGLE_MODIFY_BAUDRATE,           onCmdModifyBaudRate},
    {OPTION_SET_ADDR_BY_UID,                    onCmdSetAddrByUid},
    {OPTION_GET_INFO_BY_ADDR,                   onCmdGetInfoByAddr},
    {OPTION_SET_ADDR_BY_ADDR,                   onCmdSetAddrByAddr},
    {OPTION_CLEAR_UART_BUFFER,                  onCmdClearUartBuffer},
    {OPTION_REQUEST_UPGRADE,                    onCmdRequestUpgrade},
    {0, NULL},//must end with NULL
};

cmd_query_t factory_query_cmd[] = {
    {OPT_CODE_FACTORY_QUERY,            onCmdFactoryQuery},
    {0, NULL},//must end with NULL
};

/**
 * @brief 添加发布指令步骤
 * （1）在user_protocol.h中定义opt
 * （2）修改common.h中结构体cmd_control_t，增加对应的发布指令
 * （3）编写监听解析函数
 * （4）在report_cmd增加对应接口
 */

cmd_report_t report_cmd[] = {
    {&lock.cmdControl.reportStatus.sendCmdEnable, &lock.cmdControl.reportStatus.sendCmdDelay, onReportDeviceStatus},
    {&lock.cmdControl.operateResult.sendCmdEnable, &lock.cmdControl.operateResult.sendCmdDelay, onReportDeviceOptResult},
    {&lock.cmdControl.basicSetting.sendCmdEnable, &lock.cmdControl.basicSetting.sendCmdDelay, onReportSetDeviceResult},
    {&lock.cmdControl.ledFlashSetting.sendCmdEnable, &lock.cmdControl.ledFlashSetting.sendCmdDelay, onReportSetLedFlashStatus},
    {&lock.cmdControl.alarmSetting.sendCmdEnable, &lock.cmdControl.alarmSetting.sendCmdDelay, onReportClearDevAlarmSetting},
    {&lock.cmdControl.reportOperateStatus.sendCmdEnable, &lock.cmdControl.reportOperateStatus.sendCmdDelay, onReportDevAlarm},
    {&lock.cmdControl.reportAutoLockAlarm.sendCmdEnable, &lock.cmdControl.reportAutoLockAlarm.sendCmdDelay, onReportAutoLockAlarm},
    {&lock.cmdControl.reportLockFaultAlarm.sendCmdEnable, &lock.cmdControl.reportLockFaultAlarm.sendCmdDelay, onReportLockFaultAlarm},
    {&lock.cmdControl.reportCheckSensorLockAlarm.sendCmdEnable, &lock.cmdControl.reportCheckSensorLockAlarm.sendCmdDelay, onReportAutoLockAlarmByGunState},
    {&lock.cmdControl.reportGunState.sendCmdEnable, &lock.cmdControl.reportGunState.sendCmdDelay, onReportGunStatus},
    {&lock.cmdControl.singleModifyBaudRate.sendCmdEnable, &lock.cmdControl.singleModifyBaudRate.sendCmdDelay, onReportSingleModifyBaudRate},
    {&lock.cmdControl.setAddrByUid.sendCmdEnable, &lock.cmdControl.setAddrByUid.sendCmdDelay, onReportSetAddrByUid},
    {&lock.cmdControl.getInfoByAddr.sendCmdEnable, &lock.cmdControl.getInfoByAddr.sendCmdDelay, onReportGetInfoByAddr},
    {&lock.cmdControl.setAddrByAddr.sendCmdEnable, &lock.cmdControl.setAddrByAddr.sendCmdDelay, onReportSetAddrByAddr},
    {&lock.cmdControl.clearUartBuffer.sendCmdEnable, &lock.cmdControl.clearUartBuffer.sendCmdDelay, onReportClearUartBuffer},
    {&lock.cmdControl.factoryCmd.sendCmdEnable, &lock.cmdControl.factoryCmd.sendCmdDelay, onReportFactoryCmd},
     {&lock.cmdControl.upgrade.sendCmdEnable, &lock.cmdControl.upgrade.sendCmdDelay, onReportRequestUpgrade},
};

void onCmdQueryDeviceStatus(uint8_t *data, uint8_t length)
{
    uint8_t addr = 0;
    uint8_t pos = 0;

    addr = data[pos++];

    if(IS_ADDR_INVALID(addr)){
        printf("[%s]address is invalid!\r\n", __FUNCTION__);
        return;
    }

    /* send dev status here */
    lock.cmdControl.reportStatus.sendCmdEnable = CMD_ENABLE;
    lock.cmdControl.reportStatus.sendCmdDelay = 0;
}

void onCmdSetDeviceStatus(uint8_t *data, uint8_t length)
{
    uint8_t addr = 0;
    uint8_t pos = 0;
    uint8_t lockSetState = 0;

    lockSetState = data[pos++];
    addr = data[pos++];
    if(IS_ADDR_INVALID(addr)){
        printf("[%s]address is invalid!\r\n", __FUNCTION__);
        return;
    }

    
    if(lock.lockState != lockSetState){
        /* set dev state here */
        if(lockSetState)    lock.lockTaskState = LOCK_TASK_STATE_BACKWARD;//lock
        else                lock.lockTaskState = LOCK_TASK_STATE_FORWARD;//unlock
    }
    
    lock.autoLockEnable = 0;

    /* send ack msg here */
    // lock.cmdControl.operateResult.sendCmdEnable = CMD_ENABLE;
    // lock.cmdControl.operateResult.sendCmdDelay = lock.lockReplyDelay * DELAY_BASE;
}

void onCmdModifyDeviceSetting(uint8_t *data, uint8_t length)
{
    uint8_t addr = 0;
    uint16_t pos = 0;
    uint32_t lockDelay;
    uint16_t lockReplyDelay;
    uint8_t  isReport;
    uint8_t autoLockFlag;
    uint8_t sensorLockDelay;

    lockDelay = data[pos++] << 16;
    lockDelay += data[pos++] << 8;
    lockDelay += data[pos++];

    autoLockFlag = data[pos++];
    
    lockReplyDelay = data[pos++] << 8;
    lockReplyDelay += data[pos++];
    
    isReport = data[pos++];

    sensorLockDelay = data[pos++];

    addr = data[pos++];

    if(CHECK_ADDR_INVALID(addr)){
        printf("[%s]addr is invalid!\r\n", __FUNCTION__);
        return;
    }

    /* set dev state here */
    lock.lockDelay = lockDelay;
    lock.lockReplyDelay = lockReplyDelay;
    lock.isReport = isReport;
    lock.autoLockFlag = autoLockFlag;
    lock.sensorLockDelay = sensorLockDelay;
    user_database_save();
    /* send ack msg here */
    if(CHECK_ACK(addr)){
        lock.cmdControl.basicSetting.sendCmdEnable = CMD_ENABLE;
        lock.cmdControl.basicSetting.sendCmdDelay = 0;
    }
}

void onCmdSetLedFlash(uint8_t *data, uint8_t length)
{
    uint8_t addr = 0;
    uint16_t pos = 0;
    uint8_t  isFlash;

    isFlash = data[pos++];
    addr = data[pos++];

    if(IS_ADDR_INVALID(addr)){
        printf("[%s]address is invalid!\r\n", __FUNCTION__);
        return;
    }

    /* set led flash here */
    lock.ledFlashStatus = isFlash;
    //user_database_save();
	lock.ledTask.state = isFlash;
    /* send ack msg here */
    lock.cmdControl.ledFlashSetting.sendCmdEnable = CMD_ENABLE;
    lock.cmdControl.ledFlashSetting.sendCmdDelay = 0;
}

void onCmdClrDevAlarmSetting(uint8_t *data, uint8_t length)
{
    uint8_t addr = 0;
    uint8_t pos = 0;

    addr = data[pos++];
    if (IS_ADDR_INVALID(addr)){
        printf("[%s]address is invalid!\r\n", __FUNCTION__);
        return;
    }

    lock.alarmStatus = LOCK_ALARM_NONE;
    user_database_save();
    
    /* send ack msg here */
    lock.cmdControl.alarmSetting.sendCmdEnable = CMD_ENABLE;
    lock.cmdControl.alarmSetting.sendCmdDelay = 0;
}

void onCmdModifyBaudRate(uint8_t *data, uint8_t length)
{
    uint8_t addr = 0;
    uint16_t pos = 0;
    uint8_t baudRateIndex = 0;

    baudRateIndex = data[pos++];

    addr = data[pos++];
    if (IS_ADDR_INVALID(addr)){
        printf("[%s]address is invalid!\r\n", __FUNCTION__);
        return;
    }

    lock.baudRateIndex = baudRateIndex;

    user_database_save();

    lock.cmdControl.singleModifyBaudRate.sendCmdEnable = CMD_ENABLE;
    lock.cmdControl.singleModifyBaudRate.sendCmdDelay = 0;
}

void onCmdSetAddrByUid(uint8_t *data, uint8_t length)
{
    uint32_t uid0 = 0;
    uint32_t uid1 = 0;
    uint32_t uid2 = 0;
    uint8_t addr = 0;
    uint16_t pos = 0;

     /* UID */
    uid0 = data[pos++] << 24;
    uid0 += data[pos++] << 16;
    uid0 += data[pos++] << 8;
    uid0 += data[pos++];
    uid1 = data[pos++] << 24;
    uid1 += data[pos++] << 16;
    uid1 += data[pos++] << 8;
    uid1 += data[pos++];
    uid2 = data[pos++] << 24;
    uid2 += data[pos++] << 16;
    uid2 += data[pos++] << 8;
    uid2 += data[pos++];
    /* new address */
    addr = data[pos++];

    if(IS_UID_INVALID(uid0, uid1, uid2)){
        printf("[%s]uid is not matched!\r\n", __FUNCTION__);
        return;
    }

    if(BROADCAST_ADDR == addr){
        printf("[%s]address is invalid!\r\n", __FUNCTION__);
        return;
    }

    lock.address = addr;
    user_database_save();
    
    /* send ack msg here */
    lock.cmdControl.setAddrByUid.sendCmdEnable = CMD_ENABLE;
    lock.cmdControl.setAddrByUid.sendCmdDelay = 0;
}

void onCmdGetInfoByAddr(uint8_t *data, uint8_t length)
{
    uint8_t addr = 0;
    uint8_t pos = 0;

    addr = data[pos++];
    if(IS_ADDR_INVALID(addr)){
        printf("[%s]address is not matched!\r\n", __FUNCTION__);
        return;
    }

    /* send dev status here */
    lock.cmdControl.getInfoByAddr.sendCmdEnable = CMD_ENABLE;
    lock.cmdControl.getInfoByAddr.sendCmdDelay = 0;
}

void onCmdSetAddrByAddr(uint8_t *data, uint8_t length)
{
    uint8_t addr = 0;
    uint8_t pos = 0;
    uint8_t newAddr = 0;

    newAddr = data[pos++];
    addr = data[pos++];

    if(IS_ADDR_INVALID(addr)){
        printf("[%s]address is not matched!\r\n", __FUNCTION__);
        return;
    }

    if(BROADCAST_ADDR == newAddr){
        printf("[%s]address is invalid!\r\n", __FUNCTION__);
        return;
    }

    lock.address = newAddr;
    lock.oldAddr = addr;
    user_database_save();
    
    /* send ack msg here */
    lock.cmdControl.setAddrByAddr.sendCmdEnable = CMD_ENABLE;
    lock.cmdControl.setAddrByAddr.sendCmdDelay = 0;
}

void onCmdClearUartBuffer(uint8_t *data, uint8_t length)
{
    uint8_t addr = 0;
    uint8_t pos = 0;

    addr = data[pos++];
    if(IS_ADDR_INVALID(addr)){
        printf("[%s]address is not matched!\r\n", __FUNCTION__);
        return;
    }

    /* clear uart buffer here */
    user_set_clear_buffer_flag(1);
    
    /* send ack msg here */
    lock.cmdControl.clearUartBuffer.sendCmdEnable = CMD_ENABLE;
    lock.cmdControl.clearUartBuffer.sendCmdDelay = 0;
}

void onCmdFactoryQuery(uint8_t *data, uint8_t length)
{
    lock.cmdControl.factoryCmd.sendCmdEnable = CMD_ENABLE;
    lock.cmdControl.factoryCmd.sendCmdDelay = 0;
}

void onCmdRequestUpgrade(uint8_t *data, uint8_t length)
{
    uint8_t addr = 0;
    uint8_t pos = 0;

    addr = data[pos++];
    upgradeStatus = data[pos++];
    packetSize = (data[pos++] << 24);
    packetSize += (data[pos++] << 16);
    packetSize += (data[pos++] << 8);
    packetSize += data[pos++];

    packetIndex = (data[pos++] << 24);
    packetIndex += (data[pos++] << 16);
    packetIndex += (data[pos++] << 8);
    packetIndex += data[pos++];

    for(uint8_t i=0;i<PACKET_SIZE;i++){
        packetData[i] = data[pos++];
    }

    if(IS_ADDR_INVALID(addr)){
        printf("[%s]address is not matched!\r\n", __FUNCTION__);
        return;
    }

    if(upgradeStatus != STATUS_REQUEST_UPGRADE && upgradeStatus != STATUS_UPGRADE_GOING){
        printf("[%s]request upgrade failed!\r\n", __FUNCTION__);
        return;
    }

    /* send ack msg here */
    lock.cmdControl.upgrade.sendCmdEnable = CMD_ENABLE;
    lock.cmdControl.upgrade.sendCmdDelay = 0;
}

void onReportDeviceStatus(void)
{
    uint8_t buffer[50];
    uint8_t pos = 0;
	
	/* lock state */
    buffer[pos++] = lock.lockState;
	/* gun state */
    buffer[pos++] = lock.gunState;
	/* auto lock delay time */
    buffer[pos++] = (lock.lockDelay >> 16) & 0xff;
    buffer[pos++] = (lock.lockDelay >> 8) & 0xff;
    buffer[pos++] = lock.lockDelay & 0xff;
	/* auto lock enable flag */
    buffer[pos++] = lock.autoLockFlag;
	/* lock reply delay time */
    buffer[pos++] = (lock.lockReplyDelay >> 8) & 0xff;
    buffer[pos++] = lock.lockReplyDelay & 0xff;
	/* led status */
    buffer[pos++] = lock.ledFlashStatus;
	/* alarm status */
    buffer[pos++] = lock.alarmStatus;
	/* auto report enable flag */
    buffer[pos++] = lock.isReport;
	/* delay time of auto lock by gun status */
    buffer[pos++] = lock.sensorLockDelay;
	/* sw version */
	buffer[pos++] = (SW_VERSION >> 8) & 0xff;
    buffer[pos++] = SW_VERSION & 0xff;
    /* addr */
    buffer[pos++] = lock.address;
    
    user_protocol_send_data(CMD_ACK, OPTION_QUERY_SINGLE_LOCK_STATUS, buffer, sizeof(buffer));       
}

void onReportDeviceOptResult(void)
{
    uint8_t buffer[14];
    uint8_t pos = 0;
    
    buffer[pos++] = lock.lockState;
    buffer[pos++] = lock.address;

    user_protocol_send_data(CMD_ACK, OPTION_LOCK_SINGLE_DEVICE, buffer, pos);    
}

void onReportSetDeviceResult(void)
{
    uint8_t buffer[50];
    uint8_t pos = 0;
    
    buffer[pos++] = (lock.lockDelay >> 16) & 0xff;
    buffer[pos++] = (lock.lockDelay >> 8) & 0xff;
    buffer[pos++] = lock.lockDelay & 0xff;
		
    buffer[pos++] = lock.autoLockFlag;
	
	buffer[pos++] = (lock.lockReplyDelay >> 8) & 0xff;
    buffer[pos++] = lock.lockReplyDelay & 0xff;
    
    buffer[pos++] = lock.isReport;
	
    buffer[pos++] = lock.sensorLockDelay;

    buffer[pos++] = lock.address;

    user_protocol_send_data(CMD_ACK, OPTION_SET_SINGLE_DEVICE, buffer, pos);     
}

void onReportSetLedFlashStatus(void)
{
    uint8_t buffer[23];
    uint8_t pos = 0;
    
    buffer[pos++] = lock.ledFlashStatus;
    buffer[pos++] = lock.address;

    user_protocol_send_data(CMD_ACK, OPTION_SET_SINGLE_DEVICE_LED, buffer, pos);     
}

void onReportClearDevAlarmSetting(void)
{
    uint8_t buffer[23];
    uint8_t pos = 0;
    
    buffer[pos++] = lock.alarmStatus;
    buffer[pos++] = lock.address;

    user_protocol_send_data(CMD_ACK, OPTION_CLR_SINGLE_DEVICE_ALARM_SETTING, buffer, pos);     
}

void onReportDevAlarm(void)
{
    uint8_t buffer[23];
    uint8_t pos = 0;
    
    buffer[pos++] = lock.lockState;
    buffer[pos++] = lock.address;

    user_protocol_send_data(CMD_QUERY, OPTION_MANUAL_ALARM, buffer, pos);     
}

void onReportAutoLockAlarm(void)
{
    uint8_t buffer[23];
    uint8_t pos = 0;
    buffer[pos++] = lock.address;

    user_protocol_send_data(CMD_QUERY, OPTION_AUTO_LOCK_ALARM, buffer, pos);    
}

void onReportLockFaultAlarm(void)
{
    uint8_t buffer[23];
    uint8_t pos = 0;
    
    // buffer[pos++] = lock.faultControl.faultState;
    buffer[pos++] = lock.address;
    user_protocol_send_data(CMD_QUERY, OPTION_LOCK_FAIL_ALARM, buffer, pos);    
}

void onReportAutoLockAlarmByGunState(void)
{
    uint8_t buffer[23];
    uint8_t pos = 0;
    buffer[pos++] = lock.address;

    user_protocol_send_data(CMD_QUERY, OPTION_AUTO_LOCK_BY_GUN_STATE, buffer, pos);    
}

void onReportSingleModifyBaudRate(void)
{
    uint8_t buffer[23];
    uint8_t pos = 0;
    
    /* lock baudRateIndex */
    buffer[pos++] = lock.baudRateIndex;
    /* addr */
    buffer[pos++] = lock.address;
    

    user_protocol_send_data(CMD_ACK, OPT_CODE_SINGLE_MODIFY_BAUDRATE, buffer, pos); 

    HAL_Delay(100);
    HAL_NVIC_SystemReset();
}

void onReportGunStatus(void)
{
	uint8_t buffer[40];
    uint8_t pos = 0;
	
	/* alarm type(gun state) */
	buffer[pos++] = lock.gunState;
    /* addr */
	buffer[pos++] = lock.address;

    user_protocol_send_data(CMD_QUERY, OPTION_REPORT_GUN_STATE, buffer, pos);   	
}

void onReportSetAddrByUid(void)
{
    uint8_t buffer[23];
    uint8_t pos = 0;
    
    /* UID */
    buffer[pos++] = (lock.uid0 >> 24) & 0xff;
    buffer[pos++] = (lock.uid0 >> 16) & 0xff;
    buffer[pos++] = (lock.uid0 >> 8) & 0xff;
    buffer[pos++] = lock.uid0 & 0xff;
    buffer[pos++] = (lock.uid1 >> 24) & 0xff;
    buffer[pos++] = (lock.uid1 >> 16) & 0xff;
    buffer[pos++] = (lock.uid1 >> 8) & 0xff;
    buffer[pos++] = lock.uid1 & 0xff;
    buffer[pos++] = (lock.uid2 >> 24) & 0xff;
    buffer[pos++] = (lock.uid2 >> 16) & 0xff;
    buffer[pos++] = (lock.uid2 >> 8) & 0xff;
    buffer[pos++] = lock.uid2 & 0xff;
    /* addr */
    buffer[pos++] = lock.address;

    user_protocol_send_data(CMD_ACK, OPTION_SET_ADDR_BY_UID, buffer, pos);     
}

void onReportGetInfoByAddr(void)
{
    uint8_t buffer[30];
    uint8_t pos = 0;

    /* UID */
    buffer[pos++] = (lock.uid0 >> 24) & 0xff;
    buffer[pos++] = (lock.uid0 >> 16) & 0xff;
    buffer[pos++] = (lock.uid0 >> 8) & 0xff;
    buffer[pos++] = lock.uid0 & 0xff;
    buffer[pos++] = (lock.uid1 >> 24) & 0xff;
    buffer[pos++] = (lock.uid1 >> 16) & 0xff;
    buffer[pos++] = (lock.uid1 >> 8) & 0xff;
    buffer[pos++] = lock.uid1 & 0xff;
    buffer[pos++] = (lock.uid2 >> 24) & 0xff;
    buffer[pos++] = (lock.uid2 >> 16) & 0xff;
    buffer[pos++] = (lock.uid2 >> 8) & 0xff;
    buffer[pos++] = lock.uid2 & 0xff;
    /* version */
    buffer[pos++] = (SW_VERSION >> 8) & 0xff;
    buffer[pos++] = SW_VERSION & 0xff;
    /* addr */
    buffer[pos++] = lock.address;

    user_protocol_send_data(CMD_ACK, OPTION_GET_INFO_BY_ADDR, buffer, pos);     
}

void onReportSetAddrByAddr(void)
{
    uint8_t buffer[23];
    uint8_t pos = 0;
    
    /* addr */
    buffer[pos++] = lock.address;
    /* old addr */
    buffer[pos++] = lock.oldAddr;

    user_protocol_send_data(CMD_ACK, OPTION_SET_ADDR_BY_ADDR, buffer, pos);     
}

void onReportClearUartBuffer(void)
{
    uint8_t buffer[23];
    uint8_t pos = 0;
    
    /* addr */
    buffer[pos++] = lock.address;

    user_protocol_send_data(CMD_ACK, OPTION_CLEAR_UART_BUFFER, buffer, pos);     
}

void onReportFactoryCmd(void)
{
    uint8_t buffer[50];
    uint8_t pos = 0;

    /* type */
    buffer[pos++] = CMD_ACK;

    /* addr */
    buffer[pos++] = lock.address;

    /* uid */
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

    user_protocol_send_data(CMD_FACTORY_ACK, OPT_CODE_FACTORY_QUERY, buffer, pos); 
}

void onReportRequestUpgrade(void)
{

	printf("[%s]status: %d, packet index: %d, packet num: %d\r\n", __FUNCTION__, upgradeStatus, packetIndex, packetSize);
    uint8_t buffer[50];
    uint8_t pos = 0;

    switch(upgradeStatus){
        case STATUS_REQUEST_UPGRADE:{
             if(0 != write_upgrade_flag()){
                printf("Write upgrade flag fail!");
                return;
            }
            HAL_NVIC_SystemReset();//boot再回复
            return;
        }

        case STATUS_UPGRADE_GOING:{
            if(packetSize && packetSize == packetIndex){
                upgradeStatus = STATUS_UPGRADE_SUCCESS;
            }else{
                return;
            }
            break;
        }

        case STATUS_UPGRADE_SUCCESS:break;
        default:return;
    }

    /* 地址 */
    buffer[pos++] = lock.address;
    /* 状态 */
    buffer[pos++] = upgradeStatus;
    /* 包总数 */
    buffer[pos++] = (packetSize >> 24) & 0xFF;
    buffer[pos++] = (packetSize >> 16) & 0xFF;
    buffer[pos++] = (packetSize >> 8) & 0xFF;
    buffer[pos++] = packetSize & 0xFF;
    /* 包序号 */
    buffer[pos++] = (packetIndex >> 24) & 0xFF;
    buffer[pos++] = (packetIndex >> 16) & 0xFF;
    buffer[pos++] = (packetIndex >> 8) & 0xFF;
    buffer[pos++] = packetIndex & 0xFF;
    /* 数据 */
    for(int i =0; i< PACKET_SIZE;i++){
        buffer[pos++] = packetData[i];
    }

    // printf("[%s]status: %d, packet index: %d, packet num: %d\r\n", __FUNCTION__, upgradeStatus, packetIndex, packetSize);
	user_protocol_send_data(CMD_ACK, OPTION_REQUEST_UPGRADE, buffer, pos);  
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
        lock.autoLockFlag = DEFAULT_AUTO_LOCK_FLAG;
        lock.sensorLockDelay = DEFAULT_SENSOR_LOCK_DELAY;
        lock.baudRateIndex = DEFAULT_BAUD_RATE_INDEX;
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
        lock.autoLockFlag = (uint8_t)readDataBase.autoLockFlag;
        lock.sensorLockDelay = (uint8_t)readDataBase.sensorLockDelay;
        lock.baudRateIndex = (readDataBase.baudRateIndex == 0xffff) ? DEFAULT_BAUD_RATE_INDEX : readDataBase.baudRateIndex;
    }

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
    writeDataBase.autoLockFlag = lock.autoLockFlag;
    writeDataBase.sensorLockDelay = lock.sensorLockDelay;
		writeDataBase.baudRateIndex = lock.baudRateIndex;
	
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
    uint16_t funcNum = sizeof(report_cmd) / sizeof(cmd_report_t);
    for(uint16_t i = 0; i < funcNum; i++){
        if(*report_cmd[i].cmdEnable && !(*report_cmd[i].cmdDelay)){
            *report_cmd[i].cmdEnable = CMD_DISABLE;
            report_cmd[i].func();
        }
    }
}

void user_check_report_delay(void)
{
    uint16_t funcNum = sizeof(report_cmd) / sizeof(cmd_report_t);
    for(uint16_t i = 0; i < funcNum; i++){
        if(*report_cmd[i].cmdEnable && (*report_cmd[i].cmdDelay > 0)){
            *report_cmd[i].cmdDelay --;
        }
    }
}

void printSetting(void)
{
    printf("Enter App!!!!\r\n");
    printf("version: %d\r\n", SW_VERSION);
    printf("Chip uuid: 0x%x%x%x\r\n", lock.uid0, lock.uid1, lock.uid2);
    printf("address: 0x%X\r\n", lock.address);
    printf("isReport: 0x%X\r\n", lock.isReport);
    printf("ledFlash: 0x%X\r\n", lock.ledFlashStatus);
    printf("lockDelay: 0x%X\r\n", lock.lockDelay);
    printf("lockReplyDelay: 0x%X\r\n", lock.lockReplyDelay);
    printf("alarm status: 0x%X\r\n", lock.alarmStatus);
	printf("baudRateIndex: 0x%02X\r\n", lock.baudRateIndex);
}

int user_write_flash(uint32_t address, uint16_t *data, uint16_t size)
{
    HAL_StatusTypeDef status;
    FLASH_EraseInitTypeDef flashEraseInitType;
    uint32_t PageError;
    int ret = 0;

    if(NULL == data)    return -1;

    HAL_FLASH_Unlock();

    flashEraseInitType.TypeErase = FLASH_TYPEERASE_PAGES;
    flashEraseInitType.PageAddress = address;
    flashEraseInitType.NbPages = 1;
    status = HAL_FLASHEx_Erase(&flashEraseInitType, &PageError);
    
    if(HAL_OK != status){
        HAL_FLASH_Lock();
        printf("Flash erase error: %d\r\n", status);
        return -1;
    }

    for(uint16_t i=0;i<size;i++){
        if(HAL_OK != HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, address + 2U*i, data[i])){
            printf("Write data[%d] fail!\r\n", i);
            ret = -1;
        }
    }

    HAL_FLASH_Lock();

    return ret;
}

int write_upgrade_flag(void)
{
    upgrade_t upgradeData;
    uint16_t *pData = NULL;
    uint16_t lenOfDataBase = sizeof(upgrade_t) / sizeof(uint16_t);

    upgradeData.magic = DATABASE_MAGIC;
    upgradeData.address = lock.address;
    upgradeData.deviceCmd = CMD_QUERY;
    upgradeData.baudIndex = lock.baudRateIndex;
    upgradeData.upgradeFlag = APP_UPGREQ_IS_VALID;

    pData = (uint16_t *)&upgradeData;

    return user_write_flash(APP_UPGRADE_FLAG_ADDRESS, pData, lenOfDataBase);
}

void sync_boot_env(void)
{
    uint16_t i;
    uint16_t lenOfDataBase = sizeof(upgrade_t) / sizeof(uint16_t);
    upgrade_t readDataBase;
    uint16_t *pData = (uint16_t *)&readDataBase;

    for(i=0;i<lenOfDataBase;i++){
        pData[i] = user_read_flash(APP_UPGRADE_FLAG_ADDRESS + 2U*i);
    }

    if(DATABASE_MAGIC == readDataBase.magic && readDataBase.upgradeStatus == STATUS_UPGRADE_SUCCESS){
        /* sync boot env */
        upgradeStatus = STATUS_UPGRADE_SUCCESS;
        packetSize = readDataBase.packetSize;
        packetIndex = readDataBase.packetIndex;
        for(uint8_t i=0;i<PACKET_SIZE;i++){
            packetData[i] = readDataBase.packetData[i];
        }
        /* send msg */
        lock.cmdControl.upgrade.sendCmdEnable = CMD_ENABLE;
        lock.cmdControl.upgrade.sendCmdDelay = 0;
        /* clear upgrade status */
        readDataBase.upgradeStatus = 0;
        user_write_flash(APP_UPGRADE_FLAG_ADDRESS, pData, lenOfDataBase);
        printf("Upgrade done!!!!!\r\n");
    }
}
