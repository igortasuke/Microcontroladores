#ifndef __AVR_ATmega328P__
    #define __AVR_ATmega328P__
#endif

#include <avr/interrupt.h>
#include "timer.h"


/*
 * Definição da estrutura de alguns registradores (mas não todos) do
 * temporizador 0 e 2 de acordo com a ordem sequencial dos
 * registradores no endereçamento de memória
 */
typedef struct {
    volatile uint8_t tccra; // Timer/Counter Control Register A
    volatile uint8_t tccrb; // Timer/Counter Control Register B
    volatile uint8_t tcnt;  // Timer/Counter
    volatile uint8_t ocra;  // Output Compare Register A
    volatile uint8_t ocrb;  // Output Compare Register B
} GPT_Regs_t;

/*
 * Definição da estrutura de dados do driver que representa o
 * temporizador. Esta estrutura deve conter tudo o que é necessário
 * para as função abaixo trabalharem com os temporizadores 0 e 2.
 */
struct GPT_Struct {
    GPT_Regs_t* regs;        // registradores
    volatile uint8_t* tifr;  // timer flag
    volatile uint8_t* timsk; // timer mascara
    gpt_mode_t current_mode; // modo de operação
    gpt_divisor_t divisor;   //
    uint8_t max_nbr_overflows;
    gpt_cb_t current_cb;     // callback atual 
    uint8_t is_oneshot;      // 
    gpt_cb_t current_channel_cb[2];
    uint8_t is_channel_oneshot[2];
};

/*
 * Definição das variáveis referentes a estrutura de dados para os
 * temporizador 0 e 2, respectivamente.
 */
GPT_t GPT_obj1;
GPT_t GPT_obj3;

/*
 * Função para inicializar as variáveis relativas as estruturas que
 * representam os temporizadores do microcontrolador
 */
void gpt_init(void) {
    GPT_obj1.regs = (GPT_Regs_t *) &TCCR0A;
    GPT_obj3.regs = (GPT_Regs_t *) &TCCR2A;
    GPT_obj1.tifr = &TIFR0;
    GPT_obj3.tifr = &TIFR2;
    GPT_obj1.timsk = &TIMSK0;
    GPT_obj3.timsk = &TIMSK2;
    GPT_obj1.max_nbr_overflows = 1;
    GPT_obj3.max_nbr_overflows = 1;

    GPT_obj1.current_cb = 0;
    GPT_obj1.current_channel_cb[0] = GPT_obj1.current_channel_cb[1] = 0;
    GPT_obj3.current_cb = 0;
    GPT_obj3.current_channel_cb[0] = GPT_obj3.current_channel_cb[1] = 0;

    /* NÃO HÁ MAIS NADA A IMPLEMENTAR AQUI */
}

uint8_t get_gpt_overflows(GPT_t *gptp){
    return gptp->max_nbr_overflows;
}

/*
 * Função para iniciar a contagem do temporizador (mas não as
 * notificações ou geração de sinal PWM).  Aqui deve ser configurado,
 * de acordo com os valores passados no parâmetro config, o modo de
 * contagem e, dependendo do modo, o topo da contagem.
 */
void gpt_start(GPT_t *gptp, const GPT_Config *config) {
    uint8_t MODE_MSK_A = 0b00000011;
    uint8_t MODE_MSK_B = 0b00001000;
    uint8_t DIV_MSK = 0b00000111;

    gptp->regs->tcnt &= ~(0xFF);
    gptp->current_mode = config->mode; // modo de operação
    gptp->divisor = config->divisor;   // setando o divisor
    gptp->regs->ocra = config->top;   // valor topo da contagem
    gptp->max_nbr_overflows = config->max_overlows;

    gptp->regs->tccra &= ~(MODE_MSK_A);
    gptp->regs->tccrb &= ~(MODE_MSK_B);
    switch (gptp->current_mode)
    {
        case MODE_NORMAL: // 0 00 
            gptp->regs->tccra |= (0x00 & MODE_MSK_A);
        case MODE_CTC: //0 10
            gptp->regs->tccra |= (0x02 & MODE_MSK_A);
        case MODE_FAST_PWM_MAX_TOP: // 0 11
            gptp->regs->tccra |= (0x03 & MODE_MSK_A);
        case MODE_FAST_PWM_USER_TOP: //1 01
            gptp->regs->tccra |= (0x03 & MODE_MSK_A);
            gptp->regs->tccrb |= (MODE_MSK_B);  
        default:
            gptp->regs->tccra |= (0x00 & MODE_MSK_A);
    }

    gptp->regs->tccrb &= ~(DIV_MSK);   
    switch (gptp->divisor)
    {
        case DIVISOR_1: // 001
            gptp->regs->tccrb |= (0x01 & DIV_MSK);
        case DIVISOR_8: // 010
            gptp->regs->tccrb |= (0x02 & DIV_MSK);
        case DIVISOR_64: // 011
            gptp->regs->tccrb |= (0x03 & DIV_MSK);
        case DIVISOR_256: // 100
            gptp->regs->tccrb |= (0x04 & DIV_MSK);
        case DIVISOR_1024: // 101
            gptp->regs->tccrb |= (0x05 & DIV_MSK);
        default:
            gptp->regs->tccrb |= (0x01 & DIV_MSK);
    }

    /* ACHO QUE TÁ OK */
}

