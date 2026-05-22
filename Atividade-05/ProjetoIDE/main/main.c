#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#define LED_GPIO        GPIO_NUM_2
#define BUTTON_GPIO     GPIO_NUM_4

#define DEBOUNCE_MS     50
#define TIMEOUT_MS      10000

void app_main(void)
{
    gpio_config_t led_cfg = {
        .pin_bit_mask = (1ULL << LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&led_cfg);

    gpio_config_t btn_cfg = {
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&btn_cfg);

    bool led_state = false;
    gpio_set_level(LED_GPIO, 0);

    int last_stable = 1;
    int last_read = 1;
    int64_t last_change_time = 0;
    int64_t led_on_time = 0;

    printf("Sistema iniciado.\n");
    printf("LED inicia apagado.\n");
    printf("Pressione o botao para alternar o LED.\n");
    printf("Desligamento automatico em 10 segundos.\n");

    while (1) {
        int reading = gpio_get_level(BUTTON_GPIO);
        int64_t now_ms = esp_timer_get_time() / 1000;

        if (reading != last_read) {
            last_change_time = now_ms;
            last_read = reading;
        }

        if ((now_ms - last_change_time) >= DEBOUNCE_MS) {
            if (reading != last_stable) {
                last_stable = reading;

                // Botão pressionado: pull-up interno => nível 0
                if (last_stable == 0) {
                    led_state = !led_state;
                    gpio_set_level(LED_GPIO, led_state);

                    if (led_state) {
                        led_on_time = now_ms;
                        printf("LED ligado.\n");
                    } else {
                        printf("LED desligado manualmente.\n");
                    }
                }
            }
        }

        if (led_state && (now_ms - led_on_time >= TIMEOUT_MS)) {
            led_state = false;
            gpio_set_level(LED_GPIO, 0);
            printf("LED desligado automaticamente apos 10 segundos.\n");
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}