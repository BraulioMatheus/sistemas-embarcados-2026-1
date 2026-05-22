#include <stdio.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/queue.h"

#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"

#define LED_GPIO            GPIO_NUM_4
#define BUTTON_GPIO         GPIO_NUM_15

#define LED_TIMEOUT_MS      10000
#define LONG_PRESS_MS       2000
#define DEBOUNCE_MS         100

static const char *TAG = "EVENT_CTRL";

static TimerHandle_t led_timer;
static QueueHandle_t gpio_evt_queue;

volatile int64_t last_interrupt_time = 0;
volatile int64_t button_press_time = 0;

volatile bool button_pressed = false;
volatile bool long_press_detected = false;

bool led_state = false;
int64_t led_deadline = 0;

// =====================================================
// CALLBACK DO TIMER
// =====================================================

void led_timer_callback(TimerHandle_t xTimer)
{
    gpio_set_level(LED_GPIO, 0);

    led_state = false;

    ESP_LOGI(TAG,
             "[TIMER] LED desligado após 10 segundos");
}

// =====================================================
// ISR
// =====================================================

static void IRAM_ATTR button_isr_handler(void *arg)
{
    int gpio_num = (int)arg;

    int64_t now =
        esp_timer_get_time() / 1000;

    // Debounce
    if ((now - last_interrupt_time) < DEBOUNCE_MS)
    {
        return;
    }

    last_interrupt_time = now;

    xQueueSendFromISR(gpio_evt_queue,
                      &gpio_num,
                      NULL);
}

// =====================================================
// TASK PRINCIPAL DO BOTÃO
// =====================================================

void button_task(void *arg)
{
    int io_num;

    while (1)
    {
        if (xQueueReceive(gpio_evt_queue,
                          &io_num,
                          portMAX_DELAY))
        {
            int level =
                gpio_get_level(BUTTON_GPIO);

            int64_t now =
                esp_timer_get_time() / 1000;

            // =====================================
            // BOTÃO PRESSIONADO
            // =====================================

            if (level == 0)
            {
                button_pressed = true;

                long_press_detected = false;

                button_press_time = now;

                ESP_LOGI(TAG,
                         "[PRESS] Botão pressionado");

                // =================================
                // LED LIGA IMEDIATAMENTE
                // =================================

                gpio_set_level(LED_GPIO, 1);

                led_state = true;

                // Reinicia timer de 10 segundos
                xTimerReset(led_timer, 0);

                led_deadline =
                    now + LED_TIMEOUT_MS;

                ESP_LOGI(TAG,
                         "[LED] Ligado/Reiniciado por 10s");
            }

            // =====================================
            // BOTÃO SOLTO
            // =====================================

            else
            {
                button_pressed = false;

                ESP_LOGI(TAG,
                         "[RELEASE] Botão solto");
            }
        }
    }
}

// =====================================================
// TASK DE LONG PRESS
// =====================================================

void long_press_task(void *arg)
{
    while (1)
    {
        // Long press só funciona
        // se LED estiver ligado

        if (button_pressed &&
            led_state &&
            !long_press_detected)
        {
            int64_t now =
                esp_timer_get_time() / 1000;

            int64_t duration =
                now - button_press_time;

            // =====================================
            // LONG PRESS
            // =====================================

            if (duration >= LONG_PRESS_MS)
            {
                gpio_set_level(LED_GPIO, 0);

                led_state = false;

                xTimerStop(led_timer, 0);

                long_press_detected = true;

                ESP_LOGW(TAG,
                         "[LONG PRESS] LED desligado imediatamente");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// =====================================================
// TASK DE MONITORAMENTO
// =====================================================

void monitor_task(void *arg)
{
    while (1)
    {
        if (led_state)
        {
            int64_t now =
                esp_timer_get_time() / 1000;

            int64_t remaining =
                led_deadline - now;

            if (remaining < 0)
                remaining = 0;

            ESP_LOGI(TAG,
                     "[STATUS] LED ON | %.1f s restantes",
                     remaining / 1000.0);
        }
        else
        {
            ESP_LOGI(TAG,
                     "[STATUS] LED OFF");
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// =====================================================
// MAIN
// =====================================================

void app_main(void)
{
    // =============================================
    // CONFIGURAÇÃO DO LED
    // =============================================

    gpio_config_t led_config = {
        .pin_bit_mask = (1ULL << LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    gpio_config(&led_config);

    gpio_set_level(LED_GPIO, 0);

    // =============================================
    // CONFIGURAÇÃO DO BOTÃO
    // =============================================

    gpio_config_t button_config = {
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE
    };

    gpio_config(&button_config);

    // =============================================
    // TIMER
    // =============================================

    led_timer = xTimerCreate(
        "led_timer",
        pdMS_TO_TICKS(LED_TIMEOUT_MS),
        pdFALSE,
        NULL,
        led_timer_callback
    );

    // =============================================
    // FILA
    // =============================================

    gpio_evt_queue =
        xQueueCreate(10, sizeof(int));

    // =============================================
    // ISR
    // =============================================

    gpio_install_isr_service(0);

    gpio_isr_handler_add(
        BUTTON_GPIO,
        button_isr_handler,
        (void *)BUTTON_GPIO
    );

    // =============================================
    // TASKS
    // =============================================

    xTaskCreate(
        button_task,
        "button_task",
        4096,
        NULL,
        10,
        NULL
    );

    xTaskCreate(
        long_press_task,
        "long_press_task",
        4096,
        NULL,
        9,
        NULL
    );

    xTaskCreate(
        monitor_task,
        "monitor_task",
        4096,
        NULL,
        5,
        NULL
    );

    ESP_LOGI(TAG,
             "Sistema iniciado");
}