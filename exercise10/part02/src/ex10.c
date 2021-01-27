
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
#define PROC_DIRECTORY "ex10"
#define PROC_FILENAME "buffer"
#define BUFFER_SIZE 128

enum tr_types {
	TR_NONE = 0,
	TR_FLIP,
	TR_UPPER,
};

static char *proc_buffer;
static size_t proc_msg_length;
static size_t proc_msg_read_pos;

static uint translation_mode;

static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_file;

static int ex10_read(struct file *file_p, char __user *buffer, size_t length,
		     loff_t *offset);
static int ex10_write(struct file *file_p, const char __user *buffer,
		      size_t length, loff_t *offset);

static ssize_t translation_show(struct class *class,
				struct class_attribute *attr, char *buf)
{
	char reply[8];
	sprintf(reply, "%hhu\n", (unsigned char)translation_mode);
	strcpy(buf, reply);
	printk("read %ld\n", (long)strlen(buf));
	return strlen(buf);
}

static ssize_t translation_store(struct class *class,
				 struct class_attribute *attr, const char *buf,
				 size_t count)
{
	char _buff;
	strncpy(&_buff, buf, sizeof(_buff));
	if (translation_mode)
		printk("write %hhx\n", _buff);
	if (_buff < 0 ||
	    (_buff > 2 && !(isdigit(_buff) && _buff <= '2' && _buff >= '0'))) {
		pr_err("write error, no such translation '%hhx'", _buff);
		return -EIO;
	}

	if (isdigit(_buff))
		translation_mode = _buff - '0';
	else
		translation_mode = _buff;

	return count;
}

CLASS_ATTR_RW(translation);

static struct class *ex10_class;

static int create_sfs_ex10(void)
{
	int err;

	ex10_class = class_create(THIS_MODULE, "ex10");
	err = IS_ERR(ex10_class);
	if (err)
		return err;
	err = class_create_file(ex10_class, &class_attr_translation);
	return err;
}

static void cleanup_sfs_ex10(void)
{
	class_remove_file(ex10_class, &class_attr_translation);
	class_destroy(ex10_class);
	return;
}

static struct file_operations proc_fops = {
	.read = ex10_read,
	.write = ex10_write,
};

static int create_buffer(void)
{
	proc_buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);
	if (NULL == proc_buffer)
		return -ENOMEM;
	proc_msg_length = 0;

	return 0;
}

static void cleanup_buffer(void)
{
	if (proc_buffer) {
		kfree(proc_buffer);
		proc_buffer = NULL;
	}
	proc_msg_length = 0;
}

static int create_proc_ex10(void)
{
	proc_dir = proc_mkdir(PROC_DIRECTORY, NULL);
	if (NULL == proc_dir)
		return -EFAULT;

	proc_file = proc_create(PROC_FILENAME, S_IFREG | S_IRUGO | S_IWUGO,
				proc_dir, &proc_fops);
	if (NULL == proc_file)
		return -EFAULT;

	return 0;
}

static void cleanup_proc_ex10(void)
{
	if (proc_file) {
		remove_proc_entry(PROC_FILENAME, proc_dir);
		proc_file = NULL;
	}
	if (proc_dir) {
		remove_proc_entry(PROC_DIRECTORY, NULL);
		proc_dir = NULL;
	}
}

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

static int ex10_read(struct file *file_p, char __user *buffer, size_t length,
		     loff_t *offset)
{
	size_t left;
	char *out_str;

	if (length > (proc_msg_length - proc_msg_read_pos))
		length = (proc_msg_length - proc_msg_read_pos);

	switch (translation_mode) {
	case TR_FLIP:
		out_str = kmalloc(length, GFP_KERNEL);
		ex10_flip_words(&proc_buffer[proc_msg_read_pos], out_str,
				length);
		break;
	case TR_UPPER:
		out_str = kmalloc(length, GFP_KERNEL);
		ex10_to_upper(&proc_buffer[proc_msg_read_pos], out_str, length);
		break;
	case TR_NONE:
	default:
		out_str = &proc_buffer[proc_msg_read_pos];
		break;
	}
	left = raw_copy_to_user(buffer, out_str, length);
	if (translation_mode & (TR_FLIP | TR_UPPER))
		kfree(out_str);

	proc_msg_read_pos += length - left;

	if (left)
		printk(KERN_ERR MODULE_TAG "failed to read %u from %u chars\n",
		       left, length);
	else
		printk(KERN_NOTICE MODULE_TAG "read %u chars\n", length);

	return length - left;
}

static int ex10_write(struct file *file_p, const char __user *buffer,
		      size_t length, loff_t *offset)
{
	size_t msg_length;
	size_t left;

	if (length > BUFFER_SIZE) {
		printk(KERN_WARNING MODULE_TAG
		       "reduse message length from %u to %u chars\n",
		       length, BUFFER_SIZE);
		msg_length = BUFFER_SIZE;
	} else {
		msg_length = length;
	}

	left = raw_copy_from_user(proc_buffer, buffer, msg_length);

	proc_msg_length = msg_length - left;
	proc_msg_read_pos = 0;

	if (left)
		printk(KERN_ERR MODULE_TAG "failed to write %u from %u chars\n",
		       left, msg_length);
	else
		printk(KERN_NOTICE MODULE_TAG "written %u chars\n", msg_length);

	return length;
}

static int __init ex10_init(void)
{
	int err;

	err = create_sfs_ex10();
	if (err)
		goto error;

	err = create_buffer();
	if (err)
		goto error;

	err = create_proc_ex10();
	if (err)
		goto error;

	printk(KERN_NOTICE MODULE_TAG "loaded\n");
	return 0;

error:
	printk(KERN_ERR MODULE_TAG "failed to load\n");
	cleanup_proc_ex10();
	cleanup_buffer();
	cleanup_sfs_ex10();
	return err;
}

static void __exit ex10_exit(void)
{
	cleanup_proc_ex10();
	cleanup_buffer();
	cleanup_sfs_ex10();
	printk(KERN_NOTICE MODULE_TAG "exited\n");
}

module_init(ex10_init);
module_exit(ex10_exit);
