#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xb89a34a1, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x4301027c, __VMLINUX_SYMBOL_STR(ethtool_op_get_link) },
	{ 0xce15e9c2, __VMLINUX_SYMBOL_STR(eth_mac_addr) },
	{ 0xe40dac93, __VMLINUX_SYMBOL_STR(rtnl_link_unregister) },
	{ 0x6d2fc5a6, __VMLINUX_SYMBOL_STR(net_namespace_list) },
	{ 0x49f69261, __VMLINUX_SYMBOL_STR(unregister_pernet_subsys) },
	{ 0x83efa9e9, __VMLINUX_SYMBOL_STR(rtnl_link_register) },
	{ 0x9fec2c4b, __VMLINUX_SYMBOL_STR(register_pernet_subsys) },
	{ 0x1a079195, __VMLINUX_SYMBOL_STR(__netlink_kernel_create) },
	{ 0x2ae86919, __VMLINUX_SYMBOL_STR(netlink_kernel_release) },
	{ 0x5ffd743e, __VMLINUX_SYMBOL_STR(kfree_skb) },
	{ 0x2c7d3623, __VMLINUX_SYMBOL_STR(netlink_broadcast) },
	{ 0x66cfd9cb, __VMLINUX_SYMBOL_STR(__nlmsg_put) },
	{ 0x6e95d3dc, __VMLINUX_SYMBOL_STR(__alloc_skb) },
	{ 0x49c1402f, __VMLINUX_SYMBOL_STR(netlink_has_listeners) },
	{ 0x2a78d108, __VMLINUX_SYMBOL_STR(netif_carrier_on) },
	{ 0x24c1e164, __VMLINUX_SYMBOL_STR(dev_forward_skb) },
	{ 0x69acdf38, __VMLINUX_SYMBOL_STR(memcpy) },
	{ 0xb0acef40, __VMLINUX_SYMBOL_STR(skb_put) },
	{ 0x738e6a65, __VMLINUX_SYMBOL_STR(__netdev_alloc_skb) },
	{ 0x7628f3c7, __VMLINUX_SYMBOL_STR(this_cpu_off) },
	{ 0x8cb2496e, __VMLINUX_SYMBOL_STR(dev_get_by_index) },
	{ 0x949f7342, __VMLINUX_SYMBOL_STR(__alloc_percpu) },
	{ 0xfe7c4287, __VMLINUX_SYMBOL_STR(nr_cpu_ids) },
	{ 0xc0a3d105, __VMLINUX_SYMBOL_STR(find_next_bit) },
	{ 0x3928efe9, __VMLINUX_SYMBOL_STR(__per_cpu_offset) },
	{ 0xb9249d16, __VMLINUX_SYMBOL_STR(cpu_possible_mask) },
	{ 0x5792f848, __VMLINUX_SYMBOL_STR(strlcpy) },
	{ 0x510a4872, __VMLINUX_SYMBOL_STR(free_netdev) },
	{ 0xc9ec4e21, __VMLINUX_SYMBOL_STR(free_percpu) },
	{ 0x7607dce4, __VMLINUX_SYMBOL_STR(ether_setup) },
	{ 0x79aa04a2, __VMLINUX_SYMBOL_STR(get_random_bytes) },
	{ 0x28318305, __VMLINUX_SYMBOL_STR(snprintf) },
	{ 0xf2e441e5, __VMLINUX_SYMBOL_STR(register_netdevice) },
	{ 0xe9de18ca, __VMLINUX_SYMBOL_STR(dev_alloc_name) },
	{ 0x349cba85, __VMLINUX_SYMBOL_STR(strchr) },
	{ 0x6b640864, __VMLINUX_SYMBOL_STR(nla_strlcpy) },
	{ 0xafd09096, __VMLINUX_SYMBOL_STR(netif_carrier_off) },
	{ 0x87bd487b, __VMLINUX_SYMBOL_STR(unregister_netdevice_queue) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "A439D0E026FDBAB2DEB972F");
