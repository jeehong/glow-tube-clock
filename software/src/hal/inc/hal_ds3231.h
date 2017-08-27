#ifndef __HAL_DS3231_H__
#define __HAL_DS3231_H__

#include "common_type.h"

#include "mid_rtc.h"


int hal_ds3231_read_time(struct rtc_time *ptime);

void hal_ds3231_set_time(const struct rtc_time *ptime);
/*
 * DS3232 has two alarm, we only use alarm1
 * According to linux specification, only support one-shot alarm
 * no periodic alarm mode
 */
void hal_ds3231_read_alarm(struct rtc_wkalrm *alarm);
/*
 * linux rtc-module does not support wday alarm
 * and only 24h time mode supported indeed
 */
void hal_ds3231_set_alarm(struct rtc_wkalrm *alarm);
/*
 * 入口参数:  
 * 0: 当 RTC时间与该值匹配时，中断触发； 
 * 1: 该值不作为判定条件，即无需匹配定时即可触发
 */
void hal_ds3231_set_match(U8 mday_match, U8 hour_match, U8 min_match, U8 sec_match);

/* 清除中断状态，当触发中断后，只有清除中断状态，才允许再次触发中断 */
void hal_ds3231_de_irq(void);

void hal_ds3231_en_irq(U8 flag);

void hal_ds3231_init(QueueHandle_t *p_time_sync);


#endif

