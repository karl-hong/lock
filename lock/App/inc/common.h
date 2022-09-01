#ifndef __COMMON_H__
#define __COMMON_H__
#include <stdint.h>

typedef struct {
    uint8_t gunState;
    uint8_t lockDetectState1;
    uint8_t lockDetectState2;
    uint8_t lockState;
    uint8_t lockTaskState;

}lock_ctrl_t;

enum {
    LOCK_TASK_STATE_IDLE = 0,
    LOCK_TASK_STATE_FORWARD,
    LOCK_TASK_STATE_BACKWARD,
    LOCK_TASK_STATE_STOP,
};


extern lock_ctrl_t lock;

#endif
