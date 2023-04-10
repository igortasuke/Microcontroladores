#include "uteis.h"
#include <stdlib.h>

GPT_Config cfg = {MODE_CTC, DIVISOR_1024, 194, 3};

void set_microstep_speed(GPT_t* gptp, int8_t velocidade) {
    uint8_t reg_top;
    uint8_t overflows;
    static int flag = 0;

    if (!velocidade){
        gpt_stop(gptp);
        flag = 1;
    }else{
        if(flag){
            gpt_start(gptp, &cfg);
            flag = 0;
        }else{
            switch (abs(velocidade)) {
                case (50):
                reg_top = 77;
                overflows = 0;
                break;

                case (45):
                reg_top = 85;
                overflows = 0;
                break;

                case (40):
                reg_top = 97;
                overflows = 0;
                break;

                case (35):
                reg_top = 111;
                overflows = 0;
                break;

                case (30):
                reg_top = 129;
                overflows = 0;
                break;

                case (25):
                reg_top = 155;
                overflows = 0;
                break;

                case (20):
                reg_top = 194;
                overflows = 0;
                break;

                case (15):
                reg_top = 255;
                overflows = 0;
                break;

                case (10):
                reg_top = 195;
                overflows = 1;
                break;

                case (5):
                reg_top = 194;
                overflows = 3;
                break;

                case (2):
                reg_top = 194;
                overflows = 10;
                break;

                default:
                reg_top = 194;
                overflows = 3;
                break;
            }

            gpt_change_interval(gptp, reg_top, overflows);
        }
    }
}