#include <linux/init.h>           // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/kernel.h>         // Contains types, macros, functions for the kernel
#include <linux/fs.h>             // Header for the Linux file system support
#include <linux/uaccess.h>        // Required for the copy to user function

#include <linux/ioctl.h>          //necessário para ioctl()  


#define  DEVICE_NAME "i2cSOdriver"    /// Nome do nosso dispositivo, ira aparecer /proc/device
#define  CLASS_NAME  "i2cSO"        ///< The device class -- this is a character device driver

MODULE_LICENSE("GPL");            ///< The license type -- this affects available functionality
MODULE_AUTHOR("Luis Fernando e Igor dos Santos");    ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("A simple Linux char driver for i2c communication");  ///< The description -- see modinfo
MODULE_VERSION("0.1");            ///< A version number to inform users


//////////////////////////////// HEADERS ///////////////////////////////////////////////////////

//inicializa o módulo, registra o character device
static __init int my_init(void);

//limpa qualquer vestígio do módulo
static __exit void my_exit(void);

//chamada quando um processo tenta abrir o device file
static int my_open(struct inode *inode, struct file *file);

//chamada quando um processo fecha o device file
static int my_release(struct inode *inode, struct file *file);

//implementa uma versão do ioctl
static long my_ioctl(struct inode*inode, struct file*file,unsigned int cmd, unsigned long arg);

/////////////////////////////////////////////////////////////////////////////////////////////////

static dev_t majorandminor;//esse parâmetro serve para que possamos guarda o <major,min> numbers
static struct class *i2cClass = NULL;//defino a "classe" i2c
static int NUM_DEVICES = 2;//número de dispositivos que eu irei alocar

//criando essa struct para definir quem é o master e quem é o slave
typedef struct
{
    bool isMaster;
    dev_t major_min;
    struct cdev dev;
}_i2c;

static struct file_operations fops = //Uso esta struct para fazer o kernel convesar com o módulo
{
    .owner = THIS_MODULE,
    
    .unlocked_ioctl = my_ioctl,
}


//inicializa o módulo, registra o character device
static __init int my_init(void)
{
    printk(KERN_INFO "Inicializando o módulo i2c.\n");
    
    int returnValue = 0;
    
    //Registro os major numbers
    returnValue = alloc_chrdev_region(majorandminor, 0, NUM_DEVICES, DEVICE_NAME); //tenta alocar uma faixa de device numbers. Quantos device numbers vamos precisar?? Acho que vão ser 2, 1 slave e 1 master
    if(returnValue < 0)
    {
        printk(KERN_ALERT "Falha ao se alocar os major numbers.\nRetornando...\n");
        return returnValue;
    }
    printk(KERN_INFO "Major numbers alocados com sucesso.\n");
    
    
    //registro a classe do dispositivo
    i2cClass = class_create(THIS_MODULE,CLASS_NAME);
    if(IS_ERR(i2cClass))
    {
        unregister_chrdev_region(majorandminor,2);//NÃO SEI SE ESTÁ CERTA A SINTAXE //desaloca os device numbers que foram alocados dinâmicamente antes
        printk(KERN_ALERT "Falha ao se alocar a classe.\nRetornando...\n");
        return PTR_ERR(i2cClass);
    }
    printk(KERN_INFO "Classe alocada com sucesso.\n");
    
    for(int i = 0; i < NUM_DEVICES; i++)
    {
        dev_t temp = MKDEV(MAJOR(majorandminor),i);
        
        if(i == 0)
        {
            struct device *tempi2cCharDevice = device_create(i2cClass,NULL,temp,NULL,"Master");
            if(IS_ERR(tempi2cCharDevice))
            {
                printk(KERN_ALERT "Falha ao se criar o dispositivo.\nRetornando...\n");
                return PTR_ERR(i2cClass);
            }
        }
        else
        {
            char name[10];
            
            snprintf(name,10,"slave_%d",i);
            
            struct device *tempi2cCharDevice = device_create(i2cClass,NULL,temp,NULL,"Master");
            
            if(IS_ERR(tempi2cCharDevice))
            {
                printk(KERN_ALERT "Falha ao se criar o dispositivo.\nRetornando...\n");
                return PTR_ERR(i2cClass);
            }
        }
    }
    
}

//limpa qualquer vestígio do módulo
static __exit void my_exit(void)
{}

//chamada quando um processo tenta abrir o device file
static int my_open(struct inode *inode, struct file *file)
{}

//chamada quando um processo fecha o device file
static int my_release(struct inode *inode, struct file *file)
{}


static long my_ioctl(struct inode*inode, struct file*file,unsigned int cmd, unsigned long arg)
{
    //confere se as entradas em cmd e arg são válidas
    
    switch(cmd)
    {
        case
        //...
    }
    
}
