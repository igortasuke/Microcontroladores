#ifndef __AVR_ATmega328P__
    #define __AVR_ATmega328P__
#endif

#include <stdint.h>
#include <avr/interrupt.h>
#include "timer.h"
#include "gpio.h"
#include <stdlib.h>

/*
* Pinos de saída PORT B
*/
#define PIN_AP 0
#define PIN_BP 1
#define PIN_AN 2
#define PIN_BN 3

/*
* Pinos de Entrada PORT D
*/

#define PIN_INCREMENT 4
#define PIN_DECREMENT 5
#define PIN_MODE 6
#define PIN_ENTER 7

/*
 * Constante definindo quantos eventos de overflow devem acontecer
 * antes do callback inverter o nível lógico do LED
 */
#define MAX_NBR_OVERFLOWS 32

typedef enum {
    MODE_OFF,
    MODE_CONTINUOUS,
    MODE_ON_DEMAND_SLOW,
    MODE_ON_DEMAND_FAST,
} Oper_mode;

int8_t velocity;
uint8_t instantaneous;
uint8_t step;
uint8_t start;
uint8_t cont;
uint8_t coil1;
uint8_t coil2;
uint8_t coil3;
uint8_t coil4;

Oper_mode mode;

/*
 * Função de callback chamada a todo evento de overflow pelas
 * funcionalidades do temporizador. Na versão abaixo, a cada certo
 * número de eventos, o nível lógico do pino PB5 (ao qual o LED da
 * placa Arduino Nano está ligado) é invertido.
 */
void cb(GPT_t* drv) {
    static int ctr = MAX_NBR_OVERFLOWS;
    
    if (--ctr == 0) {
	gpio_toggle_pin(GPIOD2, 5);
	ctr = MAX_NBR_OVERFLOWS;
    }
}

/*
 * Função main que simplesmente configura o pino digital PB5 para
 * saída e o temporizador 0 para gerar eventos de overflow a uma
 * frequência de aproximadamente a 16 kHz.
 */
int main() {
    GPT_Config cfg = {MODE_NORMAL, DIVISOR_1024, 0xFF};

    sei();
    gpt_init();
    
    gpio_set_group_mode(GPIOD2, 0xF, GPIO_OUT);
    gpio_set_group_mode(GPIOD4, 0xF0, GPIO_IN_PULLUP);

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

        if(!PIN_MODE){
            mode += 1;
            mode %= 4;
            //delay debounce...
            velocity = 0;
            instantaneous = 0;
            start = 0;
            cont = 0;
        }

        if(!PIN_ENTER){
            !start;
            //delay debounce...
        }

        switch(mode){
            case MODE_OFF:       
                gpio_write_group(GPIOD2, 0xF, 0x0);
            break;
            case MODE_CONTINUOUS:                  
                if(!PIN_INCREMENT && velocity < 50){
                    velocity += 5;
                    //delay debounce...
                }
                if(!PIN_DECREMENT && velocity > -50){
                    velocity -= 5;
                    //delay debounce...
                } 
                if(start){     
                    if(instantaneous < velocity){
                        instantaneous += 5;
                        //delay 50passos/s²
                    }
                    gpio_write_group(GPIOD2, 0x1, coil1);
                    gpio_write_group(GPIOD2, 0x2, coil2);
                    gpio_write_group(GPIOD2, 0x3, coil3);
                    gpio_write_group(GPIOD2, 0x4, coil4);
                    //delay...
                }
            break;
            case MODE_ON_DEMAND_SLOW:
                if(!PIN_INCREMENT){
                    velocity += 5;
                    //delay debounce...
                }
                if(!PIN_DECREMENT){
                    velocity -= 5;
                    //delay debounce...
                } 
                if(start && cont < abs(velocity)){     
                    gpio_write_group(GPIOD2, 0x1, coil1);
                    gpio_write_group(GPIOD2, 0x2, coil2);
                    gpio_write_group(GPIOD2, 0x3, coil3);
                    gpio_write_group(GPIOD2, 0x4, coil4);
                    cont++;
                    //delay 2 passos/s...
                }
            break;
            case MODE_ON_DEMAND_FAST:
                if(!PIN_INCREMENT){
                    velocity += 5;
                    //delay debounce...
                }
                if(!PIN_DECREMENT){
                    velocity -= 5;
                    //delay debounce...
                } 
                if(start && cont < abs(velocity)){     
                    gpio_write_group(GPIOD2, 0x1, coil1);
                    gpio_write_group(GPIOD2, 0x2, coil2);
                    gpio_write_group(GPIOD2, 0x3, coil3);
                    gpio_write_group(GPIOD2, 0x4, coil4);
                    cont++;
                    //delay 10 passos/s...
                }
            break;
        }
    }
}
