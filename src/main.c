#include <stdint.h>
#include <avr/interrupt.h>
#include "timer.h"
#include "gpio.h"


/*
 * Constante definindo quantos eventos de overflow devem acontecer
 * antes do callback inverter o nível lógico do LED
 */
#define MAX_NBR_OVERFLOWS 8000

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
    gpio_set_pin_mode(GPIOD2, 5, GPIO_OUT);

    gpt_start(GPTD1, &cfg);

    gpt_start_notification(GPTD1, cb, 0);

    while (1)
	;
}
