#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/tty.h>
#include <linux/slab.h>

#define DEVICE_NAME "my_chrdev"
#define IOCTL_SET_USER _IO('m', 1)

dev_t mod_dev;
struct cdev mod_cdev;
struct class* mod_class;

char *messages = NULL;
int message_count = 0;

char current_author[256];
static long mod_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch (cmd) {
        case IOCTL_SET_USER:
            copy_from_user(current_author, arg, sizeof(current_author));
            pr_info("user set to %s", current_author);
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

static ssize_t mod_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
    copy_to_user(buffer, messages, len);
    return 0;
}

static ssize_t mod_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {
    // Grow the messages
    message_count++;
    char *temp = kmalloc(1024 * message_count, GFP_KERNEL);

    if (messages != NULL) {
        strcpy(temp, messages);
        kfree(messages);
    }

    messages = temp;
    
    // Add new message to the messages
    char message[1024];
    copy_from_user(message, buffer, len);
    strcat(messages, message);
    pr_info("Received [%s] %s", current_author, message);
    return 0;
}

static const struct file_operations mod_fops = {
    .owner = THIS_MODULE,
    .open = mod_open,
    .release = mod_release,
    .read = mod_read,
    .write = mod_write,
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
