
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
#include <linux/time.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Yaroslav Fedoriachenko <yar.fed99@gmail.com>");
MODULE_DESCRIPTION("GLKH-BC task08");
MODULE_VERSION("1.0");

#define MODNAME "task08"
#define MODULE_TAG "t8_module "
#define PROC_DIRECTORY "t8"
#define BUFFER_SIZE 1024
#define TMP_BUF_SIZE 256

#define EOK 0
#define DEVICE_FIRST 0
#define DEVICE_COUNT 1

static struct timer_list t8_q_timer;
static char *raw_buf;
static ssize_t raw_buf_size = BUFFER_SIZE;
static ssize_t raw_buf_offset = 0;
module_param(raw_buf_size, int, S_IRUGO);

static char current_user[8] = "monkey1";

LIST_HEAD(rmnds_head);
struct rmnd_list_t {
	struct rmnd_t {
		char *rmnd;
		char *recipient;
		u64 delay;
		bool ready;
	} item;
	struct list_head meta;
};

LIST_HEAD(ready_rmnds_head);
struct ready_rmnds_q_t {
	struct rmnd_list_t *rmnd;
	struct list_head meta;
};

static struct cdev hcdev;
static int device_open = 0;

static int major = 0;
module_param(major, int, S_IRUGO);

//static int t8_read_usage(struct file *file_p, char __user *buffer,
//			 size_t length, loff_t *offset);
//static struct file_operations proc_usage_fops = {
//	.read = t8_read_usage,
//};

static int t8_read_user(struct file *file_p, char __user *buffer, size_t length,
			loff_t *offset);
static int t8_write_user(struct file *file_p, const char __user *buffer,
			 size_t length, loff_t *offset);
static struct file_operations proc_user_fops = {
	.read = t8_read_user,
	.write = t8_write_user,
};

static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_user;
//static struct proc_dir_entry *proc_usage;

static ssize_t clear_store(struct class *class, struct class_attribute *attr,
			   const char *buf, size_t count);
CLASS_ATTR_WO(clear);
static struct class *t8_class;
static struct device *t8_device;

static void cleanup_heap(void);
static void t8_clear_buff(void);
static int create_dev_t8(void);
static void cleanup_dev_t8(void);
static int create_proc_t8(void);
static void cleanup_proc_t8(void);
static int create_sfs_t8(void);
static void cleanup_sfs_t8(void);

static ssize_t clear_store(struct class *class, struct class_attribute *attr,
			   const char *buf, size_t count)
{
	char _buff[4];
	strncpy(_buff, buf, sizeof(_buff));
	_buff[3] = '\0';

	if (strcmp(_buff, "1") == 0 || strcmp(_buff, "1\n") == 0) {
		t8_clear_buff();
		cleanup_heap();
	}
	return count;
}

static int create_sfs_t8(void)
{
	int err;

	t8_class = class_create(THIS_MODULE, "t8");
	err = IS_ERR(t8_class);
	if (err)
		goto error_dir;

	err = class_create_file(t8_class, &class_attr_clear);
	if (err)
		goto error_file;

	return 0;

error_file:
	class_destroy(t8_class);
error_dir:
	return err;
}

static void cleanup_sfs_t8(void)
{
	if (t8_class) {
		class_remove_file(t8_class, &class_attr_clear);
		class_destroy(t8_class);
	}
}

/*
static int t8_read_usage(struct file *file_p, char __user *buffer,
			 size_t length, loff_t *offset)
{
	static char proc_rmnd[TMP_BUF_SIZE];
	size_t rmnd_len = 0;
	size_t left;
	struct list_head *pos = NULL;
	size_t rmnds_n = 0;

	if (*offset > 0)
		return 0;

	list_for_each (pos, &rmnds_head)
		++rmnds_n;

	rmnd_len = sprintf(proc_rmnd, "used: %u\n size: %u\n rmnds_n: %u\n",
			  raw_buf_offset, raw_buf_size, rmnds_n);
	if (length > rmnd_len)
		length = rmnd_len;

	left = raw_copy_to_user(buffer, proc_rmnd, length);
	offset += length - left;

	if (left) {
		printk(KERN_ERR MODULE_TAG "failed to read %u from %u chars\n",
		       left, length);
	} else {
		printk(KERN_NOTICE MODULE_TAG "read %u chars\n", length);
	}

	return length - left;
}
*/

static int t8_read_user(struct file *file_p, char __user *buffer, size_t length,
			loff_t *offset)
{
	static char proc_rmnd[TMP_BUF_SIZE];
	size_t left;
	size_t rmnd_len = 0;

	if (*offset > 0)
		return 0;

	rmnd_len = snprintf(proc_rmnd, TMP_BUF_SIZE, "%s\n", current_user);
	if (length > rmnd_len)
		length = rmnd_len;

	left = raw_copy_to_user(buffer, proc_rmnd, length);
	*offset += length - left;

	if (left) {
		printk(KERN_ERR MODULE_TAG "failed to read %u from %u chars\n",
		       left, length);
	} else {
		printk(KERN_NOTICE MODULE_TAG "read %u chars\n", length);
	}

	return length - left;
}

