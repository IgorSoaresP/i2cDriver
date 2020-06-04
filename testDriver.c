#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/cdev.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/seq_file.h>
#include <linux/uaccess.h>	/* copy_*_user */
#include <linux/i2c.h>
#include <linux/string.h> //para o strlen no meuWrite

#include "testDriver.h"

MODULE_AUTHOR("Luis Fernando Segalla e Igor Santos");
MODULE_DESCRIPTION("Módulo para comunicação i2c.");
MODULE_LICENSE("GPL");

//major e minor number, usados na alocação de memória para o driver
int meuMajor = 0;
int meuMinor = 0;

//Funções utilitárias
//Descrição: Função usada para "quebrar" a mensagem e envia-la ao display
//Recebe: client: dispositivo com a qual vai se comunicar, data: mensagem, instr: LCD_CHR quando envia caracteres e LCD_CMD quando envia comandos
//Retorna: nada
void write_command(struct i2c_client *client, u8 data, int instr);

//Descrição: Função usada para mostrar ou deixar de mostrar o cursor na tela do display
//Recebe: display: struct que contém o dispositivo escravo, value: 1 mostra o cursor, 0 deixa de mostrar o cursor
//Retorna: nada
void display_cursor(i2cDriverSO_t *display, u8 value);

//Descrição: Função usada para mostrar ou deixar de mostrar o cursor piscando na tela do display
//Recebe: display: struct que contém o dispositivo escravo, value: 1 piscar o cursor, 0 deixa de piscar o cursor
//Retorna: nada
void display_blink(i2cDriverSO_t *display, u8 value);

//Descrição: Função usada para acender ou apagar a backlight do display, só contém dois módos, completamente acesa ou completamente apagada
//Recebe: display: struct que contém o dispositivo escravo, value: 1 acende a tela, 0 apaga a tela
//Retorna: nada
void setBacklight(i2cDriverSO_t *display, u8 value);

//Estruturas e funções referentes a parte i2c do problema

//struct necessárias para o funcionamento do driver
static i2cDriverSO_t      *display;//representação interna do nosso driver


//tabela com o endereço dos devices que meu driver atende //passo 1 do livro: Linux Device Drivers Development_ Develop customized drivers for embedded Linux
static struct i2c_device_id meuIdTable[] = 
{
    {DRIVER_NAME,LCD_ADDRESS},
    {},
};

//Macro que coloca a tabela de ids no espaço de usuário //PASSO 2 do livro: Linux Device Drivers Development_ Develop customized drivers for embedded Linux
MODULE_DEVICE_TABLE(i2c, meuIdTable);

//Funções responsáveis por configurar e depois limpar meu driver //PASSO 3 do livro: Linux Device Drivers Development_ Develop customized drivers for embedded Linux
static int meuProbe(struct i2c_client *client, const struct i2c_device_id *id);
static int meuRemove(struct i2c_client *client);


//Struct responsável por representar meu driver no nível do kernel //PASSO 4 do livro: Linux Device Drivers Development_ Develop customized drivers for embedded Linux
static struct i2c_driver meuDriveri2c = 
{
    .probe      = meuProbe,
    .remove     = meuRemove,
    .id_table   = meuIdTable,
    .driver =
    {
        .owner      = THIS_MODULE,
        .name       = DRIVER_NAME,
    }
};

//Estruturas e funções referente a parte de device drivers

//Descrição: Função usada para escrever na tela
//Recebe: filp: file pointer para o arquivo que representa nosso driver no /dev, buff: mensagem a ser impressa, count: tamanho da mensagem
//Retorna: o número de bits escrito
static ssize_t meuWrite(struct file *filp, const char __user *buff, size_t count, loff_t *offp);
static ssize_t meuRead(struct file *filp, char __user *buff, size_t count, loff_t *offp);
static int meuOpen(struct inode *inode, struct file *filp);
static int meuRelease(struct inode *inode, struct file *filp);

