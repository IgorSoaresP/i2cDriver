
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <iostream>

#include "LCDlib.h"

int main(){

    int q = 0,i;
    LCD_t *display = LCD_Init();

    if(!display->inicializado){ // Falha ao inicializar display
	return 0; 
    } 
    
    while(q < 100){

	LCD_EscreveTexto(display,"teste display");
	for(i=0;i < 100000;i++){}

	LCD_LimpaTela(display);
	for(i=0;i < 100000;i++){}

	LCD_CursorShow(display,1);
	for(i=0;i < 100000;i++){}

	LCD_CursorShow(display,0);
	for(i=0;i < 100000;i++){}

	LCD_BlinkCursor(display,1);
	for(i=0;i < 100000;i++){}

	LCD_BlinkCursor(display,0);
	for(i=0;i < 100000;i++){}

	LCD_LimpaTela(display);
	for(i=0;i < 100000;i++){}

        q++;
    }

    LCD_Finaliza(&display);
    
    return 0;
}