/*
 * Função para parar a contagem do temporizador (as notificações e
 * geração de sinal PWM, apesar de continuarem habilitadas, não mais
 * ocorrerão pelo contador estar parado)
 */
void gpt_stop(GPT_t *gptp) {
    uint8_t CS_MSK = 0b00000111;

    gptp->regs->tccrb &= ~(CS_MSK);
    gptp->regs->tcnt &= ~(0xFF);

    /* ACHO QUE TÁ OK */
}

/*
 * Função para iniciar as notificações de reinício da contagem (timer
 * overflow).
 *
 * Essas notificações consistem na chamada da função passada como
 * parâmetro no ponteiro cb(). Se o parâmetro is_oneshot for diferente
 * de 0, a função é chamada uma única vez (apenas uma notificação
 * ocorrerá)
 */
uint8_t gpt_start_notification(GPT_t *gptp, gpt_cb_t cb, uint8_t is_oneshot) {
    gptp->current_cb = cb;
    gptp->is_oneshot = is_oneshot;

    *gptp->tifr |= (1 << TOV0);
    *gptp->timsk |= (1 << TOIE0);

    /* NÃO HÁ MAIS NADA A IMPLEMENTAR AQUI */
}

/*
 * Função para interromper as notificações.  O temporizador ainda deve
 * continuar a contar, mas as notificações não mais ocorrerão.
 */
uint8_t gpt_stop_notification(GPT_t *gptp) {
    *gptp->timsk &= ~(1 << TOIE0);

    /* NÃO HÁ MAIS NADA A IMPLEMENTAR AQUI */
}

/*
 * Função para mudar o intervalo de ocorrência da notificação de
 * reinício da contagem, o que é o mesmo do topo da contagem.  Observe
 * que se o modo for o modo normal de contagem, este valor não pode
 * ser mudado e deve ser ignorado.
 */
uint8_t gpt_change_interval(GPT_t *gptp, uint8_t interval, uint8_t nbr_overflows) {
    gptp->max_nbr_overflows = nbr_overflows;
    if (gptp->current_mode != MODE_NORMAL)
        gptp->regs->ocra = interval;

    /* ACHO QUE TÁ OK */
}

/*
 * Função para iniciar a geração do sinal PWM em um dos canais do
 * temporizador. A duração do valor lógico 1 do sinal PWM em pulsos do
 * sinal de relógio é dado pelo parâmetro width, de modo que o ciclo
 * de trabalho será a razão entre o valor de width e o valor do topo
 * da contagem mais 1.
 *
 * Observe que os temporizadores do ATmega328p tem 2 canais cada. O
 * parâmetro channel é 0 para o primeiro canal e 1 para o segundo.
 */
uint8_t gpt_enable_pwm_channel(GPT_t *gptp, uint8_t channel, uint8_t width) {
    uint8_t top = gptp->regs->ocra;

    *gptp->timsk |= (1 << TOIE0);
    if (!channel) {
        gptp->regs->ocra = width / (top + 1);
        *gptp->timsk |= (1 << OCIE0A);
    }else{
        gptp->regs->ocrb = width / (top + 1);
        *gptp->timsk |= (1 << OCIE0B);
    }
}

/*
 * Função para parar a geração do sinal PWM no canal
 * especificado. Observe que o contador do temporizador deve continuar
 * a contar.
 */
void gpt_disable_pwm_channel(GPT_t *gptp, uint8_t channel) {
    *gptp->timsk &= ~(1 << TOIE0);
    if (!channel)
        *gptp->timsk &= ~(1 << OCIE0A);
    else
        *gptp->timsk &= ~(1 << OCIE0B);

    /* ACHO QUE TÁ OK */
}

/*
 * Função para iniciar as notificações de igualdade na comparação
 * (compare match).
 *
 * Essas notificações consistem na chamada da função passada como
 * parâmetro no ponteiro cb(). Se o parâmetro is_oneshot for diferente
 * de 0, a função é chamada uma única vez (apenas uma notificação
 * ocorrerá)
 */
