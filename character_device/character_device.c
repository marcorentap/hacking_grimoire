#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>

#define DEVICE_NAME "my_chrdev"
#define MY_IOCTL_CMD  _IO('m', 1)

dev_t mod_dev;
struct cdev mod_cdev;
struct class* mod_class;

static long mod_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    switch (cmd) {
        case MY_IOCTL_CMD:
            printk(KERN_INFO "Received MY_IOCTL_CMD\n");
            // do something here
            break;
        default:
            return -ENOTTY;
    }
    return 0;
}

static int mod_open(struct inode *inode, struct file *file) {
  pr_info("Open device!");
  return 0;
}

static int mod_release(struct inode *inode, struct file *file) {
  pr_info("Close device!");
  return 0;
}

static const struct file_operations mod_fops = {
    .owner = THIS_MODULE,
    .open = mod_open,
    .release = mod_release,
    //.read = mod_read,
    //.write = mod_write,
    .unlocked_ioctl = mod_ioctl,
};

static int __init mod_init(void) {

    /*Allocating Major number*/
    if ((alloc_chrdev_region(&mod_dev, 0, 1, DEVICE_NAME)) < 0) {
        pr_err("Cannot allocate major number\n");
        return -1;
    }
    pr_info("Major = %d Minor = %d \n", MAJOR(mod_dev), MINOR(mod_dev));

    /*Creating cdev structure*/
    cdev_init(&mod_cdev, &mod_fops);

    /*Adding character device to the system*/
    if ((cdev_add(&mod_cdev, mod_dev, 1)) < 0) {
        pr_err("Cannot add the device to the system\n");
        goto r_class;
    }

    /*Creating struct class*/
    if ((mod_class = class_create(THIS_MODULE, "mod_class")) == NULL) {
        pr_err("Cannot create the struct class\n");
        goto r_class;
    }

    /*Creating device*/
    if ((device_create(mod_class, NULL, mod_dev, NULL, DEVICE_NAME)) ==
      NULL) {
        pr_err("Cannot create the Device 1\n");
        goto r_device;
    }
    pr_info("Device Driver Insert...Done!!!\n");
    return 0;

    r_device:
        class_destroy(mod_class);
    r_class:
        unregister_chrdev_region(mod_dev, 1);
        return -1;
    return 0;
}


static void __exit mod_exit(void) {
  device_destroy(mod_class, mod_dev);
  class_destroy(mod_class);
  cdev_del(&mod_cdev);
  unregister_chrdev_region(mod_dev, 1);
  pr_info("Device Driver Remove...Done!!!\n");
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Marc Owen");
MODULE_DESCRIPTION("Example kernel module that uses ioctl");
