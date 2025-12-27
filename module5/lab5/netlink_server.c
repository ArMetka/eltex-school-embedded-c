#include <linux/module.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <net/net_namespace.h>
#include <net/sock.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Artyom Mironov");
MODULE_DESCRIPTION("Module for simple communication via netlink");

#define NETLINK_USER 31
#define BUF_SIZE 128

struct sock *nl_sk = NULL;

static void hello_nl_recv_msg(struct sk_buff *skb) {
    struct nlmsghdr *nlh;
    int pid;
    struct sk_buff *skb_out;
    int msg_size;
    int res;
    char buf[BUF_SIZE];
    
    nlh = (struct nlmsghdr *)skb->data;
    pr_info("Netlink received msg payload: %s\n", (char *)nlmsg_data(nlh));
    msg_size = strlen((char *)nlmsg_data(nlh));
    msg_size = msg_size >= BUF_SIZE ? BUF_SIZE - 1 : msg_size;
    strncpy(buf, (char *)nlmsg_data(nlh), msg_size);
    pid = nlh->nlmsg_pid; /*pid of sending process */

    skb_out = nlmsg_new(msg_size, 0);

    if (!skb_out) {
        pr_err("Failed to allocate new skb\n");
        return;
    }
    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
    NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
    strncpy(nlmsg_data(nlh), buf, msg_size);

    res = nlmsg_unicast(nl_sk, skb_out, pid);

    if (res < 0) {
        pr_warn("Error while sending bak to user\n");
    }
}

struct netlink_kernel_cfg cfg = {
    .groups = 1,
    .input = hello_nl_recv_msg,
};

static int __init hello_init(void) {
    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);

    if (!nl_sk) {
        pr_alert("Error creating socket\n");
        return -10;
    }

    return 0;
}

static void __exit hello_exit(void) {
    netlink_kernel_release(nl_sk);
}

module_init(hello_init);
module_exit(hello_exit);
