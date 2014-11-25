
/*
 *  drivers/net/deth.c
 *
 *  Copyright (C) 2013 TNT Lab Unige Italy
 *
 * Author: Sergio Mangialardi <s.mangialardi@econet-project.eu>
 * Based on veth module from: Pavel Emelianov <xemul@openvz.org>
 * Ethtool interface from: Eric W. Biederman <ebiederm@xmission.com>
 *
 */

#include "deth.h"

#include <linux/netdevice.h>
#include <linux/slab.h>
#include <linux/ethtool.h>
#include <linux/etherdevice.h>
#include <linux/u64_stats_sync.h>
#include <linux/list.h>

#include <net/dst.h>
#include <net/xfrm.h>
#include <net/net_namespace.h>
#include <net/netns/generic.h>
#include <linux/veth.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/netlink.h>

#define DRV_NAME        "deth"
#define DRV_VERSION     "1.0"

MODULE_DESCRIPTION("DROP Virtual Ethernet Device");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS_RTNL_LINK(DRV_NAME);

#define NETLINK_DETH 31
#define NETLINK_DETH_GROUP 1

#define MIN_MTU 68              // Min L3 MTU
#define MAX_MTU 65535           // Max L3 MTU (arbitrary)

static int deth_net_id;

struct deth_net
{
    struct sock* nl_sk;
};

struct deth_net_stats
{
    u64                     rx_packets;
    u64                     rx_bytes;
    u64                     rx_dropped;
    u64                     tx_packets;
    u64                     tx_bytes;
    u64                     tx_dropped;
    struct u64_stats_sync   syncp;
};

struct deth_priv
{
    struct deth_net_stats __percpu* stats;
};

static int deth_netlink_do_broadcast_packet(struct sock* nl_sk, const char* pkt, int size, u32 index)
{
    int totsize = size + sizeof(u32);

    struct sk_buff* skb = nlmsg_new(totsize, 0);
    struct nlmsghdr* nlh;

    char* ptr;

    if (!skb)
    {
        printk(KERN_ERR "Failed to allocate new skb\n");

        return -ENOBUFS;
    }

    nlh = nlmsg_put(skb, 0, 0, PACKET, totsize, 0);

    if (nlh == NULL)
    {
        nlmsg_free(skb);
        
        return -EMSGSIZE;
    }

    ptr = nlmsg_data(nlh);

    memcpy(ptr, &index, sizeof(u32));
    ptr += sizeof(u32);
    memcpy(ptr, pkt, size);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
	NETLINK_CB(skb).portid = 0;
#else
    NETLINK_CB(skb).pid = 0;
#endif

    NETLINK_CB(skb).dst_group = NETLINK_DETH_GROUP;

    return netlink_broadcast(nl_sk, skb, 0, NETLINK_DETH_GROUP, GFP_KERNEL);
}

static int deth_netlink_do_broadcast_addresses(struct sock* nl_sk, u32 index)
{
    struct sk_buff* skb = nlmsg_new(sizeof(u32), 0);
    struct nlmsghdr* nlh;

    if (!skb)
    {
        printk(KERN_ERR "Failed to allocate new skb\n");

        return -ENOBUFS;
    }

    nlh = nlmsg_put(skb, 0, 0, MULTICAST_ADDRESS_CHANGE, sizeof(u32), 0);

    if (nlh == NULL)
    {
        nlmsg_free(skb);
        
        return -EMSGSIZE;
    }

    memcpy(nlmsg_data(nlh), &index, sizeof(u32));

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
	NETLINK_CB(skb).portid = 0;
#else
    NETLINK_CB(skb).pid = 0;
#endif

    NETLINK_CB(skb).dst_group = NETLINK_DETH_GROUP;

    return netlink_broadcast(nl_sk, skb, 0, NETLINK_DETH_GROUP, GFP_KERNEL);
}

static int deth_netlink_broadcast_packet(struct net* net, const char* pkt, int size, u32 index)
{
    struct deth_net* dn = net_generic(net, deth_net_id);

    if (!dn || !dn->nl_sk || !netlink_has_listeners(dn->nl_sk, NETLINK_DETH_GROUP))
    {
        return -ESRCH;
    }

    return deth_netlink_do_broadcast_packet(dn->nl_sk, pkt, size, index);
}

static int deth_netlink_broadcast_addresses(struct net* net, u32 index)
{
    struct deth_net* dn = net_generic(net, deth_net_id);

    if (!dn || !dn->nl_sk || !netlink_has_listeners(dn->nl_sk, NETLINK_DETH_GROUP))
    {
        return -ESRCH;
    }

    return deth_netlink_do_broadcast_addresses(dn->nl_sk, index);
}