//Descrição: Função usada para chamar outras funções mais específicas do driver
//Recebe: f: file pointer para o arquivo que representa nosso driver no /dev, cmd: usado para definir qual função sera chamada no switch/case. arg: usada em algumas funçẽos para setar valores
//Retorna: 0 caso sucesso
static long meuIoctl(struct file *f, unsigned int cmd, unsigned long arg);

struct file_operations meuFile_operations = 
{
    .owner                   = THIS_MODULE,
    .write                   = meuWrite,
    .read                    = meuRead,
    .open                    = meuOpen,
    .release                 = meuRelease,
    .unlocked_ioctl          = meuIoctl,
};


//IMPLEMENTAÇÃO DAS FUNÇÕES

//Descrição: Função chamada durante a meuInit para que se inicializem algumas variáveis da tela e então escreva uma mensagem na tela
//Recebe: display: usado para ajustar alguns parâmetros
//Retorno: nada
void IniciaTela(i2cDriverSO_t *display)
{
    int i;
    char *init = "Carregando Modulo";
    
    printk(KERN_ALERT "Dentro da função IniciaTela.\n");
    write_command(display->meuCliente,0x33,0);//Inicializa?   
    write_command(display->meuCliente,0x32,0);//Inicializa?

    write_command(display->meuCliente,0x06,0);//Entry Mode
    write_command(display->meuCliente,0x0C,0);
    write_command(display->meuCliente,0x28,0);//Function Set: 4-bit, 2 Line, 5x7 Dots
    
    write_command(display->meuCliente,0x01,0);//Clear Display (also clear DDRAM content)    
       
    write_command(display->meuCliente,0x80,0);//linha 0    
    
    display->linha = display->coluna = 0;
    display->regs_cntrl = 0x0C; // para usar quando houver modificações no blink ou cursor -> padrão = 0x06
    
    mutex_lock(&display->meuMutex);
    
    for(i = 0; i < strlen(init); i++)
    {
        write_command(display->meuCliente,init[i],1);
        display->coluna++;
        if(display->coluna >15)
        {
            if(display->linha == 0)
            {
                display->linha = 1;
                write_command(display->meuCliente,0xC0,0);
            }
            else
            {
                display->linha = 0;
                write_command(display->meuCliente,0x80,0);
            }
            display->coluna = 0;
        }
    }
    
    mdelay(500);
    mdelay(500);
    mdelay(500);
    mdelay(500);

    display->linha = display->coluna = 0;
    write_command(display->meuCliente,0x01,0);//Clear Display (also clear DDRAM content)
    mdelay(500);    

    mutex_unlock(&display->meuMutex);
}

//Descrição: Função chamada durante a meuExit para apagar todo o conteúdo da tela e desligar seu backlight
//Recebe: display: usado para escrita
//Retorno: nada
void DesligaTela(i2cDriverSO_t *display)
{   
    int i;
    char *init = "Descarregando Modulo";
    
    mutex_lock(&display->meuMutex);

    write_command(display->meuCliente,LCD_CLEAR,0);
    udelay(100);
    
    display->coluna = display->linha = 0;

    for(i = 0; i < strlen(init); i++)
    {
        write_command(display->meuCliente,init[i],1);
        display->coluna++;
        if(display->coluna >15)
        {
	    display->coluna = 0;
            if(display->linha == 0)
            {
                display->linha = 1;
                write_command(display->meuCliente,0xC0,0);
            }
            else
            {
                display->linha = 0;
                write_command(display->meuCliente,0x80,0);
            }
            display->coluna = 0;
        }
    }
    
    mdelay(500);
    mdelay(500);
    mdelay(500);
    mdelay(500);

    display->linha = display->coluna = 0;
    
    display_cursor(display,0);
    display_blink(display,0);
    write_command(display->meuCliente,LCD_CLEAR,0);
    udelay(100);
    i2c_smbus_write_byte(display->meuCliente,0);
    mdelay(500);    

    mutex_unlock(&display->meuMutex);
}

