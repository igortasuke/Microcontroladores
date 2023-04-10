#ifndef __AVR_ATmega328P__
    #define __AVR_ATmega328P__
#endif

#include <stdint.h>
#include <avr/interrupt.h>
#include "timer.h"
#include "gpio.h"
#include "uteis.h"
#include <stdlib.h>


/*
* Pinos de saída PORT D
*/
#define PIN_AP 0
#define PIN_BP 1
#define PIN_AN 2
#define PIN_BN 3

/*
* Pinos de Entrada PORT B
*/

#define PIN_INCREMENT PINB0
#define PIN_DECREMENT PINB1
#define PIN_MODE PINB2
#define PIN_ENTER PINB3

/*
 * Constante definindo quantos eventos de overflow devem acontecer
 * antes do callback inverter o nível lógico do LED
 */
#define MAX_NBR_OVERFLOWS 8

typedef enum {
    MODE_OFF,
    MODE_CONTINUOUS,
    MODE_ON_DEMAND_SLOW,
    MODE_ON_DEMAND_FAST,
} Oper_mode;

typedef enum{
    MICROSTEP1,
    MICROSTEP2,
    MICROSTEP3,
    MICROSTEP4
} Coil;

int8_t velocity = 0;
int8_t final_speed = 0;
uint8_t instantaneous;
uint8_t steps = 0;
uint8_t start = 0;
uint8_t cont;
uint8_t coil1;
uint8_t coil2;
uint8_t coil3;
uint8_t coil4;
uint8_t cb_achieved;
uint8_t old_value = 0x0;

Oper_mode mode;
Coil currentCoil;

uint8_t overflows_debouce = 7;
uint8_t overflows_t0 = 32;

// GPT_Config cfg0 = {MODE_CTC, DIVISOR_1024, 194, 3};


/*
 * Função de callback chamada a todo evento de overflow pelas
 * funcionalidades do temporizador. Na versão abaixo, a cada certo
 * número de eventos, o nível lógico do pino PB5 (ao qual o LED da
 * placa Arduino Nano está ligado) é invertido.
 */
void cb(GPT_t* drv) {
    static int ctr = -1;
    uint8_t overflows;

    overflows = get_nbr_overflows();
    if (++ctr == overflows) {
	    cb_achieved = 1;
	    ctr = -1;
    }
}

void cb_aceleracao(GPT16_t* drv) {

    if (!(final_speed & (1 << 7)))
        if (velocity < final_speed)
	        velocity += 5;
    else
        if (velocity > final_speed)
	        velocity -= 5;
}

void cb_debounce(GPT_t* drv) {
    static int ctr = -1;

    if (++ctr == overflows_debouce) {
        ctr = -1;
        PCICR |= (1 << PCIE0); // habilita a interrupção da porta B
	    gpt_stop(GPTD3);
    }
}


ISR(PCINT0_vect) {
    uint8_t new_value;
    uint8_t chg;
    GPT_Config cfg_debounce = {MODE_CTC, DIVISOR_1024, 194}; //,7

    // desabilita interrupção da porta B
    PCICR &= ~(1 << PCIE0);
    // ler valor da porta
    new_value = PINB; //checar registrador da porta B
    chg = new_value & old_value;
    
    if((~chg & 0x04) && (old_value & 0x04)){ // PIN_INCREMENT pino 2
        if(final_speed < 50){
            final_speed += 5;  
        }
        steps += 1;
    }   
    if((~chg & 0x08) && (old_value & 0x08)){ // PIN_DECREMENT pino 3
        if(final_speed> -50){
            final_speed -= 5;
        }
        steps -= 1;
    }  
        
    if((~chg & 0x10) && (old_value & 0x10)){ // PIN_MODE pino 4
        mode += 1;
        mode %= 4;
        velocity = 0;
        final_speed = 0;
        start = 0;
        cont = 0;
        steps = 0;
        gpt_stop_notification_t1(GPIOD2);
    }
  
    if((~chg & 0x20) && (old_value & 0x20)){ // PIN_ENTER pino 5
        start = !start;
    }

    old_value = new_value;
    // chama o temporizador
    gpt_start(GPTD3, &cfg_debounce);
    // habilita interrupção do temporizador
    gpt_start_notification(GPTD3, cb_debounce, 0);

}


