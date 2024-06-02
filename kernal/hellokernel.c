#include <cstddef>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
static struct cdev *m_cdev;
static struct class *lyh_dev;
static dev_t dev = 0;
#define M_MAJOR 150
#define DEV_NAME "lyh_dev"
MODULE_LICENSE("Dual BSD/GPL");

static int fp_open(struct inode *node, struct file *f) { return 1; }

static ssize_t fp_read(struct file *f, char __user *buf, size_t len,
                       loff_t *off) {
  return 1;
}

static ssize_t fp_write(struct file *f, const char *buf, size_t len,
                        loff_t *off) {
  return 1;
}

static int fp_release(struct inode *node, struct file *f) { return 1; }

static struct file_operations fp = {
    .owner = THIS_MODULE,
    .open = fp_open,
    .read = fp_read,
    .write = fp_write,
    .release = fp_release,
};

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
  lyh_dev = class_creat(THIS_MODULE, DEV_NAME);
  if (lyh_dev!=NULL) {
	printk(KERN_ERR "Add Error\n");
    return -1;
  }

  return 0;
}

// 实现模块退出函数 
static void __exit hello_exit(void) {
  cdev_del(m_cdev);
  unregister_chrdev_region(dev, 1);
  printk(KERN_ERR "Success Exit!\n");
}
// 申明模块初始化函数
module_init(hello_init);
// 申明模块退出函数
module_exit(hello_exit);
