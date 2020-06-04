#ifndef LCD_LIB_H
#define LCD_LIB_H

#include <unistd.h>
#include <sys/ioctl.h> /* ioctl */
#include <errno.h>
#include <fcntl.h>

#define LCD_CLEAR               0x01
#define LCD_HOME                0x02
#define INICIA_TELA             10  // Valores para ioctl (switch-case)
#define BACKLIGHT              	11
#define CURSOR			12
#define BLINK			13

// Biblioteca utilizada para os comandos para o display HD44780 a nivel de usuário
// Autores: Luis Fernando Segalla e Igor dos Santos

typedef struct lcd{ // struct relativa à abstração do dispositivo utilizando o driver do display

    int pos_linha; // posicao atual representando cursor para coluna
    int pos_coluna; // posicao atual representando cursor para coluna
    int lin_num; // num total colunas tela (se suporte para mais de um tipo de tela com o driver)
    int col_num; // num total colunas tela (se suporte para mais de um tipo de tela com o driver)
    int fd_display; // inteiro que será o file descriptor
    bool inicializado; // 0 - desativado ; 1 inicializado (Estado display)    

}LCD_t;

// ----------------------------------------- Protótipos Funções biblioteca usuário display 16x02 ------------------------------------------------------------//

LCD_t* LCD_Init(); // retorna a struct relativa à abstração do dispositivo utilizando o driver do display
void LCD_EscreveTexto(LCD_t *display, char* msg); // Escreve a mensagem msg na tela, iniciando na posicao linha = 0, coluna = 0
void LCD_LimpaTela(LCD_t *display); // Limpa a tela do display
void LCD_Backlight(LCD_t *display, int value); // Seleciona backlight ativo ou não, de acordo com o valor de value(0 apagado, 1 ligado)
void LCD_CursorShow(LCD_t *display, int value); // Exibe ou não o cursor da tela, de acordo com o valor passado em value
void LCD_BlinkCursor(LCD_t *display, int value); // Pisca ou não o cursor, de acordo com o valor passado para value(0 -> Não pisca, 1 -> pisca)
void LCD_Finaliza(LCD_t **display); // Finzaliza o uso do display e libera recursos alocados

#endif