static int deth_get_settings(struct net_device* dev, struct ethtool_cmd* cmd)
{
    cmd->supported          = 0;
    cmd->advertising        = 0;
    ethtool_cmd_speed_set(cmd, SPEED_1000);
    cmd->duplex             = DUPLEX_FULL;
    cmd->port               = PORT_TP;
    cmd->phy_address        = 0;
    cmd->transceiver        = XCVR_INTERNAL;
    cmd->autoneg            = AUTONEG_DISABLE;
    cmd->maxtxpkt           = 0;
    cmd->maxrxpkt           = 0;

    return 0;
}

static void deth_get_drvinfo(struct net_device* dev, struct ethtool_drvinfo* info)
{
    strlcpy(info->driver, DRV_NAME, sizeof(info->driver));
    strlcpy(info->version, DRV_VERSION, sizeof(info->version));
}

static void deth_get_ethtool_stats(struct net_device* dev, struct ethtool_stats* stats, u64* data)
{
    data[0] = dev->ifindex;
}

static const struct ethtool_ops deth_ethtool_ops =
{
    .get_settings           = deth_get_settings,
    .get_drvinfo            = deth_get_drvinfo,
    .get_link               = ethtool_op_get_link,
    .get_ethtool_stats      = deth_get_ethtool_stats,
};

static netdev_tx_t deth_xmit(struct sk_buff* skb, struct net_device* dev)
{
    struct deth_priv* priv = netdev_priv(dev);
    struct deth_net_stats* stats = this_cpu_ptr(priv->stats);
    unsigned char* p = skb->data;
    int length = skb->len;

    if (deth_netlink_broadcast_packet(dev_net(dev), p, length, dev->ifindex) != 0)
    {
        goto tx_drop;
    }

    u64_stats_update_begin(&stats->syncp);
    stats->tx_bytes += length;
    stats->tx_packets++;
    u64_stats_update_end(&stats->syncp);

    return NETDEV_TX_OK;
tx_drop:
    u64_stats_update_begin(&stats->syncp);
    stats->tx_dropped++;
    u64_stats_update_end(&stats->syncp);

    return NETDEV_TX_OK;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,15,0)

#define u64_stats_fetch_begin_bh u64_stats_fetch_begin_irq
#define u64_stats_fetch_retry_bh u64_stats_fetch_retry_irq

#endif

static struct rtnl_link_stats64* deth_get_stats64(struct net_device* dev, struct rtnl_link_stats64* tot)
{
    struct deth_priv* priv = netdev_priv(dev);
    int cpu;

    for_each_possible_cpu(cpu)
    {
        struct deth_net_stats *stats = per_cpu_ptr(priv->stats, cpu);
        u64 rx_packets;
        u64 rx_bytes;
        u64 rx_dropped;
        u64 tx_packets;
        u64 tx_bytes;
        u64 tx_dropped;
        unsigned int start;

        do
        {
            start = u64_stats_fetch_begin_bh(&stats->syncp);
            rx_packets = stats->rx_packets;
            tx_packets = stats->tx_packets;

            rx_bytes = stats->rx_bytes;
            tx_bytes = stats->tx_bytes;
            
            rx_dropped = stats->rx_dropped;
            tx_dropped = stats->tx_dropped;
        }
        while (u64_stats_fetch_retry_bh(&stats->syncp, start));

		tot->rx_packets += rx_packets;
        tot->tx_packets += tx_packets;
        tot->rx_bytes   += rx_bytes;
        tot->tx_bytes   += tx_bytes;
        tot->rx_dropped += rx_dropped;
        tot->tx_dropped += tx_dropped;
    }

    return tot;
}

static int deth_open(struct net_device* dev)
{
    return 0;
}

static int deth_close(struct net_device* dev)
{
    netif_carrier_off(dev);

    return 0;
}

static int is_valid_deth_mtu(int new_mtu)
{
    return new_mtu >= MIN_MTU && new_mtu <= MAX_MTU;
}

static int deth_change_mtu(struct net_device* dev, int new_mtu)
{
    if (!is_valid_deth_mtu(new_mtu))
    {
        return -EINVAL;
    }

    dev->mtu = new_mtu;

    return 0;
}

static void deth_change_rx_flags(struct net_device* dev, int flags)
{
    printk(KERN_DEBUG "deth_change_rx_flags: %d\n", flags);
}

static int deth_ioctl(struct net_device* dev, struct ifreq* ifr, int cmd)
{
    printk(KERN_DEBUG "deth_ioctl: %d\n", cmd);

    return 0;
}

