# calcula-classifica-dist

Código para medição e classificação de distância com sensor a laser óptico de alta precisão VL53L0X

--> O código realiza a medição em tempo real da distância entre o sensor VL53L0X e outros objetos, classificando a distância em "Perto", "Ideal", "Longe" ou "Sem captação de sinal" de acordo com os intervalos de valores definidos. A classificação é apresentada pelo sistema de forma visual e auditiva, através de uma barra LED de 10 segmentos e 4 cores e um buzzer contínuo 5V.

--> A função CalibraDistancia permite a calibração de um valor de distância padrão ideal para o sistema, deixando este valor salvo na memória EEPROM do microcontrolador para ser reutilizado em futuras medições. Sempre que o código é iniciado, esta função permite ao usuário do sistema realizar uma nova calibração, ou pular esta etapa e usar um valor pré-calibrado (através de 2 botões de estado). Caso o usuário pule a etapa e não haja um valor válido pré-calibrado, a função define um valor padrão de 15cm como ideal.

--> Após a etapa de calibração, o sensor VL53L0X reinicia a medição da distância e compara seus valores atuais "dist_atual" com a distância ideal calibrada na função CalibraDistancia, "dist_ideal". 

--> A função AtivaLEDBuzzer ativa o acionamento do buzzer e dos segmentos específicos da BarraLED de acordo com a classificação da "dist_atual" feita pelo microcontrolador.


--> Dessa forma, o sistema pode ser útil para implementação em:

     - Sistemas que exigem medições de distância constante e em tempo real
     - Sistemas que exigem que seus processos sejam realizados em um valor ou intervalo ideal específico de distância durante múltiplos procedimentos
     - Sistemas que buscam o aumento de padronização e repetibilidade em suas aquisições e/ou resultados
     - Quaisquer sistemas cuja qualidade dos resultados dependam da distância de aquisição ou medição
