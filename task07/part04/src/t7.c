
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
#include <linux/rtc.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Yaroslav Fedoriachenko <yar.fed99@gmail.com>");
MODULE_DESCRIPTION("GLKH-BC task07");
MODULE_VERSION("0.1");

#define SEC_IN_DAY 3600 * 24
#define MODNAME "task07"
#define MODULE_TAG "t7_module "
#define PROC_DIRECTORY "t7"
#define TMP_BUF_SIZE 256

#define PROC_OUT_SS_FMT "%lu\n"
#define PROC_OUT_HHMMSS_FMT "%0.2lu:%0.2lu:%0.2lu\n"

static int t7_read_abs_time(struct file *file_p, char __user *buffer,
			    size_t length, loff_t *offset);

static struct file_operations proc_abs_time_fops = {
	.read = t7_read_abs_time,
};

static int t7_read_time_after_read(struct file *file_p, char __user *buffer,
				   size_t length, loff_t *offset);

static struct file_operations proc_time_after_read_fops = {
	.read = t7_read_time_after_read,
};

static long t7_time = 0;
struct timespec ts;
static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_time_after_read;
static struct proc_dir_entry *proc_abs_time;
static bool hhmmss_flag = false;

static int create_proc_t7(void);
static void cleanup_proc_t7(void);

static ssize_t set_time_store(struct class *class, struct class_attribute *attr,
			      const char *buf, size_t count);

static ssize_t hhmmss_store(struct class *class, struct class_attribute *attr,
			    const char *buf, size_t count);

CLASS_ATTR_WO(hhmmss);
CLASS_ATTR_WO(set_time);
static struct class *t7_class;
static int create_sfs_t7(void);
static void cleanup_sfs_t7(void);

static int t7_prt_time(char *buf, ulong sec);

static int t7_read_abs_time(struct file *file_p, char __user *buffer,
			    size_t length, loff_t *offset)
{
	static char proc_msg[TMP_BUF_SIZE];
	struct timespec current_ts;
	struct rtc_time lc_tm;
	size_t msg_len = 0;
	size_t left;

	if (*offset > 0)
		return 0;

	getnstimeofday(&current_ts);
	rtc_time_to_tm(current_ts.tv_sec - sys_tz.tz_minuteswest * 60 - t7_time,
		       &lc_tm);
	msg_len = sprintf(proc_msg, PROC_OUT_HHMMSS_FMT, lc_tm.tm_hour,
			  lc_tm.tm_min, lc_tm.tm_sec);

	if (length > msg_len)
		length = msg_len;

	left = raw_copy_to_user(buffer, proc_msg, length);
	*offset += length - left;

	if (left) {
		printk(KERN_ERR MODULE_TAG "failed to read %u from %u chars\n",
		       left, length);
	} else {
		printk(KERN_NOTICE MODULE_TAG "read %u chars\n", length);
	}

	return length - left;
}

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
		return sprintf(buf, PROC_OUT_HHMMSS_FMT, sec / 3600,
			       (sec / 60) % 60, sec % 60);
	else
		return sprintf(buf, PROC_OUT_SS_FMT, sec);
}

static int create_proc_t7(void)
{
	int err;
	proc_dir = proc_mkdir(PROC_DIRECTORY, NULL);
	if (NULL == proc_dir)
		return -EFAULT;

	proc_abs_time =
		proc_create("abs_time", 0444, proc_dir, &proc_abs_time_fops);
	if (NULL == proc_abs_time) {
		pr_err("ERROR create proc entry failed in %s\n", __FUNCTION__);
		err = -EFAULT;
		goto ERROR;
	}

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
		remove_proc_entry("abs_time", proc_dir);
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

static ssize_t set_time_store(struct class *class, struct class_attribute *attr,
			      const char *buf, size_t count)
{
	char _buff[16];
	struct rtc_time lc_tm;
	ulong _hh, _mm, _ss;
	int _len, _matches;
	strncpy(_buff, buf, sizeof(_buff));

	_len = strlen(_buff);
	if ((_len == 9 && _buff[_len - 1] == '\n') || (_len == 8)) {
		_matches = sscanf(_buff, "%2lu:%2lu:%2lu", &_hh, &_mm, &_ss);
		_hh %= 24;
		_mm %= 60;
		_ss %= 60;
		if (_matches == 3) {
			struct timespec _ts;
			getnstimeofday(&_ts);
			t7_time = (_ts.tv_sec - sys_tz.tz_minuteswest * 60) %
					  (SEC_IN_DAY) -
				  (_hh * 3600 + _mm * 60 + _ss);
		} else {
			pr_err("New time not set: wrong format\n");
		}
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
		goto error_hhmmss;

	class_attr_set_time.attr.mode = 0222;
	err = class_create_file(t7_class, &class_attr_set_time);
	if (err)
		goto error_set_time;

	return 0;

error_set_time:
	class_remove_file(t7_class, &class_attr_hhmmss);
error_hhmmss:
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
