#ifndef TESTDRIVER_H
#define TESTDRIVER_H

#include <linux/ioctl.h> // Needed for the _IOW etc stuff
#include <linux/mutex.h>
#include <linux/delay.h>

#define DRIVER_NAME             "DriverSOi2c"

//Defines pegos do projeto dos alunos: Jesuino Vieira Filho e Lucas de Camargo Souza

#define LCD_ADDRESS             0x27

#define LCD_CHR                 0x01 
#define LCD_CMD                 0x00 

#define LCD_LINE0               0x80 
#define LCD_LINE1               0xC0
 
#define LCD_ENABLE              0x04
#define LCD_CLEAR               0x01
#define LCD_HOME                0x02
#define LCD_BACKLIGHT           0x08


// Valores para ioctl (switch-case)
#define INICIA_TELA             10  
#define BACKLIGHT               11
#define CURSOR                  12
#define BLINK                   13
#define LIMPA_TELA              14

#define CURSOR_ADDR             1
#define BLINK_ADDR              0


// enable - Usado para o envio dos dados

u8 retLCD_E_HI(u8 ret)
{
    return (ret | (LCD_ENABLE));
}

u8 retLCD_E_LOW(u8 ret)
{
    return (ret & ~(LCD_ENABLE));
}

u8 retLCD_RS_DATA(u8 ret)
{
    return (ret | LCD_CHR);
}

u8 retLCD_RS_INSTR(u8 ret)
{
    return (ret | LCD_CMD);
}

u8 retLCD_DATA_MSB(u8 ret)
{
    return (ret & 0xF0);
}

u8 retLCD_DATA_LSB(u8 ret)
{
    return ((ret << 4) & 0xF0);
}

#define LCD_E_HI(ret)       retLCD_E_HI(ret)
#define LCD_E_LOW(ret)      retLCD_E_LOW(ret)

// Macro para definição valor pino RS -> 0 - instrucao, 1 dados

#define LCD_RS_DATA(data)   retLCD_RS_DATA(data)
#define LCD_RS_INSTR(data)  retLCD_RS_INSTR(data)

// Separa o byte em duas partes, uma MSB e outra LSB, que serão enviadas separadamente ao display

#define LCD_DATA_MSB(x)   retLCD_DATA_MSB(x)
#define LCD_DATA_LSB(x)   retLCD_DATA_LSB(x)

//DEFINIÇÃO DA ESTRUTURA DO NOSSO DRIVER

typedef struct i2cDriverSO
{
    struct i2c_client *meuCliente;//representa o dispositivo conectado com o qual nosso SO deve se comunicar.
    struct mutex       meuMutex;
    struct cdev        meuCdev;//estrutura interna do kernel para representar char devices

    u8                 regs_cntrl;
    u8                 linha;
    u8                 coluna;
}i2cDriverSO_t;

//Algumas funções uteis para a comunicação
void ligaTela(i2cDriverSO_t *);
void escreveTela(i2cDriverSO_t *);
void limpaTela(i2cDriverSO_t *);
void IniciaTela(i2cDriverSO_t *);

#endif
