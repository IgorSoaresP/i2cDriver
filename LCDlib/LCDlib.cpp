
#include <iostream>
#include <errno.h>
#include "LCDlib.h"

// Implementações das funções de nível de usuário para tela 16x02 HD44780

// Retorna um ponteiro para a struct LCD_t, que faz uma abstração do display para o usuário

LCD_t* LCD_Init(){

	LCD_t* disp = new LCD_t; // Alocando struct que representa de forma abstraida o display, para o usuario
	disp->pos_linha = 0;
	disp->pos_coluna = 0;
	disp->col_num = 16; // display 16x02
	disp->lin_num = 2;

	char* dir_device = "/dev/DriverSOi2c"; //Diretório no qual o device está representado em /dev

	disp->fd_display = open(dir_device, O_RDWR); // Abertura do arquivo em /dev para possibilitar a posterior comunicação
        
    if(disp->fd_display < 0 ){
    		std::cerr << "Falha na inicialização do dispositivo. Verifique se o módulo(driver) para este device está instalado!\n";
		    return nullptr;
    }	

	LCD_LimpaTela(disp); // retirando possiveis valores salvos nos regs de dados do display
	for(int i=0;i < 10000;i++){} // delay

    LCD_Backlight(disp,1); // Acionando backlight
	for(int i=0;i < 10000;i++){} // delay

	return disp;
}


// Recebe um ponteiro para o display e a string da mensagem como parametros
// Retorno: void
// Escreve a mensagem msg no display representado pelo  ponteiro para a struct LCD_t *display

void LCD_EscreveTexto(LCD_t *display, char* msg){
	
	if(display->inicializado){ // Device válido
		write(display->fd_display,msg,0); // Envio para função write da struct file operations, relativo as funções disponíveis para o device
	}else{
		std::cerr << "Display nao inicializado!\n";
	}

	return;
}


void LCD_LimpaTela(LCD_t* display){

    if(display->inicializado){ // Se foi previamente inicializado o display
    	ioctl(display->fd_display,LCD_CLEAR,0); 
    }else{
        std::cerr << ("Display não inicializado!\n");
    }

    return;
}


void LCD_Backlight(LCD_t* display,int value){


    if(display->inicializado){ // Se foi previamente inicializado o display
        ioctl(display->fd_display,BACKLIGHT,value);
    }else{
        std::cerr << ("Display não inicializado!\n");
    }
		
    return;
}


void LCD_CursorShow(LCD_t* display,int value){


    if(display->inicializado){ // Se foi previamente inicializado o display
        ioctl(display->fd_display,CURSOR,value);
    }else{
        std::cerr << ("Display não inicializado!\n");
    }
		
    return;
}


void LCD_BlinkCursor(LCD_t* display,int value){


    if(display->inicializado){ // Se foi previamente inicializado o display
        ioctl(display->fd_display,BLINK,value); 
    }else{
        std::cerr << ("Display não inicializado!\n");
    }
		
    return;
}



void LCDFinaliza(LCD_t** display){
	
    if(display == nullptr || *display == nullptr){
        return;
    }

    int result = close((*display)->fd_display);
    if(result < 0){
        std::cerr << "Erro ao finalizar operação!\n";
        return;
    }

    (*display)->inicializado = false;
    delete (*display); // Desalocando struct representante do display -> arquivo relativo ao device já fechado

    return;
}


