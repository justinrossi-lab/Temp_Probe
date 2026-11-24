#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "Temp_Reading.h"

#define DS18B20_PIN 15

int main(void) 
{
    stdio_init_all();

    gpio_init(DS18B20_PIN);
    gpio_pull_up(DS18B20_PIN);
    
    while (true) {
        READ_TEMP_C();
        sleep_ms(250);
    }
}