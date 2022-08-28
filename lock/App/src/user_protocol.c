#include <string.h>
#include <stdio.h>
#include "user_protocol.h"
#include "user_crc.h"
#include "user_buffer.h"

static RingBuffer myRingBuffer; 

extern void user_uart1_send_data(uint8_t *data, uint16_t size);

/**
*@brief    串口解包函数
*@param    [in] inStream : 数据流
*		   [out]pResult : 解包结果
*@return   PTL_STATUS 解包状态
*@note     
*/
PTL_STATUS ComUnpack(ComStream_t * inStream,UnPack_t *pResult)
{
	bool bHasHead = false;
	uint16_t indexOfHead = 0;
	uint16_t nTailPos = 0;
	uint16_t nLenofActData = 0;
	uint16_t nLength;
	uint16_t cal_crc;
	uint16_t crc;
	uint8_t pTempBuf[MAX_PACK_LEN];
	static uint8_t buf[MAX_PACK_LEN];
	
	memset(&buf, 0, MAX_PACK_LEN);
	for(uint32_t j=0;j< inStream->nLenOfBuf; j++)
		buf[j] = inStream->pBuffer[j];

	//1.寻找包头
	for (uint32_t i = 0; i < inStream->nLenOfBuf; i++){
		if (inStream->pBuffer[i] == REC_FRAME_HEADER){
			bHasHead = true;
			indexOfHead = i;
			break;
		}
	}
	
	//如果没有找到包头，返回一个错误
	if (!bHasHead){
		return PTL_HEAD_ERROR;
	}

	//2.寻找包尾(从包头开始)
	for (uint32_t i = indexOfHead + 1; i < inStream->nLenOfBuf; i++){
		if (inStream->pBuffer[i] == FRAME_TAIL){
			nTailPos = i;
			break;
		}

//		if (inStream->pBuffer[i] == REC_FRAME_HEADER){
//			pResult->indexOfHead = i;
//			printf("length error\r\n");
//			return PTL_LENGTH_ERROR;
//		}
	}
	
	//包不完整(包破坏或等待继续接收)
	if (nTailPos < 1){
		pResult->indexOfHead = indexOfHead;
		return PTL_LENGTH_ERROR;

	}
	
	//3.对一个包进行反转义(除开包头、包尾)
	for (uint16_t i = indexOfHead + 1; i < nTailPos; i++){
		pTempBuf[nLenofActData++] = inStream->pBuffer[i];
	}
	
	//4.验证包长度:2byte
	nLength = (pTempBuf[1] << 8) + pTempBuf[2];//数据区长度
	pResult->indexOfHead = indexOfHead;
	if (nLenofActData != nLength + 5){
		pResult->indexOfHead = nTailPos + 1;
		return PTL_LENGTH_ERROR;
	}

	//5.获取尾部索引
	pResult->indexOfTail = nTailPos;

	//6.验证校验码（cmd + length + data）
	crc = (pTempBuf[nLenofActData - 2] << 8) + pTempBuf[nLenofActData - 1];
	cal_crc = cal_crc16((char *)pTempBuf, nLength + 3);
	if (cal_crc != crc){
		return PTL_CHECKSUM_ERROR;
	}

	//7.获取命令ID
	pResult->cmdID = pTempBuf[0];

	//8.获取数据域长度
	pResult->lenOfData = nLength;

	//9.获取数据域
	memcpy(pResult->pData, &pTempBuf[5], pResult->lenOfData);

	return PTL_NO_ERROR;
}