static int t8_write_user(struct file *file_p, const char __user *buffer,
			 size_t length, loff_t *offset)
{
	char _buff[TMP_BUF_SIZE];
	size_t rmnd_length = 0;
	size_t left;

	if (length > TMP_BUF_SIZE) {
		printk(KERN_WARNING MODULE_TAG
		       ":%s: reduse message length from %u to %u chars\n",
		       __FUNCTION__, length, TMP_BUF_SIZE);
		rmnd_length = TMP_BUF_SIZE;
	} else {
		rmnd_length = length;
	}

	left = raw_copy_from_user(_buff + *offset, buffer, rmnd_length);
	_buff[rmnd_length] = '\0';

	if (left) {
		printk(KERN_ERR MODULE_TAG "failed to write %u from %u chars\n",
		       left, rmnd_length);
	} else {
		printk(KERN_NOTICE MODULE_TAG ":%s: written %u chars\n",
		       __FUNCTION__, rmnd_length);
		strncpy(current_user, _buff, 7);
	}

	offset += rmnd_length - left;

	return rmnd_length;
}

static int create_proc_t8(void)
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

	return 0;
ERROR:
	return err;
}

static void cleanup_proc_t8(void)
{
	if (proc_dir) {
		remove_proc_entry("user", proc_dir);
		remove_proc_entry(PROC_DIRECTORY, NULL);
		proc_dir = NULL;
	}
}

static void cleanup_heap(void)
{
	struct list_head *pos;
	struct list_head *n;
	struct rmnd_list_t *rmnd;
	struct ready_rmnds_q_t *r_rmnd;

	list_for_each_safe (pos, n, &ready_rmnds_head) {
		r_rmnd = list_entry(pos, struct ready_rmnds_q_t, meta);
		list_del(pos);
		kfree(r_rmnd);
	}

	list_for_each_safe (pos, n, &rmnds_head) {
		rmnd = list_entry(pos, struct rmnd_list_t, meta);
		list_del(pos);
		kfree(rmnd);
	}
	kfree(raw_buf);
}

