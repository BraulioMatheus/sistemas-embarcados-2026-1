#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"

#define ADC_CHANNEL ADC_CHANNEL_0   // GPIO1
#define BUTTON_GPIO GPIO_NUM_4
#define LED_GPIO GPIO_NUM_18

#define LIVE 0
#define HOLD 1

int state = LIVE;
int last_adc = 0;

adc_oneshot_unit_handle_t adc_handle;

// ================= ADC =================
void config_adc() {
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    adc_oneshot_new_unit(&init_config, &adc_handle);

    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12
    };

    adc_oneshot_config_channel(adc_handle, ADC_CHANNEL, &config);
}

// ================= PWM =================
void config_pwm() {
    ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer);

    ledc_channel_config_t channel = {
        .gpio_num = LED_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&channel);
}

// ================= BOTÃO =================
void config_button() {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = 0, // usando resistor externo (r2)
        .pull_up_en = 0,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
}

// ================= MAIN =================
void app_main() {
    config_adc();
    config_pwm();
    config_button();

    int button_last = 0;

    while (1) {
        int button = gpio_get_level(BUTTON_GPIO);

        // Alterna HOLD/LIVE (detecção de borda)
        if (button == 1 && button_last == 0) {
            state = !state;
            vTaskDelay(pdMS_TO_TICKS(200)); // debounce simples
        }
        button_last = button;

        int adc_value;

        if (state == LIVE) {
            adc_oneshot_read(adc_handle, ADC_CHANNEL, &adc_value);
            last_adc = adc_value;
        } else {
            adc_value = last_adc;
        }

        // Conversão
        int voltage = (adc_value * 3300) / 4095;
        int duty = (adc_value * 8191) / 4095;

        // PWM
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

        // Monitoramento
        printf("ADC: %d | Tensão: %d mV | Estado: %s\n",
               adc_value,
               voltage,
               state == LIVE ? "LIVE" : "HOLD");

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}