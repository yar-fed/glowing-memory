
#include <linux/cdev.h>
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
MODULE_DESCRIPTION("GLKH-BC task06");
MODULE_VERSION("1.0");

#define MODNAME "task06"
#define MODULE_TAG "t6_module "
#define PROC_DIRECTORY "t6"
#define BUFFER_SIZE 1024
#define TMP_BUF_SIZE 256

#define EOK 0
#define DEVICE_FIRST 0
#define DEVICE_COUNT 1

static char *raw_buf;
static ssize_t raw_buf_size = BUFFER_SIZE;
static ssize_t raw_buf_offset = 0;
module_param(raw_buf_size, int, S_IRUGO);

static char current_user[8] = "monkey1";

LIST_HEAD(msgs_head);
struct msg_list_t {
	struct msg_t {
		char *msg;
		char *to;
		char from[8];
	} item;
	struct list_head meta;
};

static struct cdev hcdev;
static int device_open = 0;

static int major = 0;
module_param(major, int, S_IRUGO);

static int t6_read_usage(struct file *file_p, char __user *buffer,
			 size_t length, loff_t *offset);
static struct file_operations proc_usage_fops = {
	.read = t6_read_usage,
};

static int t6_read_user(struct file *file_p, char __user *buffer, size_t length,
			loff_t *offset);
static int t6_write_user(struct file *file_p, const char __user *buffer,
			 size_t length, loff_t *offset);
static struct file_operations proc_user_fops = {
	.read = t6_read_user,
	.write = t6_write_user,
};

static size_t proc_msg_read_pos;
static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_user;
static struct proc_dir_entry *proc_usage;

static ssize_t clear_store(struct class *class, struct class_attribute *attr,
			   const char *buf, size_t count);
CLASS_ATTR_WO(clear);
static struct class *t6_class;
static struct device *t6_device;

static void cleanup_heap(void);
static void t6_clear_buff(void);
static int create_dev_t6(void);
static void cleanup_dev_t6(void);
static int create_proc_t6(void);
static void cleanup_proc_t6(void);
static int create_sfs_t6(void);
static void cleanup_sfs_t6(void);

static ssize_t clear_store(struct class *class, struct class_attribute *attr,
			   const char *buf, size_t count)
{
	char _buff[4];
	strncpy(_buff, buf, sizeof(_buff));
	_buff[3] = '\0';

	if (strcmp(_buff, "1") == 0 || strcmp(_buff, "1\n") == 0) {
		t6_clear_buff();
		cleanup_heap();
	}
	return count;
}

static int create_sfs_t6(void)
{
	int err;

	t6_class = class_create(THIS_MODULE, "t6");
	err = IS_ERR(t6_class);
	if (err)
		goto error_dir;

	err = class_create_file(t6_class, &class_attr_clear);
	if (err)
		goto error_file;

	return 0;

error_file:
	class_destroy(t6_class);
error_dir:
	return err;
}

static void cleanup_sfs_t6(void)
{
	if (t6_class) {
		class_remove_file(t6_class, &class_attr_clear);
		class_destroy(t6_class);
	}
}

static int t6_read_usage(struct file *file_p, char __user *buffer,
			 size_t length, loff_t *offset)
{
	static char proc_msg[TMP_BUF_SIZE];
	size_t msg_len = 0;
	size_t left;
	struct list_head *pos = NULL;
	size_t msgs_n = 0;

	if (*offset > 0)
		return 0;

	list_for_each (pos, &msgs_head)
		++msgs_n;

	msg_len = sprintf(proc_msg, "used: %u\n size: %u\n msgs_n: %u\n",
			  raw_buf_offset, raw_buf_size, msgs_n);
	if (length > msg_len)
		length = msg_len;

	left = raw_copy_to_user(buffer, proc_msg, length);
	offset += length - left;

	if (left) {
		printk(KERN_ERR MODULE_TAG "failed to read %u from %u chars\n",
		       left, length);
	} else {
		printk(KERN_NOTICE MODULE_TAG "read %u chars\n", length);
	}

	return length - left;
}

static int t6_read_user(struct file *file_p, char __user *buffer, size_t length,
			loff_t *offset)
{
	static char proc_msg[TMP_BUF_SIZE];
	size_t left;
	size_t msg_len = 0;

	if (*offset > 0)
		return 0;

	msg_len = snprintf(proc_msg, TMP_BUF_SIZE, "%s\n", current_user);
	if (length > msg_len)
		length = msg_len;

	left = raw_copy_to_user(buffer, proc_msg, length);
	offset += length - left;

	if (left) {
		printk(KERN_ERR MODULE_TAG "failed to read %u from %u chars\n",
		       left, length);
	} else {
		printk(KERN_NOTICE MODULE_TAG "read %u chars\n", length);
	}

	return length - left;
}

