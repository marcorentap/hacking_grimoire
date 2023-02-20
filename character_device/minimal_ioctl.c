#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define MY_IOCTL_CMD  _IO('m', 1)

static long my_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
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

static const struct file_operations my_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = my_ioctl,
};

static int my_init(void)
{
    int ret = register_chrdev(0, "my_module", &my_fops);
    if (ret < 0) {
        printk(KERN_ERR "Failed to register character device\n");
        return ret;
    } else {
        printk(KERN_INFO "Registered chrdev with major number %d\n", ret);
    }
    printk(KERN_INFO "my_module loaded\n");
    return 0;
}

static void my_exit(void)
{
    unregister_chrdev(0, "my_module");
    printk(KERN_INFO "my_module unloaded\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Example kernel module that uses ioctl");
