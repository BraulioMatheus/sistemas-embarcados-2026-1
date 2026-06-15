# ⚡ Controle de Eventos Críticos com Interrupções e Temporizadores (ESP32 - ESP-IDF)

Projeto desenvolvido utilizando o ESP32-S3 no simulador Wokwi, com o objetivo de implementar um sistema de acionamento inteligente de um LED utilizando interrupções de hardware (ISR), debounce por software e temporizadores do FreeRTOS.

## 🔗 Acesse o Projeto no Wokwi

👉 https://wokwi.com/projects/463682971425667073

---

## 🎯 Objetivo

Implementar um sistema embarcado capaz de:

* Detectar eventos de botão utilizando interrupções de hardware  
* Aplicar debounce por software  
* Acionar um LED por tempo determinado  
* Reiniciar automaticamente o temporizador a cada novo acionamento  
* Detectar clique longo para desligamento imediato  
* Exibir informações de diagnóstico no monitor serial  

---

## 🧩 Componentes Utilizados

* ESP32-S3 DevKitC-1  
* 1 LED  
* 1 resistor de 220 Ω  
* 1 botão (pushbutton)  
* Protoboard e jumpers  

---

## 🔌 Ligações do Circuito

### LED

* GPIO 4 → resistor 220 Ω → ânodo do LED  
* Cátodo → GND  

### Botão

* GPIO 15 → botão  
* Outro terminal do botão → GND  

Utilização do resistor Pull-up interno do ESP32.  

---

## 🧠 Funcionamento do Sistema

O sistema opera de forma orientada a eventos, utilizando interrupções para detectar mudanças de estado do botão sem a necessidade de polling contínuo.  

### ✔️ Lógica implementada

#### Primeiro Clique

Quando o botão é pressionado e liberado rapidamente:

* O LED é ligado  
* Um temporizador de 10 segundos é iniciado

#### Cliques Subsequentes

Se o LED já estiver ligado:

* O temporizador é reiniciado  
* O LED permanece ligado  

Esse comportamento simula um sensor de presença que renova o tempo de acionamento a cada nova detecção.  

#### Clique Longo

Se o botão permanecer pressionado por mais de 2 segundos:

* O LED é desligado imediatamente  
* O temporizador é cancelado  

---

## ⚙️ Tratamento de Bounce

Botões mecânicos produzem oscilações elétricas (bounce) durante o acionamento.  

Para evitar múltiplas detecções indevidas foi implementado:  

* Registro do instante da última interrupção válida  
* Ignorar novos eventos ocorridos em menos de 200 ms  

Dessa forma, apenas acionamentos legítimos são processados.  

---

## ⏱️ Temporizador Automático

Sempre que ocorre um clique curto:  

* O LED é ligado  
* O contador é reiniciado para 10 segundos  

Caso nenhum novo evento ocorra dentro desse período:  

* O temporizador expira  
* O LED é desligado automaticamente  

---

## 🧪 Exemplo de Saída Serial

```text
I EVENT_CTRL: Sistema iniciado

I EVENT_CTRL: [PRESS] Botão pressionado  
I EVENT_CTRL: [RELEASE] duração = 180 ms  
I EVENT_CTRL: [SHORT CLICK] LED ligado | Timer reiniciado para 10s  

I EVENT_CTRL: [STATUS] LED ON | Tempo restante: 9.0 s  
I EVENT_CTRL: [STATUS] LED ON | Tempo restante: 8.0 s  

I EVENT_CTRL: [TIMER] Tempo expirado -> LED DESLIGADO  

I EVENT_CTRL: [PRESS] Botão pressionado  
I EVENT_CTRL: [RELEASE] duração = 2500 ms  
W EVENT_CTRL: [LONG PRESS] LED desligado imediatamente  
```

---

## 🖥️ Tecnologias Utilizadas

* ESP-IDF (framework oficial da Espressif)  
* FreeRTOS  
* GPIO Driver  
* GPIO Interrupts (ISR)  
* FreeRTOS Queue  
* FreeRTOS Software Timers  
* esp_timer  
* Simulação no Wokwi  

---

## 📚 Conceitos Aplicados

* Sistemas orientados a eventos  
* Interrupções de hardware  
* Debounce por software  
* Temporizadores em sistemas embarcados  
* Filas de comunicação (Queues)  
* Multitarefa com FreeRTOS  
* Tratamento de eventos assíncronos  

---

## 🚀 Possíveis Melhorias

* Adicionar buzzer para feedback sonoro  
* Implementar múltiplos LEDs independentes  
* Exibir informações em display OLED  
* Registrar eventos em memória não volátil  
* Implementar diferentes modos de operação  
* Utilizar máquina de estados finitos (FSM)  
* Adicionar comunicação Wi-Fi para monitoramento remoto  

