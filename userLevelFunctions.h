#ifndef USETLEVELFUNCTIONS_H
#define USETLEVELFUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

// Valores para ioctl (switch-case)
#define INICIA_TELA             10  
#define BACKLIGHT               11
#define CURSOR                  12
#define BLINK                   13
#define LIMPA_TELA              14

#define CURSOR_ADDR             1
#define BLINK_ADDR              0


//Tenta abrir o arquivo em DRIVER_PATH, se não for possível retorna um valor negativo
int OpenDisplay();

void Display();

#endif
