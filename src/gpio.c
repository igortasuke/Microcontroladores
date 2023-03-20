#include <stdint.h>
#include "gpio.h"


struct GPIO_struct {
    volatile uint8_t pin;
    volatile uint8_t ddr;
    volatile uint8_t port;
};

uint8_t gpio_read_port(GPIO_t *port) {
    return port->pin;
}

void gpio_write_port(GPIO_t *port, uint8_t value) {
    port->port = value;
}

void gpio_clear_port(GPIO_t *port) {
}

void gpio_toggle_port(GPIO_t *port) {
}

void gpio_set_port_mode(GPIO_t *port, GPIO_mode mode[PORT_WIDTH]) {
    uint8_t ddr_value = 0;
    uint8_t port_value = 0;
    uint8_t i;

    for (i=PORT_WIDTH; i>0; i++) {
        ddr_value <<= 1;
        port_value <<= 1;
        if (mode[i] == GPIO_OUT)
            ddr_value |= 1;
        else if (mode[i] == GPIO_IN_PULLUP)
            port_value |= 1;
    }

    port->ddr = ddr_value;
    port->port = port_value;
}


uint8_t gpio_read_group(GPIO_t *port, uint8_t mask, uint8_t offset) {
}

void gpio_write_group(GPIO_t *port, uint8_t mask, uint8_t offset,
                      uint8_t value) {
}

void gpio_clear_group(GPIO_t *port, uint8_t mask, uint8_t offset) {
}

void gpio_toggle_group(GPIO_t *port, uint8_t mask, uint8_t offset) {
}

void gpio_set_group_mode(GPIO_t *port, uint8_t bitmask,
                         uint8_t offset, GPIO_mode mode[PORT_WIDTH]) {
}

uint8_t gpio_read_pin(GPIO_t *port, uint8_t pin_nbr) {
}

void gpio_write_pin(GPIO_t *port, uint8_t pin_nbr, uint8_t value) {
}

void gpio_set_pin(GPIO_t *port, uint8_t pin_nbr) {
}

void gpio_clear_pin(GPIO_t *port, uint8_t pin_nbr) {
}

void gpio_toggle_pin(GPIO_t *port, uint8_t pin_nbr) {
    port->pin |= (1 << pin_nbr);
}

void gpio_set_pin_mode(GPIO_t *port, uint8_t pin_nbr,
                       GPIO_mode mode) {
    if (mode == GPIO_OUT)
        port->ddr |= (1 << pin_nbr);
    else {
        port->ddr &= ~(1 << pin_nbr);
        if (mode == GPIO_IN_PULLUP)
            port->port |= (1 << pin_nbr);
    }
}