static void deth_set_rx_mode(struct net_device* dev)
{
    int rc = deth_netlink_broadcast_addresses(dev_net(dev), dev->ifindex);

    if (rc != 0 && rc != -ESRCH)
    {
        printk(KERN_ERR "deth_set_rx_mode: error sending Netlink message (%d)\n", rc);
    }
}

static int deth_dev_init(struct net_device* dev)
{
    struct deth_net_stats __percpu * stats = alloc_percpu(struct deth_net_stats);
    struct deth_priv* priv;

    if (stats == NULL)
    {
        return -ENOMEM;
    }

    priv = netdev_priv(dev);
    priv->stats = stats;

    return 0;
}

static void deth_dev_free(struct net_device* dev)
{
    struct deth_priv* priv = netdev_priv(dev);

    free_percpu(priv->stats);
    free_netdev(dev);
}

static const struct net_device_ops deth_netdev_ops = {
    .ndo_init            = deth_dev_init,
    .ndo_open            = deth_open,
    .ndo_stop            = deth_close,
    .ndo_start_xmit      = deth_xmit,
    .ndo_change_mtu      = deth_change_mtu,
    .ndo_get_stats64     = deth_get_stats64,
    .ndo_set_mac_address = eth_mac_addr,
    .ndo_change_rx_flags = deth_change_rx_flags,
    .ndo_do_ioctl        = deth_ioctl,
    .ndo_set_rx_mode     = deth_set_rx_mode,
};

static void deth_setup(struct net_device *dev)
{
    ether_setup(dev);

    dev->priv_flags &= ~IFF_TX_SKB_SHARING;

    dev->netdev_ops = &deth_netdev_ops;
    dev->ethtool_ops = &deth_ethtool_ops;
    dev->features |= NETIF_F_LLTX;
    dev->destructor = deth_dev_free;

    dev->hw_features = NETIF_F_HW_CSUM | NETIF_F_SG | NETIF_F_RXCSUM;
}

static int deth_validate(struct nlattr* tb[], struct nlattr* data[])
{
    if (tb[IFLA_ADDRESS])
    {
        if (nla_len(tb[IFLA_ADDRESS]) != ETH_ALEN)
        {
            return -EINVAL;
        }

        if (!is_valid_ether_addr(nla_data(tb[IFLA_ADDRESS])))
        {
            return -EADDRNOTAVAIL;
        }
    }

    if (tb[IFLA_MTU])
    {
        if (!is_valid_deth_mtu(nla_get_u32(tb[IFLA_MTU])))
        {
            return -EINVAL;
        }
    }

    return 0;
}

static struct rtnl_link_ops deth_link_ops;

static void deth_nl_recv_msg(struct sk_buff* skb)
{
    struct nlmsghdr* nlh = (struct nlmsghdr*) skb->data;
    
    int pktsize = 0;

    const char* ptr = nlmsg_data(nlh);
    u32 index = *(u32*)ptr;

    struct deth_priv* priv = NULL;
    struct deth_net_stats* stats = NULL;
    struct sk_buff* out_skb = NULL;
    struct net* net = sock_net(skb->sk);
    struct net_device* dev = dev_get_by_index(net, index);

    ptr += sizeof(u32);

    if (dev == NULL)
    {
        printk(KERN_ERR "Error retrieving the interface with index %d.\n", index);

        return;
    }

    switch (nlh->nlmsg_type)
    {
    case PACKET:
        break;
    case LINK_CHANGE:
        {
            if (*(u32*)(ptr))
            {
                netif_carrier_on(dev);
            }
            else
            {
                netif_carrier_off(dev);
            }
    
            goto cleanup;
        }
	case LINK_STATS:
		{
			priv = netdev_priv(dev);
			stats = this_cpu_ptr(priv->stats);
			u64_stats_update_begin(&stats->syncp);

			stats->rx_packets = *(u64*)(ptr);
			ptr += sizeof(u64);
			stats->rx_bytes = *(u64*)(ptr);
			ptr += sizeof(u64);
			stats->tx_packets = *(u64*)(ptr);
			ptr += sizeof(u64);
			stats->tx_bytes = *(u64*)(ptr);

			u64_stats_update_end(&stats->syncp);

			goto cleanup;
		}
    default:
        goto cleanup;
    }

    pktsize = nlh->nlmsg_len - sizeof(u32);
    out_skb = netdev_alloc_skb(dev, pktsize + 2);

    if (out_skb == NULL)
    {
        printk(KERN_ERR "Error allocating a sk_buff (%d bytes).\n", pktsize);

        goto rx_drop;
    }

    memcpy(skb_put(out_skb, pktsize), ptr, pktsize);

    out_skb->ip_summed = CHECKSUM_UNNECESSARY;

    if (dev_forward_skb(dev, out_skb) != NET_RX_SUCCESS)
    {
        printk(KERN_ERR "dev_forward_skb failed to forward a packet of %d bytes.\n", pktsize);

        goto rx_drop;
    }

	priv = netdev_priv(dev);
    stats = this_cpu_ptr(priv->stats);
    u64_stats_update_begin(&stats->syncp);
    stats->rx_bytes += pktsize;
    stats->rx_packets++;
    u64_stats_update_end(&stats->syncp);

    goto cleanup;

rx_drop:
    u64_stats_update_begin(&stats->syncp);
    stats->tx_dropped++;
    u64_stats_update_end(&stats->syncp);

cleanup:
    dev_put(dev);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0)