static int meuProbe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int out;
    int number = MKDEV(meuMajor,meuMinor);    
    
    printk(KERN_ALERT "DENTRO DA FUNÇÃO meuProbe.\n");
    
    if ( !i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA) )//checo se existe a funcionalidade de comunicação smbus
    {
        printk(KERN_ALERT "FALHOU NO TESTE i2c_check_functionality.\n");
        return -EIO;
    }
    
    display = (i2cDriverSO_t *)devm_kzalloc(&client->dev,sizeof(i2cDriverSO_t),GFP_KERNEL);
    if(!display)
    {
        printk(KERN_ALERT "FALHOU NA HORA DE ALOCAR MEMÓRIA PARA O DRIVER.\n");
        return -ENOMEM;
    }
    
    mutex_lock(&display->meuMutex);
    
    //Inicializando o cdev igual ao exemplo do linux device drivers
    cdev_init(&display->meuCdev, &meuFile_operations);
    display->meuCdev.owner = THIS_MODULE;
    display->meuCdev.ops   = &meuFile_operations;
    out = cdev_add(&display->meuCdev, number,1);
    
    if(out < 0)
    {
        printk(KERN_ALERT "FALHOU NA HORA DE ADICIONAR O DRIVER COM cdev_add.\n");
        return -ENOMEM;
    }
    
    display->meuCliente     = client; 
    display->regs_cntrl     = 0x0C;
    display->linha          = 0;
    display->coluna         = 0;
    
    i2c_set_clientdata(client,display);
    
    mutex_unlock(&display->meuMutex);
    
    printk(KERN_ALERT "PASSOU POR TODOS OS PASSOS SEM PROBLEMAS. RETORNANDO. \n");
    return 0;
}


static int meuRemove(struct i2c_client *client)
{
    printk(KERN_ALERT "DENTRO DA FUNÇÃO meuRemove.\n");
    display = i2c_get_clientdata(client);
    if(!display)
    {
        printk(KERN_ALERT "Erro na função i2c_get_clientdata.\n");
        return -1;
    }
    printk(KERN_ALERT "DEU TUDO CERTO. RETORNANDO.\n");
    return 0;
}


void write_command(struct i2c_client *client, u8 data, int instr)
{
    u8 up,low;

    up  = LCD_DATA_MSB(data); // separando 8 bits em duas partes de 4, pelo modo como é feito o envio
    low = LCD_DATA_LSB(data);

    if(instr == 0){ // se for instrução, não caracteres 
        up  = LCD_RS_INSTR(up); // Avisando que é instrução
        low = LCD_RS_INSTR(low);
    }else{
        up  = LCD_RS_DATA(up); // Avisando que é dados(não instrução)
        low =LCD_RS_DATA(low);        
    }

    up = up | LCD_BACKLIGHT; // display ativo
    low = low | LCD_BACKLIGHT;
    
    // Parte upper bits
    i2c_smbus_write_byte(client,up); // Mandando a parte upper dos 8 bits
    udelay(500);
        
    i2c_smbus_write_byte(client,LCD_E_HI(up)); // Enable high
    udelay(500); 
    i2c_smbus_write_byte(client,LCD_E_LOW(up)); // Enable low -> Para poder enviar os bits, pois precisa de uma troca de borda no enable para envio
    udelay(500);
 
    // Parte lower bits

    i2c_smbus_write_byte(client,low); // Mandando a parte lower dos 8 bits
    udelay(500);
        
    i2c_smbus_write_byte(client,LCD_E_HI(low)); // Enable high
    udelay(500); 
    i2c_smbus_write_byte(client,LCD_E_LOW(low)); // Enable low -> Para poder enviar os bits, pois precisa de uma troca de borda no enable para envio
    udelay(500);
        
    return;
}

