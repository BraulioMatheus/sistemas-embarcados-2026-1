#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/i2c.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"

#define LED_GPIO               4
#define BUTTON_GPIO            15

#define I2C_MASTER_SCL_IO      18
#define I2C_MASTER_SDA_IO      17
#define I2C_MASTER_NUM         I2C_NUM_0

// Clock reduzido para maior estabilidade
#define I2C_MASTER_FREQ_HZ     10000

#define MPU6050_ADDR           0x68

#define PWM_FREQ               5000
#define PWM_RESOLUTION         LEDC_TIMER_13_BIT
#define PWM_MAX_DUTY           8191

#define POT_ADC_CHANNEL        ADC_CHANNEL_0

QueueHandle_t potQueue;
SemaphoreHandle_t buttonSemaphore;
SemaphoreHandle_t imuMutex;

adc_oneshot_unit_handle_t adc_handle;

typedef struct {
    float ax;
    float ay;
    float az;
} imu_data_t;

imu_data_t imuData;

volatile bool holdMode = false;
volatile uint32_t lastDuty = 0;
volatile uint32_t lastADC = 0;

void pwm_init()
{
    ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = PWM_RESOLUTION,
        .freq_hz = PWM_FREQ,
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

void adc_init_custom()
{
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };

    adc_oneshot_new_unit(
        &init_config,
        &adc_handle
    );

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_12,
    };

    adc_oneshot_config_channel(
        adc_handle,
        POT_ADC_CHANNEL,
        &config
    );
}

void button_init()
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    gpio_config(&io_conf);
}

void i2c_master_init()
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    i2c_param_config(I2C_MASTER_NUM, &conf);

    esp_err_t err = i2c_driver_install(
        I2C_MASTER_NUM,
        conf.mode,
        0,
        0,
        0
    );

    if (err == ESP_OK) {

        printf("I2C iniciado com sucesso\n");

    } else {

        printf(
            "Erro ao iniciar I2C: %s\n",
            esp_err_to_name(err)
        );
    }
}

void i2c_scanner()
{
    printf("=====================================\n");
    printf("Escaneando barramento I2C...\n");

    for (uint8_t addr = 1; addr < 127; addr++) {

        i2c_cmd_handle_t cmd =
            i2c_cmd_link_create();

        i2c_master_start(cmd);

        i2c_master_write_byte(
            cmd,
            (addr << 1) | I2C_MASTER_WRITE,
            true
        );

        i2c_master_stop(cmd);

        esp_err_t err =
            i2c_master_cmd_begin(
                I2C_MASTER_NUM,
                cmd,
                pdMS_TO_TICKS(50)
            );

        i2c_cmd_link_delete(cmd);

        if (err == ESP_OK) {

            printf(
                "Dispositivo encontrado: 0x%02X\n",
                addr
            );
        }
    }

    printf("Fim do scan I2C\n");
    printf("=====================================\n");
}

void mpu6050_write(uint8_t reg, uint8_t data)
{
    uint8_t write_buf[2] = {reg, data};

    esp_err_t err = i2c_master_write_to_device(
        I2C_MASTER_NUM,
        MPU6050_ADDR,
        write_buf,
        sizeof(write_buf),
        pdMS_TO_TICKS(200)
    );

    if (err != ESP_OK) {

        printf(
            "ERRO I2C WRITE: %s\n",
            esp_err_to_name(err)
        );
    }
}

bool mpu6050_read(
    uint8_t reg,
    uint8_t *data,
    size_t len
)
{
    for (int tentativas = 0;
         tentativas < 3;
         tentativas++) {

        esp_err_t err =
            i2c_master_write_read_device(
                I2C_MASTER_NUM,
                MPU6050_ADDR,
                &reg,
                1,
                data,
                len,
                pdMS_TO_TICKS(200)
            );

        if (err == ESP_OK) {

            return true;
        }

        printf(
            "Falha I2C tentativa %d: %s\n",
            tentativas + 1,
            esp_err_to_name(err)
        );

        vTaskDelay(pdMS_TO_TICKS(20));
    }

    return false;
}

void mpu6050_init()
{
    printf("Inicializando MPU6050...\n");

    // Wake up
    mpu6050_write(0x6B, 0x00);

    vTaskDelay(pdMS_TO_TICKS(100));

    // Escala ±2g
    mpu6050_write(0x1C, 0x00);

    vTaskDelay(pdMS_TO_TICKS(100));

    printf("MPU6050 inicializado\n");
}

