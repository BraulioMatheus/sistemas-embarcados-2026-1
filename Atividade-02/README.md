Diagrama Esquemático do Sistema de Monitoramento de Temperatura

Este projeto apresenta um sistema embarcado de monitoramento de temperatura baseado no microcontrolador ESP32-S3, responsável pela leitura de um sensor digital de temperatura, exibição das informações em um display gráfico e geração de alarmes sonoros quando necessário.

O sistema foi projetado para operar com alimentação proveniente de uma bateria LiFePO4 (4S), cuja tensão é regulada por um conversor Buck (12V → 5V) antes de alimentar os demais componentes eletrônicos.

Arquitetura do Sistema

O circuito é composto pelos seguintes módulos principais:

ESP32-S3 DevKit – unidade de processamento responsável pelo controle do sistema.

Sensor de temperatura DS18B20 – utilizado para medir a temperatura do ambiente.

Display LCD TFT 128x64 (SPI) – responsável pela visualização dos dados coletados.

Buzzer – utilizado para geração de alarmes sonoros.

Conversor Buck (12V → 5V) – responsável pela regulação da tensão proveniente da bateria.

Bateria LiFePO4 (4S) – fonte de alimentação principal do sistema.

Funcionamento do Sistema

O sensor DS18B20 realiza a medição da temperatura e envia os dados ao ESP32-S3 através do protocolo 1-Wire. O microcontrolador processa essas informações e as apresenta no display TFT, permitindo o monitoramento em tempo real.

Caso a temperatura ultrapasse um limite previamente definido no software, o buzzer é acionado para gerar um alerta sonoro.

Alimentação do Sistema

A alimentação é fornecida por uma bateria LiFePO4 de 4 células, que possui tensão nominal aproximada de 12.8 V. Essa tensão é reduzida por um conversor Buck, que fornece 5 V estáveis para o ESP32 e para os demais componentes do circuito.

Comunicação entre os Componentes
Sensor de Temperatura

O DS18B20 se comunica com o ESP32 através de uma interface 1-Wire, utilizando apenas um pino de dados.

Sensor	ESP32
VDD	3.3V
GND	GND
DQ	GPIO8

Um resistor pull-up de 4.7 kΩ deve ser conectado entre DATA e VCC.

Display TFT

O display gráfico utiliza o protocolo SPI para comunicação com o microcontrolador.

Display	ESP32
VCC	3.3V
GND	GND
SCK	GPIO36
MOSI (DIN)	GPIO35
CS	GPIO37
DC	GPIO39
Buzzer

O buzzer é utilizado como dispositivo de alerta sonoro.

Buzzer	ESP32
+	GPIO18
-	GND
