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

bool led_state = false;
int64_t led_deadline = 0;

// =====================================================
// TIMER CALLBACK
// =====================================================

void led_timer_callback(TimerHandle_t xTimer)
{
    gpio_set_level(LED_GPIO, 0);
    led_state = false;

    ESP_LOGI(TAG, "[TIMER] Tempo expirado -> LED DESLIGADO");
}

// =====================================================
// ISR
// =====================================================

static void IRAM_ATTR button_isr_handler(void *arg)
{
    int gpio_num = (int)arg;

    int64_t now = esp_timer_get_time() / 1000;

    // Debounce
    if ((now - last_interrupt_time) < DEBOUNCE_MS)
    {
        return;
    }

    last_interrupt_time = now;

    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

// =====================================================
// TASK PRINCIPAL DE EVENTOS
// =====================================================

void button_task(void *arg)
{
    int io_num;

    while (1)
    {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY))
        {
            int level = gpio_get_level(BUTTON_GPIO);
            int64_t now = esp_timer_get_time() / 1000;

            // =========================================
            // BOTÃO PRESSIONADO
            // =========================================

            if (level == 0)
            {
                button_press_time = now;

                ESP_LOGI(TAG,
                         "[PRESS] Botão pressionado em %lld ms",
                         now);
            }
            else
            {
                // =====================================
                // BOTÃO SOLTO
                // =====================================

                int64_t press_duration = now - button_press_time;

                ESP_LOGI(TAG,
                         "[RELEASE] Botão solto em %lld ms | duração = %lld ms",
                         now,
                         press_duration);

                // =====================================
                // LONG PRESS
                // =====================================

                if (press_duration >= LONG_PRESS_MS)
                {
                    gpio_set_level(LED_GPIO, 0);
                    led_state = false;

                    xTimerStop(led_timer, 0);

                    ESP_LOGW(TAG,
                             "[LONG PRESS] LED desligado imediatamente");
                }
                else
                {
                    // =================================
                    // CLIQUE CURTO
                    // =================================

                    gpio_set_level(LED_GPIO, 1);
                    led_state = true;

                    xTimerReset(led_timer, 0);

                    led_deadline = now + LED_TIMEOUT_MS;

                    ESP_LOGI(TAG,
                             "[SHORT CLICK] LED ligado | Timer reiniciado para 10s");
                }
            }
        }
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
            int64_t now = esp_timer_get_time() / 1000;
            int64_t remaining = led_deadline - now;

            if (remaining < 0)
                remaining = 0;

            ESP_LOGI(TAG,
                     "[STATUS] LED ON | Tempo restante: %.1f s",
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
    // LED
    // =============================================

    gpio_config_t led_config = {
        .pin_bit_mask = (1ULL << LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    gpio_config(&led_config);

    // =============================================
    // BOTÃO
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

    gpio_evt_queue = xQueueCreate(10, sizeof(int));

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
        monitor_task,
        "monitor_task",
        4096,
        NULL,
        5,
        NULL
    );

    ESP_LOGI(TAG, "Sistema iniciado");
}
