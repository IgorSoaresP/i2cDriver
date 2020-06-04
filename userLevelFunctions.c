#include "userLevelFunctions.h"

//Tenta abrir o arquivo em DRIVER_PATH, se não for possível retorna um valor negativo
int OpenDisplay()
{
    return open("/dev/DriverSOi2c", O_RDWR);
}

void Display()
{
    int in = 10000;
    int fd = OpenDisplay();
    char text[32];
    if(fd < 0)
    {
        printf("Erro ao abrir o arquivo.\n");
        return;
    }
    
    while( in != 0 )
    {
        printf("Digite seu comando mestre: \n0 - Sair\n1 - Escreve na tela\n2 - Apagar o display\n3 - Acende o display\n4 - Limpar o display\n5 - Ativa o cursor\n6 - Desativa o cursor\n7 - Pisca o cursor\n8 - Para de piscar o cursor\n");
        scanf("%d", &in);
        
        switch(in)
        {
	    case 0:
                ioctl(fd,LIMPA_TELA,0);
	        write(fd,"Saindo",0);
	   	close(fd);
                break;
            case 1:
                fgets(text,sizeof(text),stdin);
		text[strlen(text) - 1] = '\0';
                write(fd,text,0);
                break;
            
            case 2:
                ioctl(fd,BACKLIGHT,0);
                break;
                
            case 3:
                ioctl(fd,BACKLIGHT,1);
                break;
                
            case 4:
                ioctl(fd,LIMPA_TELA,0);
                break;
                
            case 5:
                ioctl(fd,CURSOR,1);
                break;
                
            case 6:
                ioctl(fd,CURSOR,0);
                break;
                
            case 7:
                ioctl(fd,BLINK,1);
                break;
                
            case 8:
                ioctl(fd,BLINK,0);
                break;
                
            default:
                printf("COMANDO INVÁLIDO.\n");
                break;
        }
    }
}
