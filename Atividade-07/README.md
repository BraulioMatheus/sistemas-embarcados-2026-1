# 💡 Comunicação Serial UART com Teste de Loopback (ESP32-S3 - ESP-IDF)

Projeto desenvolvido utilizando o ESP32-S3 no simulador Wokwi, com o objetivo de implementar a comunicação serial assíncrona utilizando a interface UART2, explorando o conceito de loopback para validar o envio e recebimento de dados.

## 🔗 Acesse o Projeto no Wokwi

👉 https://wokwi.com/projects/463683452082506753

## 🎯 Objetivo

Implementar um sistema embarcado capaz de:

* Configurar e utilizar a UART2 do ESP32-S3;  
* Enviar mensagens periodicamente pela interface serial;  
* Receber os próprios dados enviados através de uma conexão de loopback;  
* Controlar um LED com base nas mensagens recebidas;  
* Exibir informações de transmissão e recepção no monitor serial.

## 🧩 Componentes Utilizados

* ESP32-S3 DevKitC-1  
* 1 LED  
* 1 resistor de 220 Ω  
* Jumper para conexão de loopback (TX ↔ RX)  
* Protoboard e cabos de conexão  

## 🔌 Ligações do Circuito

### LED

* GPIO 4 → ânodo do LED  
* Cátodo do LED → resistor de 220 Ω  
* Resistor → GND

### UART2 (Loopback)

* GPIO 17 (TX2) → GPIO 16 (RX2)  

A conexão direta entre os pinos TX2 e RX2 permite que toda mensagem enviada pela UART seja imediatamente recebida pelo próprio microcontrolador.

## 🧠 Funcionamento do Sistema

O sistema executa continuamente o envio e a recepção de mensagens através da UART2.

### ✔️ Lógica implementada

A cada 2 segundos:

* O ESP32 envia uma mensagem pela UART2;  
* As mensagens alternam entre "LIGAR" e "DESLIGAR";  
* A conexão de loopback faz com que a mensagem retorne ao receptor;  
* O ESP32 processa a mensagem recebida;  
* O LED é acionado de acordo com o comando recebido.  

### Controle do LED

Ao receber:

* "LIGAR" → LED acende;  
* "DESLIGAR" → LED apaga.

## 📡 Configuração da UART

A UART2 foi configurada com os seguintes parâmetros:

* Baud Rate: 115200 bps  
* 8 bits de dados  
* Sem paridade  
* 1 bit de parada  
* Sem controle de fluxo

## 🧪 Exemplo de Saída Serial

```text
Enviado: LIGAR  
Recebido: LIGAR  
LED LIGADO
---------------------

Enviado: DESLIGAR  
Recebido: DESLIGAR  
LED DESLIGADO
---------------------
```

## 🖥️ Tecnologias Utilizadas

* ESP-IDF (Espressif IoT Development Framework)  
* FreeRTOS  
* Driver UART  
* Driver GPIO  
* Simulação no Wokwi

## 🚀 Possíveis Melhorias

* Utilizar interrupções UART para recepção de dados;  
* Implementar comunicação entre dois ESP32 físicos;  
* Adicionar protocolo de validação de mensagens;  
* Implementar comandos personalizados enviados pelo usuário;  
* Criar interface gráfica para monitoramento da comunicação serial;  
* Expandir para comunicação com sensores e dispositivos externos via UART.

## 📚 Conceitos Abordados

* Comunicação Serial UART  
* Transmissão e Recepção Assíncrona  
* Loopback de Dados  
* Manipulação de GPIO  
* Programação Embarcada com ESP-IDF  
* Integração entre Hardware e Software  
* Monitoramento Serial para Depuração

