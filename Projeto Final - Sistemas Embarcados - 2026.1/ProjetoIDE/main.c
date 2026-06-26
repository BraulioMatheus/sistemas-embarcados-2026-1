#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/FreeRTOSConfig.h"

#include "driver/gpio.h"
#include "driver/ledc.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"

#include <stdbool.h>

#include "driver/i2c.h"

//==========================================================
// I2C / MPU6050
//==========================================================

#define I2C_MASTER_NUM         I2C_NUM_0
#define I2C_MASTER_SDA_IO      8
#define I2C_MASTER_SCL_IO      9
#define I2C_MASTER_FREQ_HZ     400000

#define MPU6050_ADDR           0x68

#define MPU6050_PWR_MGMT_1     0x6B

#define MPU6050_ACCEL_XOUT_H   0x3B
#define MPU6050_ACCEL_YOUT_H   0x3D
#define MPU6050_ACCEL_ZOUT_H   0x3F

#define MPU6050_GYRO_XOUT_H    0x43
#define MPU6050_GYRO_YOUT_H    0x45
#define MPU6050_GYRO_ZOUT_H    0x47

//==========================================================
// PINOS
//==========================================================

#define LED_PIN         48

#define SERVO_X_PIN     17
#define SERVO_Y_PIN     18

// GPIO4 = ADC1_CH3
// GPIO5 = ADC1_CH4
#define JOY_X_CHANNEL   ADC_CHANNEL_3
#define JOY_Y_CHANNEL   ADC_CHANNEL_4

//==========================================================
// SERVO
//==========================================================

#define SERVO_FREQ      50

#define SERVO_MIN_US    500
#define SERVO_MAX_US    2500

#define SERVO_MIN       50.0f
#define SERVO_CENTER    90.0f
#define SERVO_MAX       130.0f

#define SERVO_STEP      0.5f

#define DEADZONE        350

#define OFFSET_X   -3.0f
#define OFFSET_Y   -12.0f

//==========================================================

static adc_oneshot_unit_handle_t adc_handle;

TaskHandle_t joystickHandle = NULL;
TaskHandle_t servoHandle = NULL;
TaskHandle_t serialHandle = NULL;
TaskHandle_t mpuHandle = NULL;

volatile int joy_x = 2048;
volatile int joy_y = 2048;

float filtroX = 2048;
float filtroY = 2048;

#define FILTRO 0.12f

volatile float target_x = 90.0f;
volatile float target_y = 90.0f;

volatile float servo_x = 90.0f;
volatile float servo_y = 90.0f;

volatile bool ledLigado = false;

//==========================================================
// MPU6050
//==========================================================

volatile float pitch = 0.0f;
volatile float roll  = 0.0f;

volatile int16_t accX = 0;
volatile int16_t accY = 0;
volatile int16_t accZ = 0;

volatile int16_t gyroX = 0;
volatile int16_t gyroY = 0;
volatile int16_t gyroZ = 0;

//==========================================================

float mapJoystick(int value)
{
    return SERVO_MIN +
           ((float)value / 4095.0f) *
           (SERVO_MAX - SERVO_MIN);
}

//==========================================================

uint32_t angleToDuty(float angle)
{
    if(angle < SERVO_MIN)
        angle = SERVO_MIN;

    if(angle > SERVO_MAX)
        angle = SERVO_MAX;

    float pulse =
        SERVO_MIN_US +
        (angle / 180.0f) *
        (SERVO_MAX_US - SERVO_MIN_US);

    return (uint32_t)((pulse / 20000.0f) * 16383.0f);
}

//==========================================================

void servoWrite(ledc_channel_t channel, float angle)
{
    if(channel == LEDC_CHANNEL_0)
        angle += OFFSET_X;
    else
        angle += OFFSET_Y;

    ledc_set_duty(
        LEDC_LOW_SPEED_MODE,
        channel,
        angleToDuty(angle));

    ledc_update_duty(
        LEDC_LOW_SPEED_MODE,
        channel);
}

