#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/init_task.h>
#include <linux/string.h>

static struct cdev *m_cdev;
static struct class *lyh_dev;
static dev_t dev = 0;
char* shared_buffer=NULL;
#define M_MAJOR 150
#define DEV_NAME "lyh_dev_2"
MODULE_LICENSE("Dual BSD/GPL");

static int fp_open(struct inode *node, struct file *f) { 
  printk(KERN_INFO "User start to Open File!!!\n");
  return 0; 
}

static ssize_t fp_read(struct file *f, char __user *buf, size_t len,
                       loff_t *off) {
  printk(KERN_INFO "User start to read the kernel!!!\n");
  if(shared_buffer==NULL){
    printk(KERN_ERR "Should Write Before Read!!!\n");
    return 0;
  }
  if(copy_to_user(buf,shared_buffer,strlen(shared_buffer))){
    printk(KERN_ERR "Copy Failed\n");
    return 0;
  }
  return strlen(shared_buffer);
}

static ssize_t fp_write(struct file *f, const char *buf, size_t len,
                        loff_t *off) {
  printk(KERN_INFO "User start to Write the kernel!!!\n");
  if(copy_from_user(shared_buffer,buf,len+1)){
    printk(KERN_ERR "Copy Failed\n");
    return 0;
  }
  return strlen(shared_buffer);
}

static int fp_release(struct inode *node, struct file *f) { return 1; }

static struct file_operations fp = {
    .owner = THIS_MODULE,
    .open = fp_open,
    .read = fp_read,
    .write = fp_write,
    .release = fp_release,
};

void Tranverse(void){
    /*宏函数：
        #define for_each_process(p) \
	        for (p = &init_task ; (p = next_task(p)) != &init_task ; )
    */
    printk(KERN_INFO "名称\t\t进程号\t\t状态\t\t优先级\t\t父进程号\n");
    struct task_struct *p=NULL;
    for_each_process(p){
        if(p)
            printk(KERN_INFO "%-20s%-20d%-20d%-20d%-20d\n",p->comm,p->pid,p->stats,p->normal_prio,p->parent->pid);
    }
    return;
}


// 实现模块初始化函数
static int __init hello_init(void) {
  printk(KERN_ERR "Start Init!\n");
  // first , alloca device
  if (alloc_chrdev_region(&dev, 0, 1, DEV_NAME)) {
    printk(KERN_ERR "Alloc Error\n");
    return -1;
  }
#ifdef DEBUG
  printk(KERN_INFO "Major = %d Minor = %d \n", MAJOR(dev), MINOR(dev));
#endif
  m_cdev = cdev_alloc();
  cdev_init(m_cdev, &fp);
  if (cdev_add(m_cdev, dev, 1)) {
    printk(KERN_ERR "Add Error\n");
    return -1;
  }
  // 注册成功后,开始创建设备文件
  lyh_dev = class_create(DEV_NAME);
  if (lyh_dev==NULL) {
	printk(KERN_ERR "Add Error\n");
    return -1;
  }
  if(!device_create(lyh_dev,NULL,dev,NULL,DEV_NAME)){
	printk(KERN_ERR "Device Create Error\n");
    return -1;
  }
  printk(KERN_INFO "Driver Insert In!!!\n");
  shared_buffer=kmalloc(1024,GFP_KERNEL);
  memset(shared_buffer,0,1024);
  Tranverse();
  return 0;
}

// 实现模块退出函数 
static void __exit hello_exit(void) {
  device_destroy(lyh_dev,dev);
  class_destroy(lyh_dev);
  cdev_del(m_cdev);
  unregister_chrdev_region(dev, 1);
  printk(KERN_ERR "Success Exit!\n");
}
// 申明模块初始化函数
module_init(hello_init);
// 申明模块退出函数
module_exit(hello_exit);
