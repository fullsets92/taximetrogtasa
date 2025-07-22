# taximetrogtasa
Um taximetro em texto para servidores SAMP (Clientside)


No .ini é posssível configurar como quiser, inclusive mudando teclas de ativação.

teclaBandeira cicla entre os 3 valores.

Apertar a teclaIniciar iniciará a corrida. Se apertar novamente, a corrida será pausada, e se apertar com ela pausada, irá despausar. (Pra simplificar ao despausar o tempo reseta, então se você pausa faltando 1s pra aumentar o preço, ao voltar ele vai levar "Tempo" segundos pra aumentar.)

Ao chegar no destino, aperte o teclaIniciar para pausar e ver o preço, pois o teclaReset resetará a corrida de volta para o preço base.


Diferença do teclaLigar pra teclaHUD:
Ligar -> Ativa/Desativa o sistema todo
HUD -> Apenas para de mostrar o texto
