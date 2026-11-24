//Step 1. Initialization
//Step 2. ROM Command (followed by any required data exchange)
//Step 3. DS18B20 Function Command (followed by any required data exchange)

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <stddef.h>
#include "hardware/gpio.h"

#define DS18B20_PIN 15
#define DS18B20_SKIP_ROM 0xCC
#define DS18B20_CONVERT_T 0x44
#define DS18B20_READ_SCRATCHPAD 0xBE
#define RAW_TO_CELSIUS_CONVERSION 0.0625

static uint8_t onewire_read_byte(void)
{
    uint8_t value = 0;
    for (int i = 0; i < 8; i++) {
        gpio_put(DS18B20_PIN, 0);
        gpio_set_dir(DS18B20_PIN, GPIO_OUT);
        sleep_us(6);
        gpio_set_dir(DS18B20_PIN, GPIO_IN);
        sleep_us(9);
        value >>= 1;
        if (gpio_get(DS18B20_PIN)) {
            value |= 0x80;
        }
        sleep_us(55);
    }
    return value;
}

static uint8_t onewire_crc8(const uint8_t *data, size_t length)
{
    uint8_t crc = 0;

    for (size_t i = 0; i < length; i++) {
        uint8_t byte = data[i];

        for (int bit = 0; bit < 8; bit++) {
            uint8_t mix = (crc ^ byte) & 1;
            crc >>= 1;
            if (mix) {
                crc ^= 0x8C;
            }
            byte >>= 1;
        }
    }

    return crc;
}

//Step 1. Initialization of the DS18B20 sensor
bool onewire_reset(void) {
    gpio_set_dir(DS18B20_PIN, GPIO_OUT);
    gpio_put(DS18B20_PIN, 0);
    sleep_us(500); //Reset pulse minimum duration is 480 microseconds

    gpio_set_dir(DS18B20_PIN, GPIO_IN);
    sleep_us(70); //Wait for presence pulse 15-60 microseconds

    bool presence = !gpio_get(DS18B20_PIN);
    sleep_us(410); //DS18B20 presence pulse duration is 60-240 microseconds
    return presence;
}
 
//Step 2. ROM Command (followed by any required data exchange)
static void onewire_write_bit(bool bit)
{
    gpio_put(DS18B20_PIN, 0);
    gpio_set_dir(DS18B20_PIN, GPIO_OUT);

    if (bit) {
        sleep_us(6);  // Brief low pulse means 1
        gpio_set_dir(DS18B20_PIN, GPIO_IN);
        sleep_us(64);
    } else {
        sleep_us(60); // Longer low pulse means 0
        gpio_set_dir(DS18B20_PIN, GPIO_IN);
        sleep_us(10);
    }
}

static void onewire_write_byte(uint8_t value)
{
    for (int i = 0; i < 8; i++) {
        onewire_write_bit(value & 1);
        value >>= 1;
    }
}

//Step 3. DS18B20 Function Command (followed by any required data exchange)
void READ_TEMP_C(void) {
    if (onewire_reset()) {
        onewire_write_byte(DS18B20_SKIP_ROM); // Skip ROM: single sensor
        onewire_write_byte(DS18B20_CONVERT_T); // Convert T
        sleep_ms(750); // Wait for conversion to complete (up to 750ms for 12-bit resolution)
        if (onewire_reset()) {
            onewire_write_byte(DS18B20_SKIP_ROM); // Skip ROM: single sensor
            onewire_write_byte(DS18B20_READ_SCRATCHPAD); // Read Scratchpad
            // Read 9 bytes from the scratchpad (temperature + CRC)
            uint8_t scratchpad[9];
            for (int i = 0; i < 9; i++) {
               scratchpad[i] = onewire_read_byte();
            }
            // Verify the CRC8
            
            // Calculate the temperature from the scratchpad
            if (onewire_crc8(scratchpad, 8) == scratchpad[8]) {
                int16_t raw = (int16_t)((scratchpad[1] << 8) | scratchpad[0]);
                float temperature = raw * RAW_TO_CELSIUS_CONVERSION;
                printf("Temperature: %.2f C\n", temperature);
} else {
    printf("Scratchpad CRC mismatch\n");
}
        }
    } else {
        printf("DS18B20 not detected\n");
    }
}

