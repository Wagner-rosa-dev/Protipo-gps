Esse projeto é básicamente uma criação de um ambiente 3D. Projeto originalmente feito para rodar em dispositivos embarcados
Placa usada no projeto: Toradex com processador Verdin imx8xmp
Consistem em um terreno infinito gerado proceduralmente, uma grade que delimite cada chunk gerado para melhor compreensão do espaço, e um triangulo representando seu personagem

O Terreno: se utiliza várias técnicas para maxima otimização de memória, gerado a partir de uma técnica de "reciclagem" de chunks aonde invez de destruir e construir chunks novos eu simplesmente mudo eles de lugar usando VBOs, VAOs, EBOs e um
construtor de movimento, terreno baseado em chunks, modelado com shader

Grade: uma grade simples aonde a unica função é contornar os chunks gerados para ter mais noção de espaço e para melhor configuração de espaço dentro do código, modelado com shaders

O Triangulo: um triangulo vermelho na tela para representar seu "personagem", feito com shader de configuração simples, desenhado na tela de forma direta então nao se utiliza de tecnologias anteriores
tem uma funcionalidade interessante aonde ele nao so esta na tela como esta grudado no chao então caso o terreno tenha elevações ou depressões o triangulo acompanha o sentido do terreno

A Camera: uma camera inteligente totalmente configurada para ser configuravel, ela acompanha o sentido do seu "personagem" e tambem tem a possibilidade de ser controlavel pela quantidade de configurações passadas no codigo 
