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
	{ 0xbdb09acb, __VMLINUX_SYMBOL_STR(i2c_del_driver) },
	{ 0xc5ed99c1, __VMLINUX_SYMBOL_STR(i2c_register_driver) },
	{ 0x5ee52022, __VMLINUX_SYMBOL_STR(init_timer_key) },
	{ 0xca2c8a68, __VMLINUX_SYMBOL_STR(input_register_device) },
	{ 0x4ee2cdc2, __VMLINUX_SYMBOL_STR(input_set_capability) },
	{ 0xe7d86d87, __VMLINUX_SYMBOL_STR(devm_input_allocate_device) },
	{ 0x1251dd19, __VMLINUX_SYMBOL_STR(dev_err) },
	{ 0x88a8b617, __VMLINUX_SYMBOL_STR(i2c_smbus_read_byte_data) },
	{ 0x9b69eb14, __VMLINUX_SYMBOL_STR(devm_gpio_request_one) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x8dafbf1e, __VMLINUX_SYMBOL_STR(of_get_named_gpio_flags) },
	{ 0xa192af56, __VMLINUX_SYMBOL_STR(dev_alert) },
	{ 0x9d885ac7, __VMLINUX_SYMBOL_STR(devm_kmalloc) },
	{ 0x526c3a6c, __VMLINUX_SYMBOL_STR(jiffies) },
	{ 0x2d3385d3, __VMLINUX_SYMBOL_STR(system_wq) },
	{ 0xa38caae0, __VMLINUX_SYMBOL_STR(mod_timer) },
	{ 0xb2d48a2e, __VMLINUX_SYMBOL_STR(queue_work_on) },
	{ 0xbbfb4689, __VMLINUX_SYMBOL_STR(gpiod_get_raw_value) },
	{ 0x480032a5, __VMLINUX_SYMBOL_STR(gpio_to_desc) },
	{ 0x7a33fe9e, __VMLINUX_SYMBOL_STR(input_event) },
	{ 0x5aaf7aac, __VMLINUX_SYMBOL_STR(i2c_smbus_read_i2c_block_data) },
	{ 0xefd6cf06, __VMLINUX_SYMBOL_STR(__aeabi_unwind_cpp_pr0) },
	{ 0x80920bb5, __VMLINUX_SYMBOL_STR(input_unregister_device) },
	{ 0x4205ad24, __VMLINUX_SYMBOL_STR(cancel_work_sync) },
	{ 0xfc982daa, __VMLINUX_SYMBOL_STR(del_timer_sync) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("of:N*T*Cinv,mpu6050");
MODULE_ALIAS("of:N*T*Cinv,mpu6050C*");
