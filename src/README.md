# Biblioteca para uso no ATmega328p

Neste diretório estão os códigos para lidar com os temporizadores 0 e 2, a interface serial e os pinos GPIO do ATmega328p. Como uma estrutura de dados para se implementar as funcionalidades da interface serial, também existe uma implementação de um buffer circular para armazenamento de bytes.

Esta biblioteca vem sendo desenvolvida ao longo das aulas e será usada para implementação dos projetos da disciplina.

Para a compilação do código, use um comando como

```bash
avr-gcc -mmcu=atmega328p -O1 file1.c file2.c file3.c
```

Isto gerará um arquivo-objeto para cada arquivo C. O exemplo acima, portanto, gerará três arquivos, file.o, file2.o e file3.o.

Estes arquivos podem ser linkados em um único arquivo binário com todas as referências resolvidas através do comando

```bash
avr-gcc -mmcu=atmega328p file1.o file2.o file3.o -o main.bin
```

Estas duas etapas, compilação e linkagem, podem ser realizadas de uma única vez com a linha de comando

```bash
avr-gcc -mmcu=atmega328p -O1 file1.c file2.c file3.c -o main.bin
```

O arquivo main.bin gerado por qualquer uma das duas maneiras acima deve ser convertido para um formato apropriado para uso posterior do programa avrdude através da linha de comando

```bash
avr-objcopy -j .text -j .data -O ihex main.bin main.hex
```

Por último, o programa gerado pode ser baixado na placa Arduino Nano usando o comando

```bash
avrdude -b 57600 -c arduino -P /dev/ttyUSB0 -p atmega328p -v -v -v -v -D -U flash:w:main.hex:i
```
