💡 Sistema de Iluminação com Temporizador (ESP32 - ESP-IDF)

Projeto desenvolvido utilizando o ESP32-S3 no simulador Wokwi, com o objetivo de implementar o controle de um LED via botão, utilizando polling, debounce por software e temporizador automático.

🔗 Acesse o Projeto no Wokwi

👉 https://wokwi.com/projects/460551782907086849

🎯 Objetivo

Implementar um sistema embarcado capaz de:

Ler o estado de um botão (entrada digital)
Alternar (toggle) o estado de um LED a cada acionamento
Aplicar debounce por software
Desligar automaticamente o LED após 10 segundos
🧩 Componentes Utilizados
ESP32-S3 DevKitC-1
1 LED
1 resistor de 220 Ω
1 botão (pushbutton)
Protoboard e jumpers
🔌 Ligações do Circuito
LED
GPIO 2 → resistor 220 Ω → ânodo do LED
Cátodo → GND
Botão
GPIO 4 → botão
Outro terminal do botão → GND
Utilização de pull-up interno do ESP32
🧠 Funcionamento do Sistema

O sistema executa um loop contínuo (polling) para monitorar o botão.

✔️ Lógica implementada
Quando o botão é pressionado:
O LED alterna seu estado (liga/desliga)
Se o LED estiver ligado:
Um temporizador é iniciado
Após 10 segundos:
O LED é desligado automaticamente

⚙️ Tratamento de Bounce

Botões mecânicos geram ruídos elétricos (bounce), causando múltiplas leituras.

Para resolver isso, foi implementado:

Detecção de mudança de estado
Aguardar 50 ms
Confirmar estabilidade do sinal antes de aceitar o clique

🧪 Exemplo de Saída Serial

Sistema iniciado.
LED inicia apagado.
Pressione o botao para alternar o LED.
Desligamento automatico em 10 segundos.

LED ligado.
LED desligado automaticamente apos 10 segundos.

🖥️ Tecnologias Utilizadas

ESP-IDF (framework oficial da Espressif)
FreeRTOS
GPIO Driver
Timer via software (esp_timer)
Simulação no Wokwi
🚀 Possíveis Melhorias
Substituir polling por interrupções (GPIO interrupt)
Adicionar buzzer para feedback sonoro
Implementar múltiplos LEDs
Exibir tempo restante na serial
Interface com display (OLED ou LCD)

👨‍💻 Autor

Braulio Matheus Brito Barbosa

📌 Observações
O sistema utiliza pull-up interno, portanto:
botão solto = nível lógico 1
botão pressionado = nível lógico 0
O debounce é totalmente feito por software
O projeto foi validado no simulador Wokwi
