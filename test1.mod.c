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
	{ 0xd2901226, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0xd8a7a277, __VMLINUX_SYMBOL_STR(usb_deregister) },
	{ 0xfa1499e8, __VMLINUX_SYMBOL_STR(usb_register_driver) },
	{ 0x84cc4934, __VMLINUX_SYMBOL_STR(mutex_unlock) },
	{ 0x7ecf1164, __VMLINUX_SYMBOL_STR(usb_deregister_dev) },
	{ 0x55701cc3, __VMLINUX_SYMBOL_STR(mutex_lock) },
	{ 0xd2b09ce5, __VMLINUX_SYMBOL_STR(__kmalloc) },
	{ 0x2e6443fe, __VMLINUX_SYMBOL_STR(usb_register_dev) },
	{ 0xeb1b3f7a, __VMLINUX_SYMBOL_STR(usb_get_dev) },
	{ 0xc277280a, __VMLINUX_SYMBOL_STR(__mutex_init) },
	{ 0x910e0dc, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0x84f43e8d, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0xdb7305a1, __VMLINUX_SYMBOL_STR(__stack_chk_fail) },
	{ 0x4f8b5ddb, __VMLINUX_SYMBOL_STR(_copy_to_user) },
	{ 0x726a1737, __VMLINUX_SYMBOL_STR(usb_bulk_msg) },
	{ 0x75f68bb7, __VMLINUX_SYMBOL_STR(usb_submit_urb) },
	{ 0x57aee259, __VMLINUX_SYMBOL_STR(usb_free_urb) },
	{ 0x4f6b400b, __VMLINUX_SYMBOL_STR(_copy_from_user) },
	{ 0xfd5443f9, __VMLINUX_SYMBOL_STR(usb_alloc_coherent) },
	{ 0xf6a5c7f4, __VMLINUX_SYMBOL_STR(usb_alloc_urb) },
	{ 0xd5b0ac1, __VMLINUX_SYMBOL_STR(usb_free_coherent) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x16305289, __VMLINUX_SYMBOL_STR(warn_slowpath_null) },
	{ 0xf2d3b221, __VMLINUX_SYMBOL_STR(usb_find_interface) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0xa31c8204, __VMLINUX_SYMBOL_STR(usb_put_dev) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("usb:v1CBEp0003d*dc*dsc*dp*ic*isc*ip*in*");

MODULE_INFO(srcversion, "777529A43A1C400C558FF92");