//==========================================================
// I2C
//==========================================================

static esp_err_t i2c_master_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ
    };

    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &conf));

    return i2c_driver_install(
        I2C_MASTER_NUM,
        conf.mode,
        0,
        0,
        0
    );
}

//==========================================================

static esp_err_t mpu_write(uint8_t reg, uint8_t data)
{
    uint8_t buffer[2] = {reg, data};

    return i2c_master_write_to_device(
        I2C_MASTER_NUM,
        MPU6050_ADDR,
        buffer,
        sizeof(buffer),
        pdMS_TO_TICKS(100)
    );
}

//==========================================================

static esp_err_t mpu_read(uint8_t reg, uint8_t *data, size_t len)
{
    return i2c_master_write_read_device(
        I2C_MASTER_NUM,
        MPU6050_ADDR,
        &reg,
        1,
        data,
        len,
        pdMS_TO_TICKS(100)
    );
}

//==========================================================

static void mpu_init(void)
{
    ESP_ERROR_CHECK(mpu_write(MPU6050_PWR_MGMT_1, 0x00));

    vTaskDelay(pdMS_TO_TICKS(100));

    printf("MPU6050 inicializado.\n");
}

//==========================================================

static void mpu_read_accel(void)
{
    uint8_t data[6];

    if (mpu_read(MPU6050_ACCEL_XOUT_H, data, 6) == ESP_OK)
    {
        accX = (data[0] << 8) | data[1];
        accY = (data[2] << 8) | data[3];
        accZ = (data[4] << 8) | data[5];
    }
}

//==========================================================

static void mpu_read_gyro(void)
{
    uint8_t data[6];

    if (mpu_read(MPU6050_GYRO_XOUT_H, data, 6) == ESP_OK)
    {
        gyroX = (data[0] << 8) | data[1];
        gyroY = (data[2] << 8) | data[3];
        gyroZ = (data[4] << 8) | data[5];
    }
}

//==========================================================
// CONTROLE DO LED
//==========================================================

void ligaLed(void)
{
    gpio_set_level(LED_PIN, 1);
    ledLigado = true;
}

void desligaLed(void)
{
    gpio_set_level(LED_PIN, 0);
    ledLigado = false;
}

//==========================================================
// TASK JOYSTICK
//==========================================================

