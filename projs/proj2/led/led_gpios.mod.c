#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

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
	{ 0xef76ba65, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x31c61f3a, __VMLINUX_SYMBOL_STR(device_destroy) },
	{ 0x24007af4, __VMLINUX_SYMBOL_STR(class_destroy) },
	{ 0x5fc81ac, __VMLINUX_SYMBOL_STR(device_create) },
	{ 0x6bc3fbc0, __VMLINUX_SYMBOL_STR(__unregister_chrdev) },
	{ 0x56b32aa4, __VMLINUX_SYMBOL_STR(__class_create) },
	{ 0xf3b5b415, __VMLINUX_SYMBOL_STR(__register_chrdev) },
	{ 0xfa2a45e, __VMLINUX_SYMBOL_STR(__memzero) },
	{ 0x28cc25db, __VMLINUX_SYMBOL_STR(arm_copy_from_user) },
	{ 0xf4fa543b, __VMLINUX_SYMBOL_STR(arm_copy_to_user) },
	{ 0xbbfb4689, __VMLINUX_SYMBOL_STR(gpiod_get_raw_value) },
	{ 0x95787d36, __VMLINUX_SYMBOL_STR(gpiod_set_raw_value) },
	{ 0xfe990052, __VMLINUX_SYMBOL_STR(gpio_free) },
	{ 0xefd6cf06, __VMLINUX_SYMBOL_STR(__aeabi_unwind_cpp_pr0) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0xb99a7129, __VMLINUX_SYMBOL_STR(gpiod_direction_output_raw) },
	{ 0x480032a5, __VMLINUX_SYMBOL_STR(gpio_to_desc) },
	{ 0x47229b5c, __VMLINUX_SYMBOL_STR(gpio_request) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

