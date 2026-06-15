#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"

#define TAG "ATV04"

// LEDs
#define LED0 GPIO_NUM_4
#define LED1 GPIO_NUM_5
#define LED2 GPIO_NUM_6
#define LED3 GPIO_NUM_7

// Botões
#define BTN_A GPIO_NUM_8   // incrementa contador
#define BTN_B GPIO_NUM_9   // alterna passo entre +1 e +2

#define DEBOUNCE_TIME_US 50000  // 50 ms

typedef struct {
    gpio_num_t pin;
    int raw_state;
    int stable_state;
    int last_stable_state;
    int64_t last_transition_time;
} button_t;

static uint8_t contador = 0;
static uint8_t passo = 1;

static button_t botao_a;
static button_t botao_b;

static void configurar_leds(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED0) |
                        (1ULL << LED1) |
                        (1ULL << LED2) |
                        (1ULL << LED3),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
}

static void configurar_botoes(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BTN_A) | (1ULL << BTN_B),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    botao_a.pin = BTN_A;
    botao_a.raw_state = gpio_get_level(BTN_A);
    botao_a.stable_state = botao_a.raw_state;
    botao_a.last_stable_state = botao_a.raw_state;
    botao_a.last_transition_time = esp_timer_get_time();

    botao_b.pin = BTN_B;
    botao_b.raw_state = gpio_get_level(BTN_B);
    botao_b.stable_state = botao_b.raw_state;
    botao_b.last_stable_state = botao_b.raw_state;
    botao_b.last_transition_time = esp_timer_get_time();
}

static void atualizar_leds(uint8_t valor) {
    gpio_set_level(LED0, (valor >> 0) & 0x01);
    gpio_set_level(LED1, (valor >> 1) & 0x01);
    gpio_set_level(LED2, (valor >> 2) & 0x01);
    gpio_set_level(LED3, (valor >> 3) & 0x01);
}

static bool detectar_pressionamento(button_t *botao) {
    int leitura = gpio_get_level(botao->pin);
    int64_t agora = esp_timer_get_time();

    if (leitura != botao->raw_state) {
        botao->raw_state = leitura;
        botao->last_transition_time = agora;
    }

    if ((agora - botao->last_transition_time) >= DEBOUNCE_TIME_US) {
        if (botao->stable_state != leitura) {
            botao->last_stable_state = botao->stable_state;
            botao->stable_state = leitura;

            // Evento de pressionamento: 1 -> 0
            if (botao->last_stable_state == 1 && botao->stable_state == 0) {
                return true;
            }
        }
    }

    return false;
}

void app_main(void) {
    configurar_leds();
    configurar_botoes();
    atualizar_leds(contador);

    ESP_LOGI(TAG, "Sistema iniciado");
    ESP_LOGI(TAG, "Contador inicial = 0x%X (%d)", contador, contador);
    ESP_LOGI(TAG, "Passo inicial = %d", passo);

    while (1) {
        if (detectar_pressionamento(&botao_a)) {
            contador = (contador + passo) & 0x0F;
            atualizar_leds(contador);

            ESP_LOGI(TAG, "Botao A pressionado -> passo=%d | contador=0x%X (%d)",
                     passo, contador, contador);
        }

        if (detectar_pressionamento(&botao_b)) {
            passo = (passo == 1) ? 2 : 1;

            ESP_LOGI(TAG, "Botao B pressionado -> novo passo=%d", passo);
        }

        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