static int t6_write_user(struct file *file_p, const char __user *buffer,
			 size_t length, loff_t *offset)
{
	char _buff[TMP_BUF_SIZE];
	size_t msg_length = 0;
	size_t left;

	if (length > TMP_BUF_SIZE) {
		printk(KERN_WARNING MODULE_TAG
		       ":%s: reduse message length from %u to %u chars\n",
		       __FUNCTION__, length, TMP_BUF_SIZE);
		msg_length = TMP_BUF_SIZE;
	} else {
		msg_length = length;
	}

	left = raw_copy_from_user(_buff + *offset, buffer, msg_length);
	_buff[msg_length] = '\0';

	if (left) {
		printk(KERN_ERR MODULE_TAG "failed to write %u from %u chars\n",
		       left, msg_length);
	} else {
		printk(KERN_NOTICE MODULE_TAG ":%s: written %u chars\n",
		       __FUNCTION__, msg_length);
		strncpy(current_user, _buff, 7);
	}

	offset += msg_length - left;

	return msg_length;
}

static int create_proc_t6(void)
{
	int err;
	proc_dir = proc_mkdir(PROC_DIRECTORY, NULL);
	if (NULL == proc_dir)
		return -EFAULT;

	proc_user = proc_create("user", 0666, proc_dir, &proc_user_fops);
	if (NULL == proc_user) {
		pr_err("ERROR create proc entry failed in %s\n", __FUNCTION__);
		err = -EFAULT;
		goto ERROR;
	}

	proc_usage = proc_create("usage", 0444, proc_dir, &proc_usage_fops);
	if (NULL == proc_usage) {
		pr_err("ERROR create proc entry failed in %s\n", __FUNCTION__);
		err = -EFAULT;
		goto ERROR;
	}
	proc_msg_read_pos = 0;

	return 0;
ERROR:
	return err;
}

static void cleanup_proc_t6(void)
{
	if (proc_dir) {
		remove_proc_entry("user", proc_dir);
		remove_proc_entry("usage", proc_dir);
		remove_proc_entry(PROC_DIRECTORY, NULL);
		proc_dir = NULL;
	}
}

static void cleanup_heap(void)
{
	struct list_head *pos;
	struct list_head *n;
	struct msg_list_t *msg;

	list_for_each_safe (pos, n, &msgs_head) {
		msg = list_entry(pos, struct msg_list_t, meta);
		list_del(pos);
		kfree(msg);
	}
	kfree(raw_buf);
}

static void t6_clear_buff(void)
{
	struct list_head *pos;
	struct list_head *n;
	struct msg_list_t *msg;

	list_for_each_safe (pos, n, &msgs_head) {
		msg = list_entry(pos, struct msg_list_t, meta);
		list_del(pos);
		kfree(msg);
	}
	memset(raw_buf, 0, raw_buf_size);
	raw_buf_offset = 0;
}

static int dev_open(struct inode *n, struct file *f)
{
	if (device_open) {
		pr_err("%s\n", __FUNCTION__);
		return -EBUSY;
	}
	device_open++;
	return EOK;
}

static int dev_release(struct inode *n, struct file *f)
{
	device_open--;
	return EOK;
}

static ssize_t dev_read(struct file *file, char *buf, size_t count,
			loff_t *offset)
{
	int len = 0;
	int len_from, len_msg;
	char _buff[TMP_BUF_SIZE];
	struct list_head *n, *pos;
	struct msg_list_t *msg;

	if (*offset != 0)
		return 0;

	list_for_each_safe (pos, n, &msgs_head) {
		msg = list_entry(pos, struct msg_list_t, meta);
		if (strcmp(msg->item.to, current_user) == 0) {
			pr_notice("%s:%s: found a message from %s\n",
				  MODULE_TAG, __FUNCTION__, msg->item.from);

			len_from = snprintf(_buff, TMP_BUF_SIZE,
					    "\n>from %s:\n  ", msg->item.from);
			len_msg = strlen(msg->item.msg);
			len += len_from + len_msg;
			if (count < len_from + len_msg)
				return -EFAULT;

			if (raw_copy_to_user(buf + *offset, _buff, len_from))
				return -EFAULT;

			if (raw_copy_to_user(buf + *offset + len_from,
					     msg->item.msg, len_msg))
				return -EFAULT;

			*offset += len_from + len_msg;
			list_del(pos);
			kfree(msg);
		}
	}
	return len;
}

