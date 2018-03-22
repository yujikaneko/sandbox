/*
 * bi-directional pipe pseudo device driver
 * kaneko
 */
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/poll.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
#define DEV_NAME "r2l"
#define R2L_MAJOR 777

#define DEVNUM 2

struct R2L_DEVICE {
    int use;
    int len;
    struct semaphore sem;
    wait_queue_head_t read_q;
    char msg[256];
    struct R2L_DEVICE *oppo;
};

static struct R2L_DEVICE *devs;

static int r2l_open(struct inode *inode, struct file *filp) {
    struct R2L_DEVICE *dev = &devs[0];
    if (dev->use) {dev = &devs[1];}
    if (dev->use) {return -EBUSY;}

    if (down_interruptible(&dev->sem)) {
        printk(KERN_INFO "%s: down_interruptible failed\n", __func__);
        return -ERESTARTSYS;
    }
    dev->use = 1;
    dev->len = 0;
    up(&dev->sem);

    filp->private_data = (void *)dev;
    //printk(KERN_INFO "%s\n", __func__);
    return 0;
}

static int r2l_close(struct inode *inode, struct file *filp) {
    struct R2L_DEVICE *dev = (struct R2L_DEVICE *)filp->private_data;

    if (down_interruptible(&dev->sem)) {
        printk(KERN_INFO "%s: down_interruptible failed\n", __func__);
        return -ERESTARTSYS;
    }
    dev->use = 0;
    dev->len = 0;
    up(&dev->sem);

    //printk(KERN_INFO "%s \n", __func__);
    return 0;
}

static ssize_t r2l_read(struct file *filp, char *buf, size_t count,
                        loff_t *pos) {
    struct R2L_DEVICE *dev = (struct R2L_DEVICE *)filp->private_data;

    wait_event_interruptible(dev->read_q, dev->len != 0);

    if (down_interruptible(&dev->sem)) {
        printk(KERN_INFO "%s: down_interruptible failed\n", __func__);
        return -ERESTARTSYS;
    }
    if (dev->len == 0) {
        up(&dev->sem);
        return 0;
    }

    if (count > dev->len) {count = dev->len;}
    if (copy_to_user(buf, dev->msg, count)) {
        up(&dev->sem);
        printk(KERN_INFO "%s: copy_to_user failed\n", __func__);
        return -EFAULT;
    }
    dev->len = 0;
    up(&dev->sem);

    //printk(KERN_INFO "%s %d\n", __func__, count);
    return count;
}

static ssize_t r2l_write(struct file *filp, const char *buf, size_t count,
                         loff_t *pos) {
    struct R2L_DEVICE *dev = (struct R2L_DEVICE *)filp->private_data;
    struct R2L_DEVICE *dst = dev->oppo;

    if (!dst->use) return -EFAULT;

    if (down_interruptible(&dst->sem)) {
        printk(KERN_INFO "%s: down_interruptible failed\n", __func__);
        return -ERESTARTSYS;
    }

    if (count > sizeof(dst->msg)) {count = sizeof(dst->msg);}
    if (copy_from_user(dst->msg, buf, count)) {
        up(&dst->sem);
        printk(KERN_INFO "%s: copy_from_user failed\n", __func__);
        return -EFAULT;
    }

    dst->len = count;
    wake_up_interruptible_sync(&dst->read_q);
    up(&dst->sem);

    //printk(KERN_INFO "%s %d\n", __func__, count);
    return count;
}

static unsigned int r2l_poll(struct file* filp, poll_table* wait) {
    struct R2L_DEVICE *dev = (struct R2L_DEVICE *)filp->private_data;
    unsigned int retmask = 0;

    poll_wait(filp, &dev->read_q, wait);
    if (dev->len > 0) {
        retmask |= (POLLIN | POLLRDNORM);
    }

    return retmask;
}

static long r2l_ioctl(struct file *filep, unsigned int cmd, unsigned long arg) {
    //printk(KERN_INFO "%s, do nothing\n", __func__);
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = r2l_read,
    .write = r2l_write,
    .open = r2l_open,
    .release = r2l_close,
    .unlocked_ioctl = r2l_ioctl,
    .compat_ioctl = r2l_ioctl,
    .poll = r2l_poll,
};

int init_module(void) {
    struct R2L_DEVICE *dev, *oppo;
    int i;

    if (register_chrdev(R2L_MAJOR, DEV_NAME, &fops)) {
        printk(KERN_INFO "%s: register_chrdev failed\n", __func__);
        return -EBUSY;
    }

    devs = (struct R2L_DEVICE *)kmalloc((sizeof(*dev) * DEVNUM), GFP_KERNEL);
    if (!devs) {
        printk(KERN_INFO "%s: kmalloc failed\n", __func__);
        return -ENOMEM;
    }

    oppo = &devs[1];
    for (i = 0; i < DEVNUM; i++) {
        dev = devs + i;
        dev->use = 0;
        dev->len = 0;
        sema_init(&dev->sem, 1);
        init_waitqueue_head(&dev->read_q);
        memset(dev->msg, 0, sizeof(dev->msg));
        dev->oppo = oppo;
        oppo = dev;
    }
    //printk(KERN_INFO "%s\n", __func__);
    return 0;
}

void cleanup_module(void) {
    unregister_chrdev(R2L_MAJOR, DEV_NAME);
    kfree(devs);
    //printk(KERN_INFO "%s\n", __func__);
}
