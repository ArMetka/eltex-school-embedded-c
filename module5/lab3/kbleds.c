#include <linux/console.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kd.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/timer.h>
#include <linux/tty.h>
#include <linux/vt.h>
#include <linux/vt_kern.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Artyom Mironov");
MODULE_DESCRIPTION("Module for keyboard leds blinking control via sysfs");

#define BLINK_DELAY HZ / 5
#define RESTORE_LEDS 0xFF

// timer & ioctl
static struct timer_list update_timer;
static int curr_led_state = 0;

// sysfs
static struct kobject *kobj;
static int ledctl = 0;

static void my_timer_func(struct timer_list *t) {
    struct tty_struct *tty;
    int ret;

    // Get the foreground console's tty
    tty = vc_cons[fg_console].d->port.tty;
    if (!tty) {
        printk(KERN_WARNING "kbleds: No tty available\n");
        goto reschedule;
    }

    curr_led_state = (curr_led_state == ledctl) ? 0 : ledctl;
    ret = tty->driver->ops->ioctl(tty, KDSETLED, curr_led_state);
    if (ret < 0) {
        printk(KERN_WARNING "kbleds: ioctl failed with error %d\n", ret);
    }

reschedule:
    mod_timer(&update_timer, jiffies + BLINK_DELAY);
}

static ssize_t ledctl_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    return sprintf(buf, "%d\n", ledctl);
}

static ssize_t ledctl_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf,
                            size_t count) {
    sscanf(buf, "%du", &ledctl);
    return count;
}

static struct kobj_attribute ledctl_attr = __ATTR(ledctl, 0664, ledctl_show, ledctl_store);

static int __init kbleds_init(void) {
    int error = 0;

    printk(KERN_INFO "kbleds: loading\n");
    printk(KERN_INFO "kbleds: fgconsole is %x\n", fg_console);

    // Check if we have a valid tty
    if (!vc_cons[fg_console].d || !vc_cons[fg_console].d->port.tty) {
        printk(KERN_ERR "kbleds: No tty available for console %d\n", fg_console);
        return -ENODEV;
    }

    // Create sysfs file
    kobj = kobject_create_and_add("kbleds", kernel_kobj);
    if (!kobj) {
        return -ENOMEM;
    }

    error = sysfs_create_file(kobj, &ledctl_attr.attr);
    if (error) {
        pr_debug("kbleds: failed to create sysfs file in /sys/kernel/kbleds\n");
        return error;
    }

    // Set up timer
    timer_setup(&update_timer, my_timer_func, 0);
    mod_timer(&update_timer, jiffies + BLINK_DELAY);

    return 0;
}

static void __exit kbleds_cleanup(void) {
    struct tty_struct *tty;

    printk(KERN_INFO "kbleds: unloading...\n");

    kobject_put(kobj);
    del_timer_sync(&update_timer);

    // Restore normal LED operation
    tty = vc_cons[fg_console].d->port.tty;
    if (tty) {
        tty->driver->ops->ioctl(tty, KDSETLED, RESTORE_LEDS);
    }
}

module_init(kbleds_init);
module_exit(kbleds_cleanup);
