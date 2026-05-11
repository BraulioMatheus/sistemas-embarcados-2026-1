#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/uart.h"
#include "driver/gpio.h"

#define UART_PORT UART_NUM_2

#define TXD_PIN 17
#define RXD_PIN 16

#define LED_PIN GPIO_NUM_4

#define BUF_SIZE 1024

void app_main(void)
{
    // =========================
    // Configuração do LED
    // =========================
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    // =========================
    // Configuração UART2
    // =========================
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT
    };

    uart_driver_install(UART_PORT, BUF_SIZE * 2, 0, 0, NULL, 0);

    uart_param_config(UART_PORT, &uart_config);

    uart_set_pin(UART_PORT,
                 TXD_PIN,
                 RXD_PIN,
                 UART_PIN_NO_CHANGE,
                 UART_PIN_NO_CHANGE);

    uint8_t data[BUF_SIZE];

    int estado = 0;

    while (1)
    {
        char mensagem[20];

        // Alterna mensagem
        if (estado == 0)
        {
            strcpy(mensagem, "LIGAR");
            estado = 1;
        }
        else
        {
            strcpy(mensagem, "DESLIGAR");
            estado = 0;
        }

        // =========================
        // Envia mensagem
        // =========================
        uart_write_bytes(UART_PORT,
                         mensagem,
                         strlen(mensagem));

        printf("Enviado: %s\n", mensagem);

        vTaskDelay(pdMS_TO_TICKS(100));

        // =========================
        // Recebe mensagem
        // =========================
        int len = uart_read_bytes(UART_PORT,
                                  data,
                                  BUF_SIZE - 1,
                                  pdMS_TO_TICKS(1000));

        if (len > 0)
        {
            data[len] = '\0';

            printf("Recebido: %s\n", (char*)data);

            if (strcmp((char*)data, "LIGAR") == 0)
            {
                gpio_set_level(LED_PIN, 1);
                printf("LED LIGADO\n");
            }
            else if (strcmp((char*)data, "DESLIGAR") == 0)
            {
                gpio_set_level(LED_PIN, 0);
                printf("LED DESLIGADO\n");
            }
        }

        printf("---------------------\n");

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