static struct netlink_kernel_cfg deth_netlink_cfg = {
    //.groups = ??,
    //.flags = ??,
    .input = deth_nl_recv_msg,
    //.cb_mutex = ,
    //.bind = ??,
};

#endif

static int deth_init_net(struct net* net)
{
    struct deth_net* dn = net_generic(net, deth_net_id);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,7,0)
    dn->nl_sk = netlink_kernel_create(net, NETLINK_DETH, &deth_netlink_cfg);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0)
    dn->nl_sk = netlink_kernel_create(net, NETLINK_DETH, THIS_MODULE, &deth_netlink_cfg);
#else
    dn->nl_sk = netlink_kernel_create(net, NETLINK_DETH, 0, deth_nl_recv_msg, NULL, THIS_MODULE);
#endif

    if (!dn->nl_sk)  
    {   
        printk(KERN_ALERT "Error creating netlink socket.\n");  
    
        return -10;  
    }

    return 0;
}

static void deth_exit_net(struct net* net)
{
    struct deth_net* dn = net_generic(net, deth_net_id);
    netlink_kernel_release(dn->nl_sk);
}

static struct pernet_operations deth_net_ops =
{
    .init = deth_init_net,
    .exit = deth_exit_net,
    .id   = &deth_net_id,
    .size = sizeof(struct deth_net)
};

static int deth_newlink(struct net* src_net, struct net_device* dev, struct nlattr* tb[], struct nlattr* data[])
{
    int err;

    if (tb[IFLA_ADDRESS] == NULL)
    {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)
		eth_hw_addr_random(dev);
#else
        dev_hw_addr_random(dev, dev->dev_addr);
#endif
    }

    if (tb[IFLA_IFNAME])
    {
        nla_strlcpy(dev->name, tb[IFLA_IFNAME], IFNAMSIZ);
    }
    else
    {
        snprintf(dev->name, IFNAMSIZ, DRV_NAME "%%d");
    }

    if (strchr(dev->name, '%'))
    {
        err = dev_alloc_name(dev, dev->name);

        if (err < 0)
        {
            goto exit;
        }
    }

    err = register_netdevice(dev);

    if (err < 0)
    {
        goto exit;
    }

    netif_carrier_off(dev);

    return 0;

exit:
    return err;
}

static void deth_dellink(struct net_device* dev, struct list_head* head)
{
    unregister_netdevice_queue(dev, head);
}

static const struct nla_policy deth_policy[VETH_INFO_MAX + 1] = {
    [VETH_INFO_PEER] = { .len = sizeof(struct ifinfomsg) },
};

static struct rtnl_link_ops deth_link_ops = {
    .kind           = DRV_NAME,
    .priv_size      = sizeof(struct deth_priv),
    .setup          = deth_setup,
    .validate       = deth_validate,
    .newlink        = deth_newlink,
    .dellink        = deth_dellink,
    .policy         = deth_policy,
    .maxtype        = VETH_INFO_MAX,
};

static __init int deth_init(void)
{
    int rc;

    printk(KERN_DEBUG "[Module %s %s loaded]\n", DRV_NAME, DRV_VERSION);

    rc = register_pernet_subsys(&deth_net_ops);

    if (rc != 0)
    {
        return rc;
    }

    rc = rtnl_link_register(&deth_link_ops);

    if (rc != 0)
    {
        unregister_pernet_subsys(&deth_net_ops);
    }

    return rc;
}

static __exit void deth_exit(void)
{
    struct net* net;
    struct deth_net* dn;
    
    for_each_net(net)
    {
        dn = net_generic(net, deth_net_id);
        netlink_kernel_release(dn->nl_sk);
    }

    unregister_pernet_subsys(&deth_net_ops);
    rtnl_link_unregister(&deth_link_ops);
}

module_init(deth_init);
module_exit(deth_exit);
