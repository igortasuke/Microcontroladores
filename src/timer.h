#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

typedef struct GPT_Struct GPT_t;
typedef void (*gpt_cb_t)(GPT_t* drv);

typedef enum {
    MODE_NORMAL,
    MODE_CNC,
    MODE_FAST_PWM
} gpt_mode_t;

typedef enum {
    DIVISOR_1,
    DIVISOR_8,
    DIVISOR_64,
    DIVISOR_256,
    DIVISOR_1024,
} gpt_divisor_t;

typedef struct {
    gpt_mode_t mode;
    gpt_divisor_t divisor;
    uint8_t top;
} GPT_Config;

extern GPT_t GPT_obj1;
extern GPT_t GPT_obj3;
#define GPTD1 (&GPT_obj1)
#define GPTD3 (&GPT_obj3)

void gpt_init(void);
void gpt_start(GPT_t *gptp, const GPT_Config *config);
void gpt_stop(GPT_t *gptp);
uint8_t gpt_start_notification(GPT_t *gptp, gpt_cb_t cb, uint8_t is_oneshot);
uint8_t gpt_stop_notification(GPT_t *gptp);
uint8_t gpt_change_interval(GPT_t *gptp, uint8_t interval);

uint8_t gpt_enable_pwm_channel(GPT_t *gptp, uint8_t channel, uint8_t width);
void gpt_disable_pwm_channel(GPT_t *gptp, uint8_t channel);

uint8_t gpt_start_channel_notification(GPT_t *gptp, uint8_t channel,
				       uint8_t interval,
				       gpt_cb_t cb, uint8_t is_oneshot);
void gpt_stop_channel_notification(GPT_t *gptp, uint8_t channel);

#endif /* TIMER_H */