void task_pot(void *pvParameters)
{
    while (1) {

        int adc_raw = 0;

        adc_oneshot_read(
            adc_handle,
            POT_ADC_CHANNEL,
            &adc_raw
        );

        lastADC = adc_raw;

        uint32_t duty =
            (adc_raw * PWM_MAX_DUTY) / 4095;

        xQueueSend(
            potQueue,
            &duty,
            portMAX_DELAY
        );

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void task_led(void *pvParameters)
{
    uint32_t duty;

    while (1) {

        if (xSemaphoreTake(buttonSemaphore, 0)) {

            holdMode = !holdMode;

            printf(
                "Modo alterado: %s\n",
                holdMode ? "HOLD" : "LIVE"
            );
        }

        if (!holdMode) {

            if (xQueueReceive(
                    potQueue,
                    &duty,
                    pdMS_TO_TICKS(50))) {

                lastDuty = duty;

                ledc_set_duty(
                    LEDC_LOW_SPEED_MODE,
                    LEDC_CHANNEL_0,
                    duty
                );

                ledc_update_duty(
                    LEDC_LOW_SPEED_MODE,
                    LEDC_CHANNEL_0
                );
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void task_button(void *pvParameters)
{
    int lastState = 0;

    while (1) {

        int currentState =
            gpio_get_level(BUTTON_GPIO);

        if (currentState == 1 &&
            lastState == 0) {

            xSemaphoreGive(
                buttonSemaphore
            );

            vTaskDelay(
                pdMS_TO_TICKS(200)
            );
        }

        lastState = currentState;

        vTaskDelay(
            pdMS_TO_TICKS(20)
        );
    }
}

void task_imu(void *pvParameters)
{
    uint8_t data[6];

    while (1) {

        bool ok =
            mpu6050_read(0x3B, data, 6);

        if (ok) {

            int16_t ax_raw =
                (data[0] << 8) | data[1];

            int16_t ay_raw =
                (data[2] << 8) | data[3];

            int16_t az_raw =
                (data[4] << 8) | data[5];

            float ax =
                ax_raw / 16384.0f;

            float ay =
                ay_raw / 16384.0f;

            float az =
                az_raw / 16384.0f;

            xSemaphoreTake(
                imuMutex,
                portMAX_DELAY
            );

            imuData.ax = ax;
            imuData.ay = ay;
            imuData.az = az;

            xSemaphoreGive(
                imuMutex
            );

        } else {

            printf(
                "MPU6050 desconectado ou falha I2C\n"
            );

            mpu6050_init();
        }

        vTaskDelay(
            pdMS_TO_TICKS(300)
        );
    }
}

void task_console(void *pvParameters)
{
    imu_data_t localIMU;

    while (1) {

        xSemaphoreTake(
            imuMutex,
            portMAX_DELAY
        );

        localIMU = imuData;

        xSemaphoreGive(
            imuMutex
        );

        int mv =
            (lastADC * 3300) / 4095;

        int percent =
            (lastDuty * 100) / PWM_MAX_DUTY;

        printf("\n");
        printf("=====================================================\n");

        printf(
            "STATUS: [%s] | POT: %lu (%d mV) | LED: %d%%\n",
            holdMode ? "HOLD" : "LIVE",
            (unsigned long)lastADC,
            mv,
            percent
        );

        printf(
            "IMU ACCEL (g): X: %.2f | Y: %.2f | Z: %.2f\n",
            localIMU.ax,
            localIMU.ay,
            localIMU.az
        );

        printf("=====================================================\n");

        vTaskDelay(
            pdMS_TO_TICKS(1000)
        );
    }
}

void app_main()
{
    printf("=====================================\n");
    printf("Inicializando sistema...\n");
    printf("=====================================\n");

    adc_init_custom();

    printf("ADC OK\n");

    pwm_init();

    printf("PWM OK\n");

    button_init();

    printf("BUTTON OK\n");

    i2c_master_init();

    i2c_scanner();

    mpu6050_init();

    potQueue =
        xQueueCreate(
            5,
            sizeof(uint32_t)
        );

    buttonSemaphore =
        xSemaphoreCreateBinary();

    imuMutex =
        xSemaphoreCreateMutex();

    xTaskCreate(
        task_pot,
        "TaskPot",
        2048,
        NULL,
        2,
        NULL
    );

    printf("Task POT criada\n");

    xTaskCreate(
        task_led,
        "TaskLED",
        2048,
        NULL,
        3,
        NULL
    );

    printf("Task LED criada\n");

    xTaskCreate(
        task_button,
        "TaskButton",
        2048,
        NULL,
        4,
        NULL
    );

    printf("Task BUTTON criada\n");

    xTaskCreate(
        task_imu,
        "TaskIMU",
        4096,
        NULL,
        2,
        NULL
    );

    printf("Task IMU criada\n");

    xTaskCreate(
        task_console,
        "TaskConsole",
        4096,
        NULL,
        1,
        NULL
    );

    printf("Task CONSOLE criada\n");

    printf("=====================================\n");
    printf("Sistema iniciado\n");
    printf("=====================================\n");
}