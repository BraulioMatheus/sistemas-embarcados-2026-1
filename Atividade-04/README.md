# 🔢 Contador Binário de 4 Bits com Botões (ESP32-S3)

Projeto desenvolvido utilizando o **ESP32-S3** no simulador **Wokwi**, com o objetivo de implementar um contador binário de 4 bits controlado por dois botões. O valor atual do contador é exibido por meio de quatro LEDs, enquanto os botões permitem incrementar o contador e alternar a unidade de incremento entre +1 e +2.

## 🔗 Acesse o Projeto no Wokwi

👉 https://wokwi.com/projects/460791608632695809

---

## 🎯 Objetivo

Implementar um sistema embarcado capaz de:

* Exibir um contador binário de 4 bits utilizando 4 LEDs
* Incrementar o contador através de um botão
* Alternar a unidade de incremento entre +1 e +2 através de um segundo botão
* Implementar debounce por software
* Garantir comportamento circular (overflow) dentro do intervalo de 4 bits

---

## 🧩 Componentes Utilizados

* ESP32-S3 DevKitC-1
* 4 LEDs
* 4 resistores de 220 Ω
* 2 botões (pushbuttons)
* Protoboard e jumpers virtuais do Wokwi

---

## 🔌 Ligações do Circuito

### LEDs

| LED          | GPIO   |
| ------------ | ------ |
| LED0 (Bit 0) | GPIO 4 |
| LED1 (Bit 1) | GPIO 5 |
| LED2 (Bit 2) | GPIO 6 |
| LED3 (Bit 3) | GPIO 7 |

Cada LED é conectado ao respectivo GPIO através de um resistor de 220 Ω, com o cátodo ligado ao GND.

### Botões

| Botão   | GPIO   | Função                      |
| ------- | ------ | --------------------------- |
| Botão A | GPIO 8 | Incrementar contador        |
| Botão B | GPIO 9 | Alternar incremento (+1/+2) |

Os botões utilizam o pull-up interno do ESP32-S3, ficando em nível lógico alto quando soltos e nível lógico baixo quando pressionados.

---

## 🧠 Funcionamento do Sistema

O sistema monitora continuamente os dois botões utilizando polling.

### ✔️ Botão A

A cada acionamento:

* O contador é incrementado de acordo com o passo atual.
* O valor atualizado é exibido nos quatro LEDs em formato binário.

### ✔️ Botão B

A cada acionamento:

* Alterna a unidade de incremento entre:

  * +1
  * +2

---

## 🔁 Lógica de Overflow

O contador possui 4 bits, podendo assumir valores entre:

```text
0x0 (0 decimal)
até
0xF (15 decimal)
```

Quando ocorre um incremento que ultrapassa o limite de 4 bits, o contador retorna automaticamente ao início.

### Exemplos

```text
0xF + 1 = 0x0
0xE + 2 = 0x0
0xF + 2 = 0x1
```

A implementação utiliza máscara de 4 bits:

```c
contador = (contador + passo) & 0x0F;
```

---

## ⚙️ Tratamento de Bounce

Botões mecânicos produzem oscilações elétricas durante o acionamento, podendo gerar múltiplas leituras indesejadas.

Para resolver esse problema foi implementado debounce por software utilizando:

* Leitura periódica dos botões
* Detecção de transições de estado
* Temporização baseada em `esp_timer_get_time()`
* Confirmação da estabilidade do sinal por 50 ms

Essa abordagem evita acionamentos múltiplos sem utilizar atrasos bloqueantes.

---

## 🧪 Exemplo de Saída Serial

```text
Sistema iniciado
Contador inicial = 0
Passo inicial = 1

Botao A pressionado
Contador = 1

Botao A pressionado
Contador = 2

Botao B pressionado
Novo passo = 2

Botao A pressionado
Contador = 4
```

---

## 🖥️ Tecnologias Utilizadas

* ESP-IDF (Espressif IoT Development Framework)
* FreeRTOS
* Driver GPIO
* ESP Timer (`esp_timer`)
* Simulação no Wokwi

---

## 🚀 Possíveis Melhorias

* Substituir polling por interrupções GPIO
* Adicionar indicação visual do modo de incremento
* Exibir o valor do contador em display OLED
* Adicionar comunicação UART para monitoramento externo
* Implementar contagem regressiva
* Adicionar função de reset do contador

---

## 📚 Conceitos Aplicados

* Sistemas embarcados
* Programação em C
* Manipulação de GPIO
* Contadores binários
* Operações bitwise
* Debounce por software
* FreeRTOS
* Controle de eventos digitais

---

## 👨‍💻 Autor

**Braulio Matheus Brito Barbosa**

Projeto desenvolvido para a disciplina de Sistemas Embarcados utilizando ESP32-S3 e ESP-IDF.