uint8_t gpt_start_channel_notification(GPT_t *gptp, uint8_t channel,
                                       uint8_t interval,
                                       gpt_cb_t cb, uint8_t is_oneshot) {
    gptp->current_cb = cb;
    gptp->is_oneshot = is_oneshot;
    if (!is_oneshot)
        gptp->current_cb(gptp);
    gptp->regs->ocra = interval;

    if (channel) {// channel 1 é a interrupção do OCIE0B 
        *gptp->tifr |= (1 << OCF0B);
        *gptp->timsk |= (1 << OCIE0B);
    }else{
        *gptp->tifr |= (1 << OCF0A);
        *gptp->timsk |= (1 << OCIE0A);
    }
}

/*
 * Função para parar as notificações de igualdade na comparação
 * (compare match). Obserqve que o contador deve continuar a contar.
 */
void gpt_stop_channel_notification(GPT_t *gptp, uint8_t channel) {
    if (channel) // channel 1 é a interrupção do OCIE0B 
        *gptp->timsk &= ~(1 << OCIE0B);
    else
        *gptp->timsk &= ~(1 << OCIE0A);

    /* ACHO QUE TÁ OK */
}


/*
 * Rotina de tratamento da interrupção de overflow do temporizador
 * 0. Se o ponteiro para a função de callback for não-nulo, chama a
 * função, e se o callback for oneshot, desativa a interrupção (mas
 * não o temporizador, pois pode ainda estar gerando algum sinal PWM.
 */
ISR(TIMER0_OVF_vect) {
    if (GPT_obj1.current_cb)
        GPT_obj1.current_cb(&GPT_obj1);

    if (GPT_obj1.is_oneshot)
        TIMSK0 &= ~(1 << TOIE0);

    /* NÃO HÁ MAIS NADA A IMPLEMENTAR AQUI */
}

/*
 * Rotina de tratamento da interrupção de comparação do 1o canal. Se o
 * ponteiro para a função de callback do canal for não-nulo, chama a
 * função, e se o callback for oneshot, desativa a interrupção
 */
ISR(TIMER0_COMPA_vect) {
    if (GPT_obj1.current_channel_cb[0])
        GPT_obj1.current_channel_cb[0](&GPT_obj1);

    if (GPT_obj1.is_channel_oneshot[0])
        TIMSK0 &= ~(1 << OCIE0A);

    /* NÃO HÁ MAIS NADA A IMPLEMENTAR AQUI */
}

/*
 * Rotina de tratamento da interrupção de comparação do 2o canal. Se o
 * ponteiro para a função de callback do canal for não-nulo, chama a
 * função, e se o callback for oneshot, desativa a interrupção
 */
ISR(TIMER0_COMPB_vect) {
    if (GPT_obj1.current_channel_cb[1])
        GPT_obj1.current_channel_cb[1](&GPT_obj1);

    if (GPT_obj1.is_channel_oneshot[1])
        TIMSK0 &= ~(1 << OCIE0B);

    /* NÃO HÁ MAIS NADA A IMPLEMENTAR AQUI */
}

/*
 * Rotina de tratamento da interrupção de overflow do temporizador
 * 2. Se o ponteiro para a função de callback for não-nulo, chama a
 * função, e se o callback for oneshot, desativa a interrupção (mas
 * não o temporizador, pois pode ainda estar gerando algum sinal PWM.
 */
ISR(TIMER2_OVF_vect) {
    if (GPT_obj3.current_cb)
        GPT_obj3.current_cb(&GPT_obj3);

    if (GPT_obj3.is_oneshot)
        TIMSK2 &= ~(1 << TOIE2);

    /* NÃO HÁ MAIS NADA A IMPLEMENTAR AQUI */
}

/*
 * Rotina de tratamento da interrupção de comparação do 1o canal. Se o
 * ponteiro para a função de callback do canal for não-nulo, chama a
 * função, e se o callback for oneshot, desativa a interrupção
 */
ISR(TIMER2_COMPA_vect) {
    if (GPT_obj3.current_channel_cb[0])
        GPT_obj3.current_channel_cb[0](&GPT_obj3);

    if (GPT_obj3.is_channel_oneshot[0])
        TIMSK2 &= ~(1 << OCIE2A);

    /* NÃO HÁ MAIS NADA A IMPLEMENTAR AQUI */
}

/*
 * Rotina de tratamento da interrupção de comparação do 2o canal. Se o
 * ponteiro para a função de callback do canal for não-nulo, chama a
 * função, e se o callback for oneshot, desativa a interrupção
 */
ISR(TIMER2_COMPB_vect) {
    if (GPT_obj3.current_channel_cb[1])
        GPT_obj3.current_channel_cb[1](&GPT_obj3);

    if (GPT_obj3.is_channel_oneshot[1])
        TIMSK2 &= ~(1 << OCIE2B);

    /* NÃO HÁ MAIS NADA A IMPLEMENTAR AQUI */
}