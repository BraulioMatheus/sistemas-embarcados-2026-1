#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"

#define LED1_GPIO 4
#define LED2_GPIO 5
#define LED3_GPIO 6
#define LED4_GPIO 7
#define BUZZER_GPIO 8

#define LED_PWM_FREQ_HZ       1000
#define BUZZER_INIT_FREQ_HZ   1000

#define LEDC_MODE             LEDC_LOW_SPEED_MODE

#define LED_TIMER             LEDC_TIMER_0
#define BUZZER_TIMER          LEDC_TIMER_1

#define LED_DUTY_RES          LEDC_TIMER_10_BIT
#define BUZZER_DUTY_RES       LEDC_TIMER_10_BIT

#define LED_CH1               LEDC_CHANNEL_0
#define LED_CH2               LEDC_CHANNEL_1
#define LED_CH3               LEDC_CHANNEL_2
#define LED_CH4               LEDC_CHANNEL_3
#define BUZZER_CH             LEDC_CHANNEL_4

#define PWM_MAX_DUTY          ((1 << 10) - 1)

#define DELAY_MS              30
#define PHASE_DELAY_MS        500
#define BUZZER_STEP_HZ        100
#define BUZZER_TONE_DELAY_MS  150

static const int led_gpios[4] = {LED1_GPIO, LED2_GPIO, LED3_GPIO, LED4_GPIO};
static const ledc_channel_t led_channels[4] = {
    LED_CH1, LED_CH2, LED_CH3, LED_CH4
};

static uint32_t percent_to_duty(float percent) {
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;
    return (uint32_t)((percent / 100.0f) * PWM_MAX_DUTY);
}

static void pwm_init(void) {
    ledc_timer_config_t led_timer = {
        .speed_mode = LEDC_MODE,
        .duty_resolution = LED_DUTY_RES,
        .timer_num = LED_TIMER,
        .freq_hz = LED_PWM_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&led_timer));

    ledc_timer_config_t buzzer_timer = {
        .speed_mode = LEDC_MODE,
        .duty_resolution = BUZZER_DUTY_RES,
        .timer_num = BUZZER_TIMER,
        .freq_hz = BUZZER_INIT_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&buzzer_timer));

    for (int i = 0; i < 4; i++) {
        ledc_channel_config_t led_channel = {
            .gpio_num = led_gpios[i],
            .speed_mode = LEDC_MODE,
            .channel = led_channels[i],
            .intr_type = LEDC_INTR_DISABLE,
            .timer_sel = LED_TIMER,
            .duty = 0,
            .hpoint = 0
        };
        ESP_ERROR_CHECK(ledc_channel_config(&led_channel));
    }

    ledc_channel_config_t buzzer_channel = {
        .gpio_num = BUZZER_GPIO,
        .speed_mode = LEDC_MODE,
        .channel = BUZZER_CH,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = BUZZER_TIMER,
        .duty = 0,
        .hpoint = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&buzzer_channel));
}

static void set_led_percent(int led_index, float percent) {
    if (led_index < 0 || led_index > 3) return;

    uint32_t duty = percent_to_duty(percent);
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, led_channels[led_index], duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, led_channels[led_index]));
}

static void set_all_leds(float percent) {
    for (int i = 0; i < 4; i++) {
        set_led_percent(i, percent);
    }
}

static void leds_off(void) {
    set_all_leds(0.0f);
}

static void buzzer_set_freq(uint32_t freq_hz) {
    ESP_ERROR_CHECK(ledc_set_freq(LEDC_MODE, BUZZER_TIMER, freq_hz));
}

static void buzzer_on(void) {
    uint32_t duty = PWM_MAX_DUTY / 2; // 50%
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, BUZZER_CH, duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, BUZZER_CH));
}

static void buzzer_off(void) {
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, BUZZER_CH, 0));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, BUZZER_CH));
}

static void fase1_fading_sincronizado(void) {
    printf("Fase 1: Fading sincronizado dos LEDs\n");

    for (int p = 0; p <= 100; p += 5) {
        set_all_leds((float)p);
        vTaskDelay(pdMS_TO_TICKS(DELAY_MS));
    }

    for (int p = 100; p >= 0; p -= 5) {
        set_all_leds((float)p);
        vTaskDelay(pdMS_TO_TICKS(DELAY_MS));
    }

    leds_off();
    vTaskDelay(pdMS_TO_TICKS(PHASE_DELAY_MS));
}

static void fade_led_individual(int led_index) {
    for (int p = 0; p <= 100; p += 5) {
        set_led_percent(led_index, (float)p);
        vTaskDelay(pdMS_TO_TICKS(DELAY_MS));
    }

    for (int p = 100; p >= 0; p -= 5) {
        set_led_percent(led_index, (float)p);
        vTaskDelay(pdMS_TO_TICKS(DELAY_MS));
    }

    set_led_percent(led_index, 0.0f);
}

static void fase2_fading_sequencial(void) {
    printf("Fase 2: Fading sequencial dos LEDs\n");

    for (int i = 0; i < 4; i++) {
        fade_led_individual(i);
    }

    vTaskDelay(pdMS_TO_TICKS(PHASE_DELAY_MS));
}

static void fase3_teste_sonoro(void) {
    printf("Fase 3: Teste sonoro com o buzzer\n");

    buzzer_on();

    for (int freq = 500; freq <= 2000; freq += BUZZER_STEP_HZ) {
        buzzer_set_freq(freq);
        vTaskDelay(pdMS_TO_TICKS(BUZZER_TONE_DELAY_MS));
    }

    for (int freq = 2000; freq >= 500; freq -= BUZZER_STEP_HZ) {
        buzzer_set_freq(freq);
        vTaskDelay(pdMS_TO_TICKS(BUZZER_TONE_DELAY_MS));
    }

    buzzer_off();
    vTaskDelay(pdMS_TO_TICKS(PHASE_DELAY_MS));
}

void app_main(void) {
    pwm_init();

    leds_off();
    buzzer_set_freq(BUZZER_INIT_FREQ_HZ);
    buzzer_off();

    printf("Sistema iniciado.\n");
    printf("LEDs apagados.\n");
    printf("Frequencia inicial do buzzer: %d Hz\n", BUZZER_INIT_FREQ_HZ);

    while (1) {
        fase1_fading_sincronizado();
        fase2_fading_sequencial();
        fase3_teste_sonoro();
    }
}