void TaskJoystick(void *pv)
{
    int value;

    while (1)
    {
        adc_oneshot_read(
            adc_handle,
            JOY_X_CHANNEL,
            &value);

        filtroX = filtroX + FILTRO * (value - filtroX);
joy_x = (int)filtroX;

adc_oneshot_read(
    adc_handle,
    JOY_Y_CHANNEL,
    &value);

filtroY = filtroY + FILTRO * (value - filtroY);
joy_y = (int)filtroY;

        if(abs(joy_x - 2048) < DEADZONE)
            target_x = SERVO_CENTER;
        else
            target_x = SERVO_MIN + SERVO_MAX - mapJoystick(joy_x);

        if(abs(joy_y - 2048) < DEADZONE)
            target_y = SERVO_CENTER;
        else
            target_y = mapJoystick(joy_y);

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

//==========================================================
// TASK SERVO
//==========================================================

void TaskServo(void *pv)
{
    while (1)
    {

if(fabs(target_x - servo_x) < 0.3f)
    servo_x = target_x;

if(fabs(target_y - servo_y) < 0.3f)
    servo_y = target_y;

        if(servo_x < target_x)
        {
            servo_x += SERVO_STEP;

            if(servo_x > target_x)
                servo_x = target_x;
        }

        if(servo_x > target_x)
        {
            servo_x -= SERVO_STEP;

            if(servo_x < target_x)
                servo_x = target_x;
        }

        if(servo_y < target_y)
        {
            servo_y += SERVO_STEP;

            if(servo_y > target_y)
                servo_y = target_y;
        }

        if(servo_y > target_y)
        {
            servo_y -= SERVO_STEP;

            if(servo_y < target_y)
                servo_y = target_y;
        }

        servoWrite(LEDC_CHANNEL_0, servo_x);
        servoWrite(LEDC_CHANNEL_1, servo_y);

        vTaskDelay(pdMS_TO_TICKS(4));
    }
}

//==========================================================
// TASK MPU6050
//==========================================================

void TaskMPU6050(void *pv)
{
    while (1)
    {
        // Lê acelerômetro
        mpu_read_accel();

        // Lê giroscópio
        mpu_read_gyro();

        // Converte para float
        float ax = (float)accX;
        float ay = (float)accY;
        float az = (float)accZ;

        // Cálculo dos ângulos (acelerômetro)
        pitch = atan2f(
                    ay,
                    sqrtf(ax * ax + az * az))
                * 180.0f / M_PI;

        roll = atan2f(
                   -ax,
                   az)
               * 180.0f / M_PI;

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

//==========================================================
// TASK SERIAL
//==========================================================

void TaskSerial(void *pv)
{
    char taskList[1024];

    while (1)
    {
        printf("\n");
        printf("=========================================================\n");
        printf("           MESA LABIRINTO - STATUS DO SISTEMA\n");
        printf("=========================================================\n");

        //=====================================================
        // JOYSTICK
        //=====================================================

        printf("Joystick\n");
        printf("  X : %4d\n", joy_x);
        printf("  Y : %4d\n", joy_y);

        //=====================================================
        // SERVOS
        //=====================================================

        printf("\nServos\n");
        printf("  Servo X : %6.2f graus\n", servo_x);
        printf("  Servo Y : %6.2f graus\n", servo_y);

//=====================================================
// MPU6050
//=====================================================

printf("\nMPU6050\n");

printf("  Acc X : %6d\n", accX);
printf("  Acc Y : %6d\n", accY);
printf("  Acc Z : %6d\n", accZ);

printf("  Gyro X : %6d\n", gyroX);
printf("  Gyro Y : %6d\n", gyroY);
printf("  Gyro Z : %6d\n", gyroZ);

printf("  Pitch : %7.2f graus\n", pitch);
printf("  Roll  : %7.2f graus\n", roll);

        printf("\nAlvos\n");
        printf("  Target X : %6.2f graus\n", target_x);
        printf("  Target Y : %6.2f graus\n", target_y);

        //=====================================================
        // LED
        //=====================================================

        printf("LED Status : %s\n",
       ledLigado ? "LIGADO" : "DESLIGADO");

        //=====================================================
        // TEMPO
        //=====================================================

        printf("\nSistema\n");
        printf("  Tick Count : %lu\n", xTaskGetTickCount());
        printf("  Tempo      : %lu ms\n",
               xTaskGetTickCount() * portTICK_PERIOD_MS);

        //=====================================================
        // STACK DAS TASKS
        //=====================================================

        printf("\nStack Livre\n");

        printf("  TaskJoystick : %u words\n",
               uxTaskGetStackHighWaterMark(joystickHandle));

        printf("  TaskServo    : %u words\n",
               uxTaskGetStackHighWaterMark(servoHandle));

        printf("  TaskSerial   : %u words\n",
               uxTaskGetStackHighWaterMark(serialHandle));
printf("  TaskMPU6050  : %u words\n",
       uxTaskGetStackHighWaterMark(mpuHandle));

#if (configUSE_TRACE_FACILITY == 1) && (configUSE_STATS_FORMATTING_FUNCTIONS == 1)

        //=====================================================
        // LISTA DAS TASKS
        //=====================================================

        vTaskList(taskList);

        printf("\n================== TASKS ==================\n");
        printf("Nome           Estado Prio Stack Numero\n");
        printf("%s\n", taskList);

#endif

        printf("=========================================================\n");

printf(
    "{\"pitch\":%.2f,\"roll\":%.2f,"
    "\"servoX\":%.2f,\"servoY\":%.2f}\n",
    pitch,
    roll,
    servo_x,
    servo_y
);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

//==========================================================
// APP MAIN
//==========================================================

void app_main(void)
{
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

//==========================================================
// INICIALIZAÇÃO I2C / MPU6050
//==========================================================

ESP_ERROR_CHECK(i2c_master_init());

mpu_init();

    adc_oneshot_unit_init_cfg_t init_cfg =
    {
        .unit_id = ADC_UNIT_1
    };

    ESP_ERROR_CHECK(
        adc_oneshot_new_unit(
            &init_cfg,
            &adc_handle));

    adc_oneshot_chan_cfg_t adc_cfg =
    {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12
    };

    ESP_ERROR_CHECK(
        adc_oneshot_config_channel(
            adc_handle,
            JOY_X_CHANNEL,
            &adc_cfg));

    ESP_ERROR_CHECK(
        adc_oneshot_config_channel(
            adc_handle,
            JOY_Y_CHANNEL,
            &adc_cfg));

    //==========================================================
    // CONFIGURAÇÃO DO PWM (LEDC)
    //==========================================================

    ledc_timer_config_t led_timer =
    {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_14_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = SERVO_FREQ,
        .clk_cfg = LEDC_AUTO_CLK
    };

    ESP_ERROR_CHECK(ledc_timer_config(&led_timer));

    ledc_channel_config_t servo_x_channel =
    {
        .gpio_num = SERVO_X_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };

    ESP_ERROR_CHECK(ledc_channel_config(&servo_x_channel));

    ledc_channel_config_t servo_y_channel =
    {
        .gpio_num = SERVO_Y_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_1,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };

    ESP_ERROR_CHECK(ledc_channel_config(&servo_y_channel));

    //==========================================================
    // POSIÇÃO INICIAL DOS SERVOS
    //==========================================================

    servo_x = SERVO_CENTER;
    servo_y = SERVO_CENTER;

    target_x = SERVO_CENTER;
    target_y = SERVO_CENTER;

    servoWrite(LEDC_CHANNEL_0, servo_x);
    servoWrite(LEDC_CHANNEL_1, servo_y);

    vTaskDelay(pdMS_TO_TICKS(500));

    //==========================================================
    // LED DE STATUS
    //==========================================================

   gpio_set_level(LED_PIN, 1);
ledLigado = true;

    printf("\n");
    printf("=====================================\n");
    printf(" Mesa Labirinto - Fase 2\n");
    printf(" ESP32-S3 DevKitC-1\n");
    printf(" Sistema Inicializado\n");
    printf("=====================================\n");

    //==========================================================
    // CRIAÇÃO DAS TASKS
    //==========================================================

    BaseType_t status;

    status = xTaskCreate(
        TaskJoystick,
        "TaskJoystick",
        4096,
        NULL,
        3,
        &joystickHandle);

    if(status != pdPASS)
    {
        printf("Erro ao criar TaskJoystick\n");
    }

    status = xTaskCreate(
        TaskServo,
        "TaskServo",
        8192,
        NULL,
        2,
        &servoHandle);

    if(status != pdPASS)
    {
        printf("Erro ao criar TaskServo\n");
    }


//==========================================================
// NOVA TASK DO MPU6050
//==========================================================

status = xTaskCreate(
    TaskMPU6050,
    "TaskMPU6050",
    4096,
    NULL,
    2,
    &mpuHandle);

if(status != pdPASS)
{
    printf("Erro ao criar TaskMPU6050\n");
}

    status = xTaskCreate(
        TaskSerial,
        "TaskSerial",
        4096,
        NULL,
        1,
        &serialHandle);

    if(status != pdPASS)
    {
        printf("Erro ao criar TaskSerial\n");
    }

    //==========================================================
    // LOOP PRINCIPAL
    //==========================================================

    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
