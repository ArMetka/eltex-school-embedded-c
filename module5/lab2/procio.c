#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

MODULE_LICENSE("WTFPL");
MODULE_AUTHOR("Artyom Mironov");
MODULE_DESCRIPTION("procfs basic i/o module");

#define PROC_FILENAME "procio"
#define MSG_MAX_LEN 10

static size_t len = 0;
static char *msg;

static ssize_t read_proc(struct file *filp, char *buf, size_t count, loff_t *offp) {
    if (*offp > 0) {
        return 0;  // EOF
    }

    count = (count > len) ? len : count;
    if (copy_to_user(buf, msg, count)) {
        return -EFAULT;
    }
    *offp = count;

    return count;
}

static ssize_t write_proc(struct file *filp, const char *buf, size_t count, loff_t *offp) {
    len = (count < MSG_MAX_LEN) ? count : MSG_MAX_LEN;
    if (copy_from_user(msg, buf, len)) {
        return -EFAULT;
    }

    return count;
}

static const struct proc_ops proc_fops = {
    .proc_read = read_proc,
    .proc_write = write_proc,
};

static void create_new_proc_entry(void) {  // use of void for no arguments is compulsory now
    msg = kmalloc(sizeof(char) * MSG_MAX_LEN, GFP_KERNEL);
    proc_create(PROC_FILENAME, 0666, NULL, &proc_fops);
}

static int proc_init(void) {
    create_new_proc_entry();
    return 0;
}

static void proc_cleanup(void) {
    remove_proc_entry(PROC_FILENAME, NULL);
    kfree(msg);
}

module_init(proc_init);
module_exit(proc_cleanup);
