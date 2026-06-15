💡 Sistema Multitarefa com FreeRTOS, MPU6050 e Controle PWM (ESP32-S3)

Projeto desenvolvido utilizando o ESP32-S3 DevKitC-1 no simulador Wokwi, com o objetivo de demonstrar a utilização de uma arquitetura multitarefa baseada em FreeRTOS, integrando leitura analógica, controle PWM, comunicação I2C e mecanismos de sincronização entre tarefas.

🔗 Acesse o Projeto no Wokwi

👉 https://wokwi.com/projects/463909116911517697

🎯 Objetivo

Implementar um sistema embarcado multitarefa capaz de:

Ler continuamente um potenciômetro utilizando o ADC do ESP32-S3;  
Controlar o brilho de um LED por PWM de forma proporcional à posição do potenciômetro;  
Alternar entre os modos LIVE e HOLD através de um botão;  
Realizar a leitura de um sensor inercial MPU6050 via barramento I2C;  
Converter os valores brutos do acelerômetro para unidades de aceleração gravitacional (g);  
Compartilhar dados entre tarefas utilizando filas, semáforos e mutexes;  
Exibir informações do sistema em tempo real através do terminal serial.  

🧩 Componentes Utilizados

ESP32-S3 DevKitC-1  
1 LED vermelho  
1 resistor de 220 Ω  
1 potenciômetro de 10 kΩ  
1 botão (pushbutton)  
1 resistor de 10 kΩ (pull-down)  
1 sensor MPU6050  
Protoboard e jumpers 

🔌 Ligações do Circuito

LED PWM  
GPIO 4 → resistor 220 Ω → ânodo do LED  
Cátodo → GND  
Potenciômetro  
Terminal VCC → 3.3 V  
Terminal GND → GND  
Terminal central (SIG) → GPIO 1 (ADC)  
Botão  
Um terminal → 3.3 V  
Outro terminal → GPIO 15  
Resistor de 10 kΩ entre GPIO 15 e GND  
MPU6050  
VCC → 3.3 V  
GND → GND  
SDA → GPIO 8  
SCL → GPIO 9  

🧠 Funcionamento do Sistema

O firmware é baseado em cinco tarefas independentes executadas pelo FreeRTOS.

Tarefa Potenciômetro

Realiza a leitura do ADC e converte o valor para a faixa utilizada pelo PWM. Os valores são enviados para uma fila compartilhada.

Tarefa LED

Recebe os dados da fila e atualiza o duty cycle do PWM, ajustando o brilho do LED proporcionalmente ao potenciômetro.

Tarefa Botão

Monitora o estado do botão e utiliza um semáforo binário para alternar entre os modos de operação.

Tarefa IMU

Realiza a leitura periódica dos registradores do MPU6050 através da interface I2C e converte os valores para aceleração em g.

Tarefa Console

Exibe as informações do sistema através da porta serial, incluindo estado do sistema, leitura do potenciômetro, brilho do LED e aceleração medida pela IMU.

🔄 Modos de Operação  
Modo LIVE

O LED acompanha continuamente a posição do potenciômetro.  

STATUS: [LIVE]  
Modo HOLD  

O brilho do LED permanece congelado no último valor recebido antes do acionamento do botão.  

STATUS: [HOLD]

Mesmo em HOLD, as demais tarefas continuam executando normalmente.

🔒 Mecanismos de Sincronização Utilizados  
Queue (Fila)

Potenciômetro → LED

Responsável por transferir os valores do ADC para a tarefa responsável pelo PWM.

Semáforo Binário

Botão → Controle de Estado

Utilizado para alternar entre os modos LIVE e HOLD.

Mutex

IMU ↔ Console

Protege o acesso simultâneo às variáveis compartilhadas contendo os dados dos eixos X, Y e Z do acelerômetro.

🖥️ Tecnologias Utilizadas

ESP-IDF  
FreeRTOS  
ESP32-S3  
ADC One-Shot Driver  
LEDC PWM Driver  
Driver I2C  
MPU6050  
Wokwi Simulator  

🚀 Possíveis Melhorias

Utilização de interrupções para o botão;  
Inclusão da leitura do giroscópio do MPU6050;  
Implementação de filtros digitais para suavização dos dados da IMU;  
Exibição das informações em display OLED;  
Registro de dados em cartão SD;  
Comunicação via Wi-Fi utilizando MQTT;  
Dashboard remoto para monitoramento em tempo real;  
Ajuste dinâmico das prioridades das tarefas do FreeRTOS.  

📚 Conceitos Aplicados

Sistemas Operacionais de Tempo Real (RTOS)  
Multitarefa cooperativa e preemptiva  
Comunicação entre tarefas  
Sincronização com semáforos e mutexes  
Controle PWM  
Conversão Analógico-Digital (ADC)  
Comunicação I2C  
Sensores Inerciais (IMU)  
Programação embarcada com ESP32-S3  
Desenvolvimento utilizando ESP-IDF e FreeRTOS  
