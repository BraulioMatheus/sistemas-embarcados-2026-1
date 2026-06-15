📘 Monitoramento Analógico e Controle de PWM

🔌 1. Esquemático do Circuito

Componentes e ligações:

Potenciômetro (10 kΩ recomendado)

Terminal 1 → 3.3V  
Terminal 2 (cursor) → GPIO34 (ADC)  
Terminal 3 → GND  

LED + Resistor (220 Ω)  
GPIO18 → Resistor → LED → GND

Botão + Resistor (10 kΩ pull-down)  
Um lado → 3.3V  
Outro lado → GPIO4  
Resistor 10kΩ → entre GPIO4 e GND  

⚙️ 2. Parâmetros de Configuração

📥 ADC (Conversão Analógica → Digital)  
Unidade: ADC1  
Canal: ADC_CHANNEL_6 (GPIO34)  
Resolução: 12 bits (0–4095)  
Atenuação: 11 dB (permite até ~3.3V)  
💡 PWM (LEDC)  
Timer: LEDC_TIMER_0  
Frequência: 5 kHz  
Resolução: 13 bits (0–8191)  
Canal: LEDC_CHANNEL_0    
GPIO: 18  
🔘 Botão  
GPIO: 4  
Modo: entrada  
Pull-down externo  

📐 3. Equação de Conversão ADC → Tensão

Aqui entra a parte mais importante conceitualmente:

V=
4095
ADC
	​

×3300

Onde:

ADC: valor lido (0–4095)  
V: tensão em mV  

💻 4. Desenvolvimento do Firmware (ESP-IDF)

📌 Funcionalidades implementadas:

Leitura do ADC  
Conversão para tensão  
Controle de brilho via PWM  
Botão com modo HOLD  
Monitoramento via terminal  

🧠 Lógica do sistema

Modo LIVE: LED acompanha o potenciômetro  
Modo HOLD: valor congelado  
Botão alterna entre os modos  