static ssize_t meuWrite(struct file *filp, const char __user *buff, size_t count, loff_t *offp)
{
    ssize_t tamSaida = 0;
    int i;
    printk(KERN_ALERT "DENTRO DA FUNÇÃO meuWrite.\n");
    
    if(!buff)//confere se o buffer não está vazio
    {
        printk(KERN_ALERT "Buffer vazio.\n");
        return -ERESTARTSYS;
    }
    
     mutex_lock(&display->meuMutex);
     
     for(i = 0; i < strlen(buff); i++)
     {
        write_command(display->meuCliente,buff[i],1); // 1 no terceiro argumento para dados 
    
        display->coluna++; //incremento minha contagem das colunas
        if(display->coluna > 15) //confiro se estou na ultima coluna
        {
            display->coluna = 0;

            if(display->linha == 0) //confiro se estou na ultima linha
            {
                display->linha = 1;                    
                write_command(display->meuCliente,LCD_LINE1,0); // Para instruções -> Segundo bit RS do display
               
            }
            else
            {
                display->linha = 0;
                write_command(display->meuCliente,LCD_LINE0,0); // Para instruções -> Segundo bit RS do display

            }
         }
    }
     
    mutex_unlock(&display->meuMutex);
    tamSaida = strlen(buff);
    
    return tamSaida;
}

static ssize_t meuRead(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
    printk("DENTRO DA FUNÇÃO meuRead.\n");
    return 0;
}

static int meuOpen(struct inode *inode, struct file *filp)
{
    i2cDriverSO_t *dev;
    printk(KERN_ALERT "CHAMANDO A FUNÇÃO meuOpen.\n");
    
    dev = container_of(inode->i_cdev, i2cDriverSO_t, meuCdev);//uso o ponteiro inode->i_cdev para pegar um ponteiro para a struct i2cDriverSO_t
    filp->private_data = dev;//guardo um ponteiro apra i2cDriverSO_t na minha file struct para um acesso mais fácil no futuro
    
    mutex_lock(&display->meuMutex);
    
    try_module_get(THIS_MODULE);//Chamando para garantir que o módulo não esteja sendo removido enquanto eu tento abri-lo
    
    mutex_unlock(&display->meuMutex);

    write_command(display->meuCliente,LCD_CLEAR,0);
    
    printk(KERN_ALERT "SE DEUS QUISER DEU TUDO CERTO ATÉ AQUI.\n");
    return 0;
}

static int meuRelease(struct inode *inode, struct file *filp)
{   
    printk(KERN_ALERT "CHAMANDO A FUNÇÃO meuRelease.\n");
    module_put(THIS_MODULE);//preciso chamar esta função na hora de sair já que chamei o try_module_get
    return 0;
}


// Funções auxiliares Display -> cursor, setCursorPos, blink;

// Ativa ou desativa a exibição do cursor;

void display_cursor(i2cDriverSO_t *display, u8 value){

	u8 addr = 1 << CURSOR_ADDR;
	
	if(value == 1){
		display->regs_cntrl |= addr;
	}else{
		display->regs_cntrl &= ~(addr);
	}
	
	write_command(display->meuCliente,display->regs_cntrl,0);
    	udelay(200);
	
	return;
}

// Ativa ou desativa a exibição do cursor piscando na tela;

void display_blink(i2cDriverSO_t *display, u8 value){

	u8 addr;
	addr = 1  << BLINK_ADDR;

	if(value == 1){
		display->regs_cntrl |= addr;	
	}else{
		display->regs_cntrl &= ~(addr);
	}
	
	write_command(display->meuCliente,display->regs_cntrl,0);
    	udelay(200);
	
	return;
}
// Ativa ou desativa o backlight, de acordo com o valor value; 0 = backlight apagado; 1 = backlight ligado;
void setBacklight(i2cDriverSO_t *display, u8 value){

	u8 addr;

	if(value == 1){
		addr = 0x08; // valor que aciona o backlight do display
	}else{
		addr = 0;
	}
	
	i2c_smbus_write_byte(display->meuCliente,addr);
    	udelay(100);
	
	return;
}