void motor_run(void){
    switch (currentCoil)
    {
        case (MICROSTEP1):
            if(cb_achieved){
                gpio_write_group(GPIOD4, 0xF0, coil1);
                cb_achieved = 0;
                currentCoil += 1;
            }
        break;
        
        case (MICROSTEP2):
            if(cb_achieved){
                gpio_write_group(GPIOD4, 0xF0, coil2);
                cb_achieved = 0;
                currentCoil += 1;
            }
        break;

        case (MICROSTEP3):
            if(cb_achieved){
                gpio_write_group(GPIOD4, 0xF0, coil3);
                cb_achieved = 0;
                currentCoil += 1;
            }
        break;

        case (MICROSTEP4):
            if(cb_achieved){
                gpio_write_group(GPIOD4, 0xF0, coil4);
                cb_achieved = 0;
                currentCoil = 0;
                cont += 1;
            }
        break;

    default:
        currentCoil = 0;
        break;
    }
}

/*
 * Função main que simplesmente configura o pino digital PB5 para
 * saída e o temporizador 0 para gerar eventos de overflow a uma
 * frequência de aproximadamente a 16 kHz.
 */
int main() {
    GPT_Config cfg = {MODE_CTC, DIVISOR_1024, 0xFF};//, 0
    GPIO_mode mode_pullup[] = {GPIO_IN_PULLUP, GPIO_IN_PULLUP, GPIO_IN_PULLUP, GPIO_IN_PULLUP};
    GPIO_mode mode_out[] = {GPIO_OUT, GPIO_OUT, GPIO_OUT, GPIO_OUT};

    sei();   //habilita interrupção (função do compilador)
             //cli() desabilita interrupção

    gpt_init();
    
    //gpio_set_group_mode(GPIOD4, 0xF, mode_out);
    //gpio_set_pin_mode(GPIOD4, 0, GPIO_OUT);
    //gpio_set_pin_mode(GPIOD4, 1, GPIO_OUT);
    //gpio_set_pin_mode(GPIOD4, 2, GPIO_OUT);
    gpio_set_pin_mode(GPIOD2, 0, GPIO_OUT);
    gpio_set_pin_mode(GPIOD4, 4, GPIO_OUT);
    gpio_set_pin_mode(GPIOD4, 5, GPIO_OUT);
    gpio_set_pin_mode(GPIOD4, 6, GPIO_OUT);
    gpio_set_pin_mode(GPIOD4, 7, GPIO_OUT);
    gpio_clear_port(GPIOD4);

    gpio_set_group_mode(GPIOD2, 0b00111100, mode_pullup);

    PCICR |= (1 << PCIE0); // habilita a interrupção da porta B
    PCMSK0 |= 0b00111100; // habilita a interrupção dos pinos 0, 1, 2 e 3 da porta B

    gpt_start(GPTD1, &cfg);
    gpt_start_notification(GPTD1, cb, 0);

    gpt_start_t1(GPTD2, 0x01, 0b00111000);  // 312 em binario 256 + 32 + 16 + 8 {0x01, 0b00111000}


    old_value = PINB;
    
    velocity = -5;

    while (1){       
        if(velocity > 0){                           //define sentido de rotação
            coil1 = 0x10;
            coil2 = 0x20;
            coil3 = 0x40;
            coil4 = 0x80;
        }
        else if(velocity < 0){
            coil1 = 0x80;
            coil2 = 0x40;
            coil3 = 0x20;
            coil4 = 0x10;
        }

        //gpio_write_pin(GPIOD2, 0, start);

        switch(mode){
            case MODE_OFF:      
                gpio_write_group(GPIOD4, 0xF0, 0x00);
            break;
            case MODE_CONTINUOUS:                  
                if(start){     
                    gpt_start_notification_t1(GPTD2, cb_aceleracao, 0);
                    gpio_write_pin(GPIOD2, 0, start); // DEBUG
                }
                else
                    gpt_stop_notification_t1(GPIOD2);
            break;
            case MODE_ON_DEMAND_SLOW:
                if(start && (cont <= abs(steps))){
                    velocity = 2;
                    //gpio_write_group(GPIOD4, 0xF0, 0x40);
                }else{
                    velocity = 0;
                }
            break;
            case MODE_ON_DEMAND_FAST:
                if(start && (cont <= abs(steps))){
                    velocity = 10;     
                    //gpio_write_group(GPIOD4, 0xF0, 0x80);
                }else{
                    velocity = 0;
                }
            break;
            //default:
                //gpio_write_group(GPIOD4, 0xF, 0x0);
        }
        set_microstep_speed(GPTD1, velocity);
        motor_run();
    }
}