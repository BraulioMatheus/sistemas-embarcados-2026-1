📊 Diagrama de Blocos – Sistema de Monitoramento de Temperatura

O sistema de monitoramento de temperatura é baseado no ESP32-S3 DevKitC-1, responsável pelo processamento dos dados, controle do display e acionamento do alarme sonoro.

🔋 Alimentação

A alimentação do sistema é feita por uma bateria LiFePO4 4S (12V nominal).
A tensão da bateria é reduzida por meio de um conversor Buck (12V → 5V), que fornece:

5V para alimentação do ESP32-S3

GND comum para todo o sistema

O ESP32 distribui internamente a tensão necessária (3,3V) para os periféricos compatíveis.

🌡️ Sensor de Temperatura

O sensor utilizado é o DS18B20, responsável pela medição da temperatura na faixa de 0°C a 100°C.

Comunicação com o ESP32 via interface digital

Conectado aos GPIO 8 e 9

Alimentação em 3,3V

O microcontrolador realiza a leitura periódica da temperatura para processamento e exibição.

🖥️ Display

O sistema utiliza um Display LCD TFT 128x64, responsável pela exibição da temperatura e informações do sistema.

Comunicação via SPI

Conectado aos GPIO 35–37 e GPIO 39

Alimentação em 3,3V

🔔 Alarme Sonoro

Um buzzer é utilizado para sinalização de temperatura fora da faixa configurada.

Controlado pelo GPIO 18

Referenciado ao GND

Acionado pelo ESP32 quando a temperatura ultrapassa o limite definido

⚙️ Funcionamento Geral

A bateria alimenta o sistema através do conversor Buck.

O ESP32-S3 realiza a leitura da temperatura via DS18B20.

O valor medido é processado internamente.

A temperatura é exibida no display TFT.

Caso o valor ultrapasse o limite estabelecido, o buzzer é acionado como alerta sonoro.

