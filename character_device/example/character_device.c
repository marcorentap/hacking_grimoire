#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/ioctl.h>

//
// ...
struct file_operations hello_fops = {
    .owner = THIS_MODULE,
    .open = hello_open,
    .release = hello_release,
    .read = hello_read,
    .write = hello_write,
    .unlocked_ioctl = hello_ioctl,
};

//
// ...
static int hello_open(struct inode *inode, struct file *file) {
  pr_info("Open hello!");
  return 0;
}

static int hello_release(struct inode *inode, struct file *file) {
  pr_info("Close hello!");
  return 0;
}

// ...
static long hello_ioctl(struct file *file, unsigned int cmd,
                        unsigned long arg) {
  if (cmd == IOCTL_LOG) {
    struct hello_msg msg;
    if (copy_from_user(&msg, (struct hello_msg *)arg,
                       sizeof(struct hello_msg))) {
      pr_info("IOCTL_LOG: %s", msg.msg);
      pr_err("IOCTL_LOG Err!\n");
    }
  } else {
    pr_info("UNKOWN IOCTL CMD");
  }
  return 0;
}


// ...
static int __init hello_init(void) {
  /*Allocating Major number*/
  if ((alloc_chrdev_region(&hello_dev, 0, 1, DEVICE_NAME)) < 0) {
    pr_err("Cannot allocate major number\n");
    return -1;
  }
  pr_info("Major = %d Minor = %d \n", MAJOR(hello_dev), MINOR(hello_dev));

  /*Creating cdev structure*/
  cdev_init(&hello_cdev, &hello_fops);

  /*Adding character device to the system*/
  if ((cdev_add(&hello_cdev, hello_dev, 1)) < 0) {
    pr_err("Cannot add the device to the system\n");
    goto r_class;
  }

  /*Creating struct class*/
  if ((hello_class = class_create(THIS_MODULE, "hello_class")) == NULL) {
    pr_err("Cannot create the struct class\n");
    goto r_class;
  }

  /*Creating device*/
  if ((device_create(hello_class, NULL, hello_dev, NULL, DEVICE_NAME)) ==
      NULL) {
    pr_err("Cannot create the Device 1\n");
    goto r_device;
  }
  pr_info("Device Driver Insert...Done!!!\n");
  return 0;

r_device:
  class_destroy(hello_class);
r_class:
  unregister_chrdev_region(hello_dev, 1);
  return -1;
  return 0;
}

static void __exit hello_exit(void) {
  device_destroy(hello_class, hello_dev);
  class_destroy(hello_class);
  cdev_del(&hello_cdev);
  unregister_chrdev_region(hello_dev, 1);
  pr_info("Device Driver Remove...Done!!!\n");
}

module_init(hello_init);
module_exit(hello_exit);
