
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/ctype.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Yaroslav Fedoriachenko <yar.fed99@gmail.com>");
MODULE_DESCRIPTION("GLKH-BC exercise10");
MODULE_VERSION("0.1");

#define MODULE_TAG "ex10_module "
#define BUFFER_SIZE 256

enum tr_types {
	TR_NONE = 0,
	TR_FLIP,
	TR_UPPER,
};

static int translation_mode = 0;
module_param_named(translation, translation_mode, int, 0660);

static char *translation_buf = "This is a default string";
module_param_named(str, translation_buf, charp, 0660);

static void ex10_flip_words(char *in_string, char *out_str, ssize_t length)
{
	int i, j, k;

	for (i = 0, j = 0; i < length; ++i) {
		if (isalnum(in_string[i])) {
			j = i;
			while (isalnum(in_string[i]))
				++i;
			k = i;
			while (j != i) {
				out_str[j++] = in_string[--k];
			}
		}
		out_str[i] = in_string[i];
	}
}

static void ex10_to_upper(char *in_string, char *out_str, ssize_t length)
{
	int i;

	for (i = 0; i < length; ++i) {
		out_str[i] = toupper(in_string[i]);
	}
}

static int __init ex10_init(void)
{
	int err;
	size_t length;
	char *out_str;

	if (translation_mode < 0 ||
	    (translation_mode > 2 &&
	     !(isdigit(translation_mode) && translation_mode <= '2' &&
	       translation_mode >= '0'))) {
		err = -EINVAL;
		goto error;
	}
	printk(KERN_NOTICE MODULE_TAG "loaded\n");

	length = strlen(translation_buf);
	switch (translation_mode) {
	case TR_FLIP:
		out_str = kmalloc(length + 1, GFP_KERNEL);
		ex10_flip_words(translation_buf, out_str, length);
		break;
	case TR_UPPER:
		out_str = kmalloc(length + 1, GFP_KERNEL);
		ex10_to_upper(translation_buf, out_str, length);
		break;
	case TR_NONE:
	default:
		out_str = translation_buf;
		break;
	}
	out_str[length] = '\0';

	printk(KERN_INFO "%s\n", out_str);

	if (translation_mode & (TR_FLIP | TR_UPPER))
		kfree(out_str);

	return 0;

error:
	printk(KERN_ERR MODULE_TAG "failed to load\n");
	return err;
}

static void __exit ex10_exit(void)
{
	printk(KERN_NOTICE MODULE_TAG "exited\n");
}

module_init(ex10_init);
module_exit(ex10_exit);
