#include "BSP_KEY.h"

// 按键处理函数
// 返回按键值
// mode: 0 不支持连续按，1 支持连续按
// 返回值:
// 0 没有任何按键按下
// KEY0_PRES KEY0按下
// KEY1_PRES KEY1按下
// KEY2_PRES KEY2按下
// KEY3_PRES KEY3按下
// 注意此函数有响应优先级，KEY0 > KEY1 > KEY2 > KEY3!!
uint8_t KEY_Scan(uint8_t mode) {
	static uint8_t key_up = 1; // 按键按松开标志
	if (mode)
		key_up = 1; // 支持连按
	if (key_up && (KEY0 == 0 || KEY1 == 0 || KEY2 == 0 || KEY3 == 0)) { // 有按键按下
		HAL_Delay(10); // 去抖动
		key_up = 0;
		if (KEY0 == 0) {
			return KEY0_PRES;
		} else if (KEY1 == 0) {
			return KEY1_PRES;
		} else if (KEY2 == 0) {
			return KEY2_PRES;
		} else if (KEY3 == 0) {
			return KEY3_PRES;
		}
	} else if (KEY0 == 1 && KEY1 == 1 && KEY2 == 1 && KEY3 == 1) {
		key_up = 1; // 没有按键按下
	}
	return 0; // 无按键按下
}