static long meuIoctl(struct file *f, unsigned int cmd, unsigned long arg)
{
    printk(KERN_ALERT "CHAMANDO A FUNÇÃO meuIoctl.\n");
    mutex_lock(&display->meuMutex);
    
    switch(cmd)
    {
        case LIMPA_TELA://MODIFIQUEI AQUI 
            write_command(display->meuCliente,LCD_CLEAR,0);
            display->linha = display->coluna = 0;
            break;
            
        case INICIA_TELA:
            IniciaTela(display);
            break;
            
        case BACKLIGHT:
            setBacklight(display,arg);
            break;

        case CURSOR:	
            display_cursor(display,arg);
            break;

        case BLINK:	
            display_blink(display,arg);
            break;

        default:
            printk(KERN_ALERT "Comando inválido.\n");

    }
    
    mutex_unlock(&display->meuMutex);
    return 0;
}

//Função chamada na hora que o módulo é carregado no kernel. Usada para inicializa-lo
static int __init meuInit(void)
{
    int result;
    struct i2c_adapter *meuAdapter;//representa meu barramento
    struct i2c_client  *meuCliente;//representa o dispositivo slave
    dev_t dev;//serve para achar os minor e major numbers
    struct i2c_board_info meuBoardinfo = { I2C_BOARD_INFO(DRIVER_NAME, LCD_ADDRESS) };//template para a criação do device
    printk(KERN_ALERT "INICIALIZANDO.\n");
    
    //inicializando a parte do device driver
    
    result = alloc_chrdev_region(&dev,meuMinor,1,DRIVER_NAME);//por opção vamos sempre alocar de forma dinâmica os major numbers
    meuMajor = MAJOR(dev);
    
    if(result < 0 )
    {
        printk(KERN_ALERT "ERRO AO ALOCAR O MMAJOR NUMBER.\n");
        return result;
    }
    
    //inicializando a parte de i2c //baseado no programa do Jesuino e do Lucas Camargo
    
    meuAdapter = i2c_get_adapter(1);//tenta pegar o barramento 1
    if(!meuAdapter)
    {
        printk(KERN_ALERT "ERRO AO ALOCAR O meuAdapter.\n");
        unregister_chrdev_region(dev,1);
        return -EINVAL;
    }
    
    meuCliente = i2c_new_device(meuAdapter, &meuBoardinfo);//cria dispositivo i2c. Quando essa função restorna podemos ligar um driver ao device criado
    printk(KERN_ALERT "LOGO APÓS A CHAMADA DE i2c_new_device\n");//
    if(!meuCliente)
    {
        printk(KERN_ALERT "ERRO AO ALOCAR O meuCliente.\n");
        unregister_chrdev_region(dev,1);
        return -EINVAL;
    }
    
    result = i2c_add_driver(&meuDriveri2c);
    printk(KERN_ALERT "LOGO APÓS A CHAMADA DE i2c_add_driver\n");//
    if(result < 0)
    {
        printk(KERN_ALERT "ERRO AO ADICIONAR o display.\n");
        unregister_chrdev_region(dev,1);
        i2c_unregister_device(meuCliente);
        return -EINVAL;
    }
    
    //i2c_put_adapter(meuAdapter);//dispenso meu adapter com segurança, talvez não precise
    
    IniciaTela(display);
    
    return 0;
}


static void __exit meuExit(void)
{    
    int value = MKDEV(meuMajor,meuMinor);
    printk(KERN_ALERT "SAINDO\n");

    DesligaTela(display);
    
    unregister_chrdev_region(value,1);//libera a região
    unregister_chrdev(meuMajor,DRIVER_NAME);//desregistra o char device
    
    i2c_unregister_device(display->meuCliente);//precisamos chamar essa função já que chamamos i2c_new_device durante o __init
    
    i2c_del_driver(&meuDriveri2c);//deleta o driver i2c//talvez não precise desse aqui, já que ele e o i2c_unregister_device tem a mesma função
}

module_init(meuInit);
module_exit(meuExit);

