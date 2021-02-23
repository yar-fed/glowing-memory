
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
#include <linux/ctype.h>
#include <linux/time.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Yaroslav Fedoriachenko <yar.fed99@gmail.com>");
MODULE_DESCRIPTION("GLKH-BC task07");
MODULE_VERSION("0.1");

#define MODNAME "task07"
#define MODULE_TAG "t7_module "
#define PROC_DIRECTORY "t7"
#define TMP_BUF_SIZE 256

static int t7_read_time_after_read(struct file *file_p, char __user *buffer,
				   size_t length, loff_t *offset);

static struct file_operations proc_time_after_read_fops = {
	.read = t7_read_time_after_read,
};

struct timespec ts;
static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_time_after_read;
static char proc_out_ss_fmt[] = "%lu\n";
static char proc_out_hhmmss_fmt[] = "%0.2lu:%0.2lu:%0.2lu\n";
static bool hhmmss_flag = false;

static int create_proc_t7(void);
static void cleanup_proc_t7(void);

static ssize_t hhmmss_store(struct class *class, struct class_attribute *attr,
			    const char *buf, size_t count);

CLASS_ATTR_WO(hhmmss);
static struct class *t7_class;
static int create_sfs_t7(void);
static void cleanup_sfs_t7(void);

static int t7_prt_time(char *buf, ulong sec);

static int t7_read_time_after_read(struct file *file_p, char __user *buffer,
				   size_t length, loff_t *offset)
{
	static char proc_msg[TMP_BUF_SIZE];
	static bool first_call = true;
	struct timespec new_ts;
	size_t msg_len = 0;
	size_t left;

	if (*offset > 0)
		return 0;

	getnstimeofday(&new_ts);
	if (first_call) {
		msg_len = t7_prt_time(proc_msg, 0);
	} else {
		msg_len = t7_prt_time(proc_msg, new_ts.tv_sec - ts.tv_sec);
	}

	if (length > msg_len)
		length = msg_len;

	left = raw_copy_to_user(buffer, proc_msg, length);
	*offset += length - left;

	if (left) {
		printk(KERN_ERR MODULE_TAG "failed to read %u from %u chars\n",
		       left, length);
	} else {
		printk(KERN_NOTICE MODULE_TAG "read %u chars\n", length);
		first_call = false;
		ts = new_ts;
	}

	return length - left;
}

static int t7_prt_time(char *buf, ulong sec)
{
	if (hhmmss_flag)
		return sprintf(buf, proc_out_hhmmss_fmt, sec / 3600, sec / 60,
			       sec % 60);
	else
		return sprintf(buf, proc_out_ss_fmt, sec);
}

static int create_proc_t7(void)
{
	int err;
	proc_dir = proc_mkdir(PROC_DIRECTORY, NULL);
	if (NULL == proc_dir)
		return -EFAULT;

	proc_time_after_read = proc_create("time_after_read", 0444, proc_dir,
					   &proc_time_after_read_fops);
	if (NULL == proc_time_after_read) {
		pr_err("ERROR create proc entry failed in %s\n", __FUNCTION__);
		err = -EFAULT;
		goto ERROR;
	}

	return 0;
ERROR:
	return err;
}

static void cleanup_proc_t7(void)
{
	if (proc_dir) {
		remove_proc_entry("time_after_read", proc_dir);
		remove_proc_entry(PROC_DIRECTORY, NULL);
		proc_dir = NULL;
	}
}

static ssize_t hhmmss_store(struct class *class, struct class_attribute *attr,
			    const char *buf, size_t count)
{
	char _buff[16];
	strncpy(_buff, buf, sizeof(_buff));

	if (strcmp(_buff, "1") == 0 || strcmp(_buff, "1\n") == 0 ||
	    strcmp(_buff, "true") == 0 || strcmp(_buff, "true\n") == 0) {
		hhmmss_flag = true;
	} else if (strcmp(_buff, "0") == 0 || strcmp(_buff, "0\n") == 0 ||
		   strcmp(_buff, "false") == 0 ||
		   strcmp(_buff, "false\n") == 0) {
		hhmmss_flag = false;
	}

	return count;
}

static int create_sfs_t7(void)
{
	int err;

	t7_class = class_create(THIS_MODULE, "t7");
	err = IS_ERR(t7_class);
	if (err)
		goto error_dir;

	class_attr_hhmmss.attr.mode = 0222;
	err = class_create_file(t7_class, &class_attr_hhmmss);
	if (err)
		goto error_file;

	return 0;

error_file:
	class_destroy(t7_class);
error_dir:
	return err;
}

static void cleanup_sfs_t7(void)
{
	if (t7_class) {
		class_remove_file(t7_class, &class_attr_hhmmss);
		class_destroy(t7_class);
	}
}

static int __init t7_init(void)
{
	int err;
	err = create_sfs_t7();
	if (err)
		goto error_create_sfs;

	err = create_proc_t7();
	if (err)
		goto error_create_proc;

	printk(KERN_INFO MODULE_TAG "loaded\n");
	return err;

error_create_proc:
	cleanup_sfs_t7();
error_create_sfs:
	printk(KERN_ERR MODULE_TAG "failed to load\n");
	return err;
}

static void __exit t7_exit(void)
{
	cleanup_proc_t7();
	cleanup_sfs_t7();
	printk(KERN_INFO "=========== module exited ==============\n");
}

module_init(t7_init);
module_exit(t7_exit);