static void t8_clear_buff(void)
{
	struct list_head *pos;
	struct list_head *n;
	struct rmnd_list_t *rmnd;
	struct ready_rmnds_q_t *r_rmnd;

	list_for_each_safe (pos, n, &ready_rmnds_head) {
		r_rmnd = list_entry(pos, struct ready_rmnds_q_t, meta);
		list_del(pos);
		kfree(r_rmnd);
	}

	list_for_each_safe (pos, n, &rmnds_head) {
		rmnd = list_entry(pos, struct rmnd_list_t, meta);
		list_del(pos);
		kfree(rmnd);
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
	int len_for, len_rmnd;
	char _buff[TMP_BUF_SIZE];
	struct list_head *n, *pos;
	struct ready_rmnds_q_t *r_rmnd;

	if (*offset != 0)
		return 0;

	list_for_each_safe (pos, n, &ready_rmnds_head) {
		r_rmnd = list_entry(pos, struct ready_rmnds_q_t, meta);
		if (strcmp(r_rmnd->rmnd->item.recipient, current_user) == 0) {
			pr_notice("%s:%s: found a reminder for %s\n",
				  MODULE_TAG, __FUNCTION__, r_rmnd->rmnd->item.recipient);

			len_for = snprintf(_buff, TMP_BUF_SIZE,
					    "\n>for %s:\n  ", r_rmnd->rmnd->item.recipient);
			len_rmnd = strlen(r_rmnd->rmnd->item.rmnd);
			len += len_for + len_rmnd;
			if (count < len_for + len_rmnd)
				return -EFAULT;

			if (raw_copy_to_user(buf + *offset, _buff, len_for))
				return -EFAULT;

			if (raw_copy_to_user(buf + *offset + len_for,
					     r_rmnd->rmnd->item.rmnd, len_rmnd))
				return -EFAULT;

			*offset += len_for + len_rmnd;
			list_del(pos);
			kfree(r_rmnd);
		} else {
			pr_notice("%s:%s: %s not a reminder for %s\n", 
				  MODULE_TAG, __FUNCTION__, r_rmnd->rmnd->item.rmnd,
				  r_rmnd->rmnd->item.recipient);
		}
	}
	return len;
}

static ssize_t dev_write(struct file *file, const char __user *user_buffer,
			 size_t size, loff_t *offset)
{
	int err;
	ssize_t len = min(raw_buf_size - raw_buf_offset, size + 1);
	struct rmnd_list_t *_new_rmnd;
	char *_c_delay;

	if (len <= 0 || *offset > 0)
		return 0;

	if (raw_copy_from_user(raw_buf + raw_buf_offset, user_buffer, len)) {
		printk(KERN_ERR MODULE_TAG ":%s: copy_from_user failed",
		       __FUNCTION__);
		err = -EFAULT;
		goto error;
	}
	*(raw_buf + raw_buf_offset + len - 1) = '\0';

	_new_rmnd = kmalloc(sizeof(*_new_rmnd), GFP_KERNEL);
	if (NULL == _new_rmnd) {
		err = -ENOMEM;
		goto error;
	}

	_new_rmnd->item.rmnd = strchr(raw_buf + raw_buf_offset, ':');
	if (NULL == _new_rmnd->item.rmnd) {
		err = -EINVAL;
		goto err_wrong_format;
	}
	*(_new_rmnd->item.rmnd) = '\0';
	_new_rmnd->item.rmnd += 1;
	_c_delay = strchr(_new_rmnd->item.rmnd, ':');
	if (NULL == _c_delay || _c_delay == _new_rmnd->item.rmnd) {
		err = -EINVAL;
		goto err_wrong_format;
	}
	if (sscanf(_c_delay, ":%llu\n", &_new_rmnd->item.delay) != 1) {
		err = -EINVAL;
		goto err_wrong_format;
	}
	*_c_delay = '\0';

	_new_rmnd->item.recipient = raw_buf + raw_buf_offset;
	_new_rmnd->item.ready = false;
	INIT_LIST_HEAD(&_new_rmnd->meta);
	list_add_tail(&_new_rmnd->meta, &rmnds_head);

	pr_notice("%s:%s: in %llu -> %10s:%5s\n", MODULE_TAG, __FUNCTION__,
		  _new_rmnd->item.delay, _new_rmnd->item.recipient, _new_rmnd->item.rmnd);

	raw_buf_offset += len;
	*offset = len;

	pr_notice("%s:%s: written %d bytes\n", MODULE_TAG, __FUNCTION__, len);
	return len;

err_wrong_format:
	kfree(_new_rmnd);
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

static int create_dev_t8(void)
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

	t8_device = device_create(t8_class, NULL, dev, NULL, "t8");
	err = IS_ERR(t8_device);
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

static void cleanup_dev_t8(void)
{
	dev_t dev;
	dev = MKDEV(major, DEVICE_FIRST);
	device_destroy(t8_class, dev);
	cdev_del(&hcdev);
	unregister_chrdev_region(MKDEV(major, DEVICE_FIRST), DEVICE_COUNT);
}

static void t8_q_timer_callback(struct timer_list * data)
{
	struct list_head *pos;
	struct rmnd_list_t *rmnd;
	pr_notice("%s:%s: \n", MODULE_TAG, __FUNCTION__);

	list_for_each(pos, &rmnds_head) {
		rmnd = list_entry(pos, struct rmnd_list_t, meta);
		pr_notice("%s:%s: decremnt %llu %.40s\n",
			  MODULE_TAG, __FUNCTION__,  rmnd->item.delay, rmnd->item.rmnd);
		if (rmnd->item.delay) {
			pr_notice("Not yet");
			--(rmnd->item.delay);
		} else if (!rmnd->item.ready) {
			struct ready_rmnds_q_t *_ready;
			_ready = kmalloc(sizeof(*_ready), GFP_KERNEL);
			if (NULL == _ready)
				break;
			_ready->rmnd = rmnd;
			INIT_LIST_HEAD(&_ready->meta);
			list_add_tail(&_ready->meta, &ready_rmnds_head);
			rmnd->item.ready = true;
			pr_notice("Added to q");
		} else {
			pr_err("??? %d", rmnd->item.ready);
		}
	}
	mod_timer(&t8_q_timer, jiffies + HZ);
}

static int __init t8_init(void)
{
	int err;
	timer_setup(&t8_q_timer, t8_q_timer_callback, 0);
	err = create_sfs_t8();
	if (err)
		goto error_create_sfs;

	err = create_dev_t8();
	if (err)
		goto error_create_dev;

	err = create_proc_t8();
	if (err)
		goto error_create_proc;

	raw_buf = (char *)kmalloc(raw_buf_size, GFP_KERNEL);
	if (NULL == raw_buf) {
		err = -ENOMEM;
		goto error_nomem;
	}

	mod_timer(&t8_q_timer, jiffies + HZ);
	printk(KERN_INFO MODULE_TAG "loaded\n");
	return err;

error_nomem:
	cleanup_proc_t8();
error_create_proc:
	cleanup_dev_t8();
error_create_dev:
	cleanup_sfs_t8();
error_create_sfs:
	del_timer(&t8_q_timer);
	printk(KERN_ERR MODULE_TAG "failed to load\n");
	return err;
}

static void __exit t8_exit(void)
{
	cleanup_heap();
	cleanup_proc_t8();
	cleanup_dev_t8();
	cleanup_sfs_t8();
	del_timer(&t8_q_timer);
	printk(KERN_INFO "=========== module exited ==============\n");
}

module_init(t8_init);
module_exit(t8_exit);