/**
*@brief    串口打包函数
*@param    [in] pPack : 需要打包的数据
*		   [out]outStream : 数据流
*@return   PTL_STATUS 打包状态
*@note
*/
PTL_STATUS ComPack(const Pack_t*  pPack, ComStream_t* outStream)					
{
	uint8_t pPackAct[MAX_PACK_LEN];
	uint16_t pos = 0;
	uint16_t crc;
	
	if (pPack == NULL || outStream == NULL)
		return PTL_DATA_ERROR;
	
	//1.填充包头
	
	pPackAct[pos++] = SND_FRAME_HEADER;
	
	//2.填充命令ID
	pPackAct[pos++] = pPack->cmdID;

	//3. 填充长度(option + cmd + nLenOfData) 
	pPackAct[pos++] = (pPack->nLenOfData >> 8) & 0xff;
	pPackAct[pos++] = (pPack->nLenOfData >> 0) & 0xff;
	
	//4.填充数据域
	memcpy(&pPackAct[pos], pPack->pData, pPack->nLenOfData);
	pos += pPack->nLenOfData;
	
	//5.填充校验字节(校验  cmd + option + data)
	crc = cal_crc16((char *)&pPackAct[1], pPack->nLenOfData + 3);
	pPackAct[pos++] = (crc >> 8) & 0xff;
	pPackAct[pos++] = (crc >> 0) & 0xff;
	
	//6.填充包尾
	pPackAct[pos++] = FRAME_TAIL;

	//7.数据包
	outStream->nLenOfBuf = 0;
	for (uint16_t i = 0; i < pos; i++){
		outStream->pBuffer[outStream->nLenOfBuf] = pPackAct[i];
		outStream->nLenOfBuf ++;
	}

	return PTL_NO_ERROR;
}

void user_event_process(uint8_t cmd, uint8_t *data, uint16_t lenOfData)
{
	printf("cmd: %d, data length: %d\r\n", cmd, lenOfData);
//	switch(cmd){
//		
//	}
}
void user_protocol_init(void)
{
	user_buffer_create(&myRingBuffer, MAX_BUFFER_SIZE, 0, 0);
}

void user_protocol_push_data(uint8_t *data, uint16_t size)
{
	user_buffer_write_items(&myRingBuffer, data, size);
}
void user_protocol_handle(void)
{
	static uint16_t size;
	PTL_STATUS rc;
	ComStream_t inStreamReseave;
	UnPack_t pResultReseave;
	uint8_t read_buftemp[MAX_PACK_LEN];
	uint8_t read_buftest[MAX_PACK_LEN];
	
	memset(read_buftemp, 0, sizeof(read_buftemp));
	memset(read_buftest, 0, sizeof(read_buftest));
	
	size = user_buffer_item_count(&myRingBuffer);
	if(size > 0){
		inStreamReseave.nLenOfBuf = size > MAX_PACK_LEN ? MAX_PACK_LEN : size;
		inStreamReseave.pBuffer = read_buftemp;
		user_buffer_read_items(&myRingBuffer, inStreamReseave.pBuffer, inStreamReseave.nLenOfBuf);
		
		pResultReseave.pData = read_buftest;
		
		rc = ComUnpack(&inStreamReseave, &pResultReseave);
		
		if(PTL_HEAD_ERROR == rc){
			user_buffer_release_items(&myRingBuffer, inStreamReseave.nLenOfBuf);
		}else if(PTL_LENGTH_ERROR == rc){
			user_buffer_release_items(&myRingBuffer, pResultReseave.indexOfHead);
		}else if((PTL_CHECKSUM_ERROR == rc) || (PTL_TAIL_ERROR == rc) || (PTL_DATA_ERROR == rc)){
			user_buffer_release_items(&myRingBuffer, pResultReseave.indexOfTail + 1);
		}else if(PTL_NO_ERROR == rc){
			user_event_process(pResultReseave.cmdID, pResultReseave.pData, pResultReseave.lenOfData);
			user_buffer_release_items(&myRingBuffer, pResultReseave.indexOfTail + 1);
		}
	}
}

void user_protocol_send_data(uint8_t cmd, uint8_t *data, uint16_t size)
{
	Pack_t packet;
	ComStream_t output;
	uint8_t buffer[MAX_PACK_LEN];
	
	packet.cmdID = cmd;
	packet.nLenOfData = size;
	packet.pData = data;
	
	output.pBuffer = (uint8_t *)buffer;
	output.nLenOfBuf = 0;
	memset(output.pBuffer, 0, MAX_PACK_LEN);
	
	ComPack(&packet, &output);
	
	user_uart1_send_data(output.pBuffer, output.nLenOfBuf);
}



