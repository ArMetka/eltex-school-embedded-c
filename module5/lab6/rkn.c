#include <linux/inet.h>
#include <linux/ip.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netfilter_ipv4.h>
#include <linux/skbuff.h>

MODULE_AUTHOR("Artyom Mironov");
MODULE_DESCRIPTION("Module for blocking IPv4 with netfilter. Controlled via sysfs");
MODULE_LICENSE("GPL");

struct ip_list {
    __be32 ip4;
    struct ip_list *next;
};

static struct nf_hook_ops nfin;
static struct ip_list *blocklist = NULL;
static struct kobject *kobj;

static unsigned int hook_func_in(void *priv, struct sk_buff *skb, const struct nf_hook_state *state) {
    struct iphdr *ip_header;
    struct ip_list *head = blocklist;

    ip_header = (struct iphdr *)skb_network_header(skb);

    while (head) {
        if (ip_header->daddr == head->ip4) {
            pr_info("dropping packet with banned IP: %pI4\n", &ip_header->daddr);
            return NF_DROP;
        }
        head = head->next;
    }

    return NF_ACCEPT;
}

static ssize_t blocklist_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    ssize_t written = 0;
    struct ip_list *head = blocklist;

    while (head) {
        written += sprintf(buf + written, "%pI4\n", &head->ip4);
        head = head->next;
    }

    return written;
}

static ssize_t add_entry_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf,
                               size_t count) {
    struct ip_list *new_node;
    struct ip_list *head = blocklist;

    new_node = kmalloc(sizeof(struct ip_list), GFP_KERNEL);
    if (!new_node) {
        pr_err("failed to allocate memory\n");
        return -ENOMEM;
    }
    new_node->next = NULL;

    if (in4_pton(buf, count, (u8 *)&new_node->ip4, '\n', NULL) != 1) {
        pr_err("wrong IPv4 format!\n");
        kfree(new_node);
        return -EFAULT;
    }

    while (head) {
        if (head->ip4 == new_node->ip4) {
            pr_warn("duplicate IPv4 address\n");
            kfree(new_node);
            return count;
        }
        head = head->next;
    }

    head = blocklist;

    if (!head) {
        blocklist = new_node;
        return count;
    }

    while (head) {
        if (!head->next) {
            head->next = new_node;
            return count;
        }
        head = head->next;
    }

    return count;
}

static ssize_t remove_entry_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf,
                                  size_t count) {
    struct ip_list *head = blocklist;
    struct ip_list *prev = NULL;
    __be32 addr = 0;

    if (!head) {
        return count;
    }

    if (in4_pton(buf, count, (u8 *)&addr, '\n', NULL) != 1) {
        pr_err("wrong IPv4 format!\n");
        return -EFAULT;
    }

    while (head) {
        if (head->ip4 == addr) {
            if (prev) {
                prev->next = head->next;
            } else {
                blocklist = head->next;
            }
            kfree(head);
            return count;
        }
        prev = head;
        head = head->next;
    }

    pr_warn("address not in list");

    return count;
}

static struct kobj_attribute blocklist_show_attr = __ATTR(list, 0664, blocklist_show, NULL);
static struct kobj_attribute add_entry_attr = __ATTR(add, 0664, NULL, add_entry_store);
static struct kobj_attribute remove_entry_attr = __ATTR(remove, 0664, NULL, remove_entry_store);

static int __init init_main(void) {
    nfin.hook = hook_func_in;
    nfin.hooknum = NF_INET_PRE_ROUTING;
    nfin.pf = PF_INET;
    nfin.priority = NF_IP_PRI_FIRST;
    nf_register_net_hook(&init_net, &nfin);

    kobj = kobject_create_and_add("rkn", kernel_kobj);
    if (!kobj) {
        return -ENOMEM;
    }

    if (sysfs_create_file(kobj, &blocklist_show_attr.attr) || sysfs_create_file(kobj, &add_entry_attr.attr) ||
        sysfs_create_file(kobj, &remove_entry_attr.attr)) {
        pr_err("failed to create sysfs files\n");
        return -EFAULT;
    }

    return 0;
}

static void __exit cleanup_main(void) {
    nf_unregister_net_hook(&init_net, &nfin);
    kobject_put(kobj);
}

module_init(init_main);
module_exit(cleanup_main);