static ssize_t dev_write(struct file *file, const char __user *user_buffer,
			 size_t size, loff_t *offset)
{
	int err;
	ssize_t len = min(raw_buf_size - raw_buf_offset, size + 1);
	struct msg_list_t *_new_msg;

	if (len <= 0 || *offset > 0)
		return 0;

	if (raw_copy_from_user(raw_buf + raw_buf_offset, user_buffer, len)) {
		printk(KERN_ERR MODULE_TAG ":%s: copy_from_user failed",
		       __FUNCTION__);
		err = -EFAULT;
		goto error;
	}
	*(raw_buf + raw_buf_offset + len - 1) = '\0';

	_new_msg = kmalloc(sizeof(*_new_msg), GFP_KERNEL);
	if (NULL == _new_msg) {
		err = -ENOMEM;
		goto error;
	}

	_new_msg->item.msg = strchr(raw_buf + raw_buf_offset, ':');
	if (NULL == _new_msg->item.msg) {
		err = -EINVAL;
		goto err_wrong_format;
	}
	*(_new_msg->item.msg) = '\0';
	_new_msg->item.msg += 1;

	_new_msg->item.to = raw_buf + raw_buf_offset;
	strcpy(_new_msg->item.from, current_user);
	INIT_LIST_HEAD(&_new_msg->meta);
	list_add_tail(&_new_msg->meta, &msgs_head);

	pr_notice("%s:%s: %10s -> %10s: %5s\n", MODULE_TAG, __FUNCTION__,
		  _new_msg->item.from, _new_msg->item.to, _new_msg->item.msg);

	raw_buf_offset += len;
	*offset = len;

	pr_notice("%s:%s: written %d bytes\n", MODULE_TAG, __FUNCTION__, len);
	return len;

err_wrong_format:
	kfree(_new_msg);
error:
	return err;
}

static const struct file_operations dev_fops = {
	.owner = THIS_MODULE,
	.open = dev_open,
	.release = dev_release,
	.read = dev_read,
	.write = dev_write,
};

static int create_dev_t6(void)
{
	int err;
	dev_t dev;
	if (major != 0) {
		dev = MKDEV(major, DEVICE_FIRST);
		err = register_chrdev_region(dev, DEVICE_COUNT, MODNAME);
	} else {
		err = alloc_chrdev_region(&dev, DEVICE_FIRST, DEVICE_COUNT,
					  MODNAME);
		major = MAJOR(dev);
	}

	if (err < 0) {
		printk(KERN_ERR "=== Can not register char device region\n");
		goto error_region_alloc;
	}
	err = cdev_add(&hcdev, dev, DEVICE_COUNT);
	if (err < 0) {
		printk(KERN_ERR "=== Can not add char device\n");
		goto error_cdev_add;
	}
	cdev_init(&hcdev, &dev_fops);

	hcdev.owner = THIS_MODULE;

	t6_device = device_create(t6_class, NULL, dev, NULL, "t6");
	err = IS_ERR(t6_device);
	if (err) {
		printk(KERN_ERR "=== Can not create device class\n");
		goto error_class;
	}

	printk(KERN_INFO MODULE_TAG "device %d:%d is used\n", MAJOR(dev),
	       MINOR(dev));
	return 0;

error_class:
	cdev_del(&hcdev);
error_cdev_add:
	unregister_chrdev_region(MKDEV(major, DEVICE_FIRST), DEVICE_COUNT);
error_region_alloc:
	return err;
}

static void cleanup_dev_t6(void)
{
	dev_t dev;
	dev = MKDEV(major, DEVICE_FIRST);
	device_destroy(t6_class, dev);
	cdev_del(&hcdev);
	unregister_chrdev_region(MKDEV(major, DEVICE_FIRST), DEVICE_COUNT);
}

static int __init t6_init(void)
{
	int err;
	err = create_sfs_t6();
	if (err)
		goto error_create_sfs;

	err = create_dev_t6();
	if (err)
		goto error_create_dev;

	err = create_proc_t6();
	if (err)
		goto error_create_proc;

	raw_buf = (char *)kmalloc(raw_buf_size, GFP_KERNEL);
	if (NULL == raw_buf) {
		err = -ENOMEM;
		goto error_nomem;
	}
	printk(KERN_INFO MODULE_TAG "loaded\n");
	return err;

error_nomem:
	cleanup_proc_t6();
error_create_proc:
	cleanup_dev_t6();
error_create_dev:
	cleanup_sfs_t6();
error_create_sfs:
	printk(KERN_ERR MODULE_TAG "failed to load\n");
	return err;
}

static void __exit t6_exit(void)
{
	cleanup_heap();
	cleanup_proc_t6();
	cleanup_dev_t6();
	cleanup_sfs_t6();
	printk(KERN_INFO "=========== module exited ==============\n");
}

module_init(t6_init);
module_exit(t6_exit);
