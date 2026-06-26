# Mesa Labirinto Inteligente com ESP32-S3

Projeto desenvolvido utilizando o **ESP32-S3** e o **ESP-IDF**, com o objetivo de controlar uma mesa labirinto de dois eixos por meio de servomotores. O sistema utiliza um joystick para o controle manual da inclinação da mesa, um sensor MPU6050 para monitoramento da orientação e uma arquitetura baseada em **FreeRTOS**, permitindo a execução concorrente das tarefas do sistema.

---

# Objetivo

Desenvolver um sistema embarcado capaz de:

- Controlar dois servomotores responsáveis pela inclinação da mesa nos eixos X e Y;
- Ler continuamente a posição de um joystick analógico;
- Monitorar a inclinação da mesa utilizando um sensor MPU6050;
- Executar todas as funcionalidades utilizando multitarefa com FreeRTOS;
- Permitir uma arquitetura modular e escalável para futuras expansões.

---

# Funcionalidades

## Controle da Mesa

O joystick permite controlar manualmente a inclinação da mesa.

Cada eixo do joystick controla diretamente um servomotor:

- Eixo X → Servo X
- Eixo Y → Servo Y

Os valores lidos pelo ADC do ESP32-S3 são convertidos para ângulos entre 0° e 180°, proporcionando um movimento suave da plataforma.

---

## Monitoramento da Inclinação

O sensor MPU6050 realiza leituras contínuas da aceleração e da orientação da mesa.

As informações podem ser utilizadas para:

- Monitoramento do movimento;
- Validação da posição da plataforma;
- Futuras estratégias de controle automático.

---

## Arquitetura Multitarefa

O projeto foi desenvolvido utilizando **FreeRTOS**, dividindo o sistema em tarefas independentes.

As principais tarefas incluem:

- Leitura do joystick;
- Controle dos servomotores;
- Leitura do MPU6050;
- Comunicação serial;
- Gerenciamento do sistema.

Essa organização melhora a modularidade, facilita a manutenção do código e permite expansão do projeto.

---

# Componentes Utilizados

- ESP32-S3 DevKitC-1
- 2 Servomotores SG90
- Joystick analógico
- Sensor MPU6050
- Protoboard
- Jumpers
- Fonte de alimentação externa para os servos

---

# Ligações do Circuito

## Servomotores

| Componente | GPIO |
|------------|------|
| Servo X | GPIO 14 |
| Servo Y | GPIO 15 |

Os servomotores são alimentados por uma fonte externa de 5 V, compartilhando o GND com o ESP32-S3.

---

## Joystick

| Sinal | GPIO |
|--------|------|
| VRX | GPIO 4 |
| VRY | GPIO 5 |

Os sinais analógicos são lidos pelo ADC1 do ESP32-S3.

---

## MPU6050

| Sinal | GPIO |
|--------|------|
| SDA | GPIO 8 |
| SCL | GPIO 9 |

A comunicação é realizada utilizando o barramento I²C.

---

# Estrutura do Projeto

```
main/
├── main.c
├── joystick.c
├── joystick.h
├── servo.c
├── servo.h
├── mpu6050.c
├── mpu6050.h
├── i2c.c
├── i2c.h
├── freertos_tasks.c
└── CMakeLists.txt
```

---

# Funcionamento

Após a inicialização:

1. O barramento I²C é configurado.
2. O MPU6050 é inicializado.
3. Os servomotores são posicionados na posição central.
4. O joystick passa a ser monitorado continuamente.
5. As tarefas do FreeRTOS iniciam sua execução.
6. O sistema atualiza continuamente a posição dos servos de acordo com o joystick.

Simultaneamente, o MPU6050 realiza leituras da orientação da plataforma.

---

# Tecnologias Utilizadas

- ESP-IDF
- Linguagem C
- FreeRTOS
- Driver GPIO
- Driver ADC
- Driver LEDC (PWM)
- Driver I²C
- ESP Timer

---

# Conceitos Aplicados

- Sistemas Embarcados
- Programação em C
- FreeRTOS
- Multitarefa
- PWM
- Conversão Analógico-Digital (ADC)
- Comunicação I²C
- Controle de Servomotores
- Leitura de Sensores
- Arquitetura Modular

---

# Desenvolvimento por Fases

## Fase 1

- Configuração do ambiente ESP-IDF;
- Acionamento dos servomotores;
- Leitura do joystick;
- Estrutura inicial do projeto.

## Fase 2

- Integração do sensor MPU6050;
- Implementação das tarefas do FreeRTOS;
- Organização modular do código;
- Sincronização das funcionalidades do sistema.

## Fase 3

- Integração completa entre joystick, servos e MPU6050;
- Comunicação serial para monitoramento;
- Ajustes de desempenho e estabilidade;
- Validação do funcionamento integrado do sistema.

---

# Possíveis Melhorias

- Implementação de controle automático da esfera utilizando PID;
- Exibição das informações em um display OLED;
- Comunicação Wi-Fi para monitoramento remoto;
- Registro de dados em cartão microSD;
- Interface Web para controle da mesa;
- Controle por aplicativo móvel.

---

# Autor

**Braulio Matheus Brito Barbosa**

Projeto desenvolvido para a disciplina de **Sistemas Embarcados**, utilizando **ESP32-S3**, **ESP-IDF** e **FreeRTOS**.
