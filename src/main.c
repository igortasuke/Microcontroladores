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

int8_t velocity;
uint8_t instantaneous;
uint8_t step;
uint8_t start;
uint8_t cont;
uint8_t coil1;
uint8_t coil2;
uint8_t coil3;
uint8_t coil4;
uint8_t cb_achieved;
uint8_t old_value = 0x0F;

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
    static int ctr = 0;
    //uint8_t overflows;

    //overflows = get_gpt_overflows(drv);
    if (++ctr == overflows_t0) {
	    cb_achieved = 1;
	    ctr = 0;
    }
}

void cb_debounce(GPT_t* drv) {
    static int ctr = 0;
    //uint8_t overflows;

    //overflows = get_gpt_overflows(drv);
    if (++ctr == overflows_debouce) {
	    gpt_stop(GPTD3);
	    ctr = 0;
        PCIFR |= ~(1 << PCIF0); // habilita a interrupção da porta B
    }
}

//void debounce() {}

ISR(PCINT0_vect) {
    uint8_t new_value;
    uint8_t chg;
    GPT_Config cfg_debounce = {MODE_CTC, DIVISOR_1024, 194}; //,7

    // desabilita interrupção da porta B
    PCICR &= ~(1 << PCIE0);
    // ler valor da porta
    new_value = PORTB; //checar registrador da porta B
    chg = new_value & old_value;
    !start;
    gpio_write_pin(GPIOD4, 0x10, start);
    if((~chg & 0x1) && (old_value & 0x1)) // PIN_INCREMENT pino 0
        if(velocity < 50){
            velocity += 5;
            set_microstep_speed(GPTD1, 15);
        }
        
    if((~chg & 0x2) && (old_value & 0x2)) // PIN_DECREMENT pino 1
        if(velocity > -50){
            velocity -= 5;
            set_microstep_speed(GPTD1, 15);
        }
        
    if((~chg & 0x4) && (old_value & 0x4)){ // PIN_MODE pino 2
        mode += 1;
        mode %= 4;
        velocity = 0;
        instantaneous = 0;
        start = 0;
        cont = 0;
    }
    if((~chg & 0x8) && (old_value & 0x8)){ // PIN_ENTER pino 3
        !start;
        gpio_write_pin(GPIOD4, 0x10, start);
    }

    old_value = new_value;
    // chama o temporizador
    gpt_start(GPTD3, &cfg_debounce);
    // habilita interrupção do temporizador
    gpt_start_notification(GPTD3, cb_debounce, 1);

}


void motor_run(void){
    switch (currentCoil)
    {
        case (MICROSTEP1):
            if(cb_achieved){
                gpio_write_group(GPIOD4, 0x1, coil1);
                cb_achieved = 0;
                currentCoil += 1;
            }
        break;
        
        case (MICROSTEP2):
            if(cb_achieved){
                gpio_write_group(GPIOD4, 0x2, coil2);
                cb_achieved = 0;
                currentCoil += 1;
            }
        break;

        case (MICROSTEP3):
            if(cb_achieved){
                gpio_write_group(GPIOD4, 0x4, coil3);
                cb_achieved = 0;
                currentCoil += 1;
            }
        break;

        case (MICROSTEP4):
            if(cb_achieved){
                gpio_write_group(GPIOD4, 0x8, coil4);
                cb_achieved = 0;
                currentCoil = 0;
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
    PORTB |= old_value; //(0x0F); 
    PCICR |= (1 << PCIE0); // habilita a interrupção da porta B
    PCMSK0 |= (0xF); // habilita a interrupção dos pinos 0, 1, 2 e 3 da porta B

    gpt_init();
    
    gpio_set_group_mode(GPIOD4, 0xF, mode_out);
    gpio_set_pin_mode(GPIOD4, 0x10, GPIO_OUT);
    gpio_clear_pin(GPIOD4, 0x10);
    gpio_set_group_mode(GPIOD2, 0xF0, mode_pullup);

    gpt_start(GPTD1, &cfg);

    gpt_start_notification(GPTD1, cb, 0);
    

    while (1){       
        if(velocity > 0){                           //define sentido de rotação
            coil1 = 0x1;
            coil2 = 0x2;
            coil3 = 0x4;
            coil4 = 0x8;
        }
        else if(velocity < 0){
            coil1 = 0x8;
            coil2 = 0x4;
            coil3 = 0x2;
            coil4 = 0x1;
        }

        switch(mode){
            case MODE_OFF:       
                gpio_write_group(GPIOD4, 0xF, 0x0);
            break;
            case MODE_CONTINUOUS:                  
                if(start){     
                    if(instantaneous < velocity){
                        instantaneous += 5;
                        //delay 50passos/s²
                    }
                    motor_run();
                }
            break;
            case MODE_ON_DEMAND_SLOW:
                if(start && cont < abs(velocity)){
                    set_microstep_speed(GPTD1, 2);     
                    motor_run();
                    cont++;
                    //delay 2 passos/s...
                }
            break;
            case MODE_ON_DEMAND_FAST:
                if(start && cont < abs(velocity)){     
                    gpio_write_group(GPIOD4, 0x1, coil1);
                    gpio_write_group(GPIOD4, 0x2, coil2);
                    gpio_write_group(GPIOD4, 0x3, coil3);
                    gpio_write_group(GPIOD4, 0x4, coil4);
                    set_microstep_speed(GPTD1, 10);
                    motor_run();  
                    cont++;
                    //delay 10 passos/s...
                }
            break;
            default:
                gpio_write_group(GPIOD4, 0xF, 0x0);
        }
    }
}