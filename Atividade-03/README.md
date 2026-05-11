🚀 Controle de LEDs e Buzzer com PWM no ESP32 (ESP-IDF)

📌 Descrição do Projeto

Este projeto implementa o controle de 4 LEDs e 1 buzzer piezoelétrico utilizando PWM (Pulse Width Modulation) no microcontrolador ESP32-S3, com o framework ESP-IDF, simulado no ambiente online Wokwi.

🔗 Acesse o projeto no Wokwi:

👉 https://wokwi.com/projects/460821604897019905

O sistema utiliza o periférico LEDC do ESP32 para gerar sinais PWM, permitindo controlar o brilho dos LEDs e a frequência sonora do buzzer. Esse tipo de abordagem é amplamente utilizado em projetos embarcados para controle de intensidade luminosa e geração de sinais sonoros.

🎯 Objetivo

Desenvolver um sistema embarcado capaz de:

Controlar o brilho de 4 LEDs via PWM
Gerar sinais sonoros em um buzzer com frequência variável
Demonstrar o uso do driver LEDC do ESP32
Implementar diferentes modos de operação utilizando PWM

🧰 Componentes Utilizados

ESP32-S3 DevKitC-1
4 LEDs (cores variadas)
4 resistores de 220 Ω
1 buzzer piezoelétrico
Ambiente de simulação Wokwi

🔌 Ligações do Circuito

LEDs
LED GPIO
LED1 GPIO 4
LED2 GPIO 5
LED3 GPIO 6
LED4 GPIO 7

Cada LED está conectado em série com um resistor de 220 Ω.

Buzzer
Componente GPIO
Buzzer GPIO 8
Terminal positivo → GPIO 8
Terminal negativo → GND

🧠 Funcionamento do Sistema

O sistema opera em 3 fases principais:

🔵 Fase 1: Fading Sincronizado

Todos os LEDs aumentam o brilho de 0% → 100%
Em seguida diminuem de 100% → 0%
Efeito de iluminação suave

🟢 Fase 2: Fading Sequencial

LEDs acendem individualmente em sequência:
LED1 → LED2 → LED3 → LED4
Cada LED executa um ciclo completo de brilho

🔴 Fase 3: Teste Sonoro

O buzzer emite sons variando:
De 500 Hz até 2000 Hz
Depois retorna para 500 Hz
Cria efeito de subida e descida de tom

⚙️ Configurações Técnicas

Parâmetro Valor
Frequência LEDs 1 kHz
Frequência inicial buzzer 1000 Hz
Faixa buzzer 500 Hz – 2000 Hz
Resolução PWM 10 bits
Duty cycle LEDs 0% – 100%

🧩 Estrutura do Código

O código foi organizado em funções para facilitar manutenção:

pwm_init() → Configuração dos canais PWM
set_led_percent() → Controle individual de brilho
set_all_leds() → Controle simultâneo
fase1_fading_sincronizado()
fase2_fading_sequencial()
fase3_teste_sonoro()

▶️ Como Executar

Acesse o link do projeto no Wokwi
Clique em "Start Simulation"
Observe:
Variação de brilho dos LEDs
Sequência de acionamento
Emissão de som pelo buzzer

📌 Observações

LEDs iniciam apagados (duty = 0%)
Buzzer inicia configurado em 1000 Hz
O uso de PWM permite controle fino de intensidade e frequência
O driver LEDC permite múltiplos canais independentes de PWM no ESP32

👨‍💻 Autor

Braulio Matheus Brito Barbosa
