
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
MODULE_DESCRIPTION("GLKH-BC task05");
MODULE_VERSION("0.1");

#define MODULE_TAG "t5_module "
#define PROC_DIRECTORY "t5"
#define BUFFER_SIZE 128

static size_t proc_buffer;
static size_t proc_msg_length;
static size_t proc_msg_read_pos;

// TODO: add max_size
struct {
	size_t size;
	struct currency_t {
		char *name;
		struct proc_dir_entry *proc_ent;
		size_t rate_to_uah001;
	} arr[];
} * currencies;

static struct proc_dir_entry *proc_dir;

static int t5_read(struct file *file_p, char __user *buffer, size_t length,
		   loff_t *offset);
static int t5_write(struct file *file_p, const char __user *buffer,
		    size_t length, loff_t *offset);

/* should output multiple currencies, but only outputs one
 * TODO: implement with different macros to handle dynamical addition of files
 */
static ssize_t show_show(struct class *class, struct class_attribute *attr,
			 char *buf);

/* dummy function
 * TODO: implement with different macros to handle dynamical addition of files
 */
static ssize_t add_store(struct class *class, struct class_attribute *attr,
			 const char *buf, size_t count);
/* dummy function
 * TODO: implement with different macros to handle dynamical addition of files
 */
static ssize_t set_store(struct class *class, struct class_attribute *attr,
			 const char *buf, size_t count);

CLASS_ATTR_WO(add);
CLASS_ATTR_WO(set);
CLASS_ATTR_RO(show);

static struct class *t5_class;

static int create_sfs_t5(void);
static void cleanup_sfs_t5(void);

static struct file_operations proc_fops = {
	.read = t5_read,
	.write = t5_write,
};

static int t5_add_currency(char *name, size_t rate_to_uah);
static int create_proc_t5(void);
static void cleanup_proc_t5(void);

static size_t t5_convert_from(char *cur_name, size_t value);
static void t5_convert_to(char *cur_name, uint *out_whole, uint *out_mod);

static ssize_t show_show(struct class *class, struct class_attribute *attr,
			 char *buf)
{
	char _buff[BUFFER_SIZE];
	char *buf_p = buf;
	size_t _size = BUFFER_SIZE;
	int i;

	for (i = 0; i < currencies->size && _size > 0; ++i) {
		uint _whole, _mod;
		_whole = t5_convert_from(currencies->arr[i].name, 1);
		_mod = _whole % 100;
		_whole /= 100;
		_size -= snprintf(_buff, _size, "%s is %u.%02u uah\n",
				  currencies->arr[i].name, _whole, _mod);
		buf_p = strcpy(buf_p, _buff);
	}

	printk("read %ld\n", (long)strlen(buf));
	return strlen(buf);
}

static ssize_t add_store(struct class *class, struct class_attribute *attr,
			 const char *buf, size_t count)
{
	char _buff;
	strncpy(&_buff, buf, sizeof(_buff));

	return count;
}

static ssize_t set_store(struct class *class, struct class_attribute *attr,
			 const char *buf, size_t count)
{
	char _buff;
	strncpy(&_buff, buf, sizeof(_buff));

	return count;
}

static int create_sfs_t5(void)
{
	int err;

	t5_class = class_create(THIS_MODULE, "t5");
	err = IS_ERR(t5_class);
	if (err)
		return err;

	err = class_create_file(t5_class, &class_attr_add);
	if (err)
		return err;

	err = class_create_file(t5_class, &class_attr_set);
	if (err)
		return err;

	err = class_create_file(t5_class, &class_attr_show);
	if (err)
		return err;

	return 0;
}

static void cleanup_sfs_t5(void)
{
	class_remove_file(t5_class, &class_attr_set);
	class_remove_file(t5_class, &class_attr_show);
	class_remove_file(t5_class, &class_attr_add);
	class_destroy(t5_class);
	return;
}

static int t5_add_currency(char *name, size_t rate_to_uah)
{
	int i = currencies->size + 1;
	size_t name_len = strlen(name);

	// TODO: use max_size and test
	//void *tmp = kmalloc(sizeof(uintptr_t) * (i), GFP_KERNEL);
	//if (tmp == NULL) {
	//pr_err("NO MEM");
	//return -ENOMEM;
	//}
	//memcpy(tmp, proc_files, proc_files->size);
	//kfree(proc_files);
	//proc_files = tmp;

	currencies->arr[i - 1].proc_ent = proc_create(
		name, S_IFREG | S_IRUGO | S_IWUGO, proc_dir, &proc_fops);

	if (NULL == currencies->arr[i - 1].proc_ent) {
		pr_err("ERROR create proc entry");
		return -EFAULT;
	}

	currencies->arr[i - 1].name = kmalloc(name_len + 1, GFP_KERNEL);
	if (NULL == currencies->arr[i - 1].name) {
		pr_err("ERROR add_currency kmalloc returned null");
		return -ENOMEM;
	}

	strcpy(currencies->arr[i - 1].name, name);
	currencies->arr[i - 1].name[name_len] = '\0';
	currencies->arr[i - 1].rate_to_uah001 = rate_to_uah;

	currencies->size += 1;
	return 0;
}

static int create_proc_t5(void)
{
	proc_dir = proc_mkdir(PROC_DIRECTORY, NULL);
	if (NULL == proc_dir)
		return -EFAULT;

	currencies = kmalloc(sizeof(currencies->size) +
				     2 * sizeof(currencies->arr[0]),
			     GFP_KERNEL);
	if (NULL == currencies)
		return -ENOMEM;

	currencies->size = 0;
	proc_msg_read_pos = 0;

	return 0;
}

static void cleanup_proc_t5(void)
{
	int i;

	if (proc_dir) {
		for (i = 0; i < currencies->size; ++i) {
			remove_proc_entry(currencies->arr[i].name, proc_dir);
		}
		remove_proc_entry(PROC_DIRECTORY, NULL);
		proc_dir = NULL;
		kfree(currencies);
	}
}

static size_t t5_convert_from(char *cur_name, size_t value)
{
	int i;
	for (i = 0; i < currencies->size; ++i) {
		if (strcmp(currencies->arr[i].name, cur_name) == 0)
			break;
	}

	return value * currencies->arr[i].rate_to_uah001;
}

static void t5_convert_to(char *cur_name, uint *out_whole, uint *out_mod)
{
	int i;
	for (i = 0; i < currencies->size; ++i) {
		if (strcmp(currencies->arr[i].name, cur_name) == 0)
			break;
	}

	*out_whole = proc_buffer / (currencies->arr[i].rate_to_uah001);
	*out_mod = (proc_buffer % (currencies->arr[i].rate_to_uah001)) * 100 /
		   (currencies->arr[i].rate_to_uah001);
}

static int t5_read(struct file *file_p, char __user *buffer, size_t length,
		   loff_t *offset)
{
	static char proc_msg[128] = "0\n";
	size_t left;
	char *name = file_p->f_path.dentry->d_iname;

	if (proc_msg_read_pos == 0) {
		uint _whole, _mod;
		t5_convert_to(name, &_whole, &_mod);
		sprintf(proc_msg, "%u.%02u", _whole, _mod);
		proc_msg_length = strlen(proc_msg);
	}
	if (length > (proc_msg_length - proc_msg_read_pos))
		length = (proc_msg_length - proc_msg_read_pos);

	left = raw_copy_to_user(buffer, proc_msg, length);

	proc_msg_read_pos += length - left;

	if (left) {
		printk(KERN_ERR MODULE_TAG "failed to read %u from %u chars\n",
		       left, length);
	} else {
		printk(KERN_NOTICE MODULE_TAG "read %u chars\n", length);
	}

	return length - left;
}

static int t5_write(struct file *file_p, const char __user *buffer,
		    size_t length, loff_t *offset)
{
	int err;
	char _buff[BUFFER_SIZE];
	uint in_value;
	size_t msg_length;
	size_t left;
	char *name = file_p->f_path.dentry->d_iname;

	if (length > BUFFER_SIZE) {
		printk(KERN_WARNING MODULE_TAG
		       "reduse message length from %u to %u chars\n",
		       length, BUFFER_SIZE);
		msg_length = BUFFER_SIZE;
	} else {
		msg_length = length;
	}

	left = raw_copy_from_user(_buff, buffer, msg_length);

	proc_msg_length = msg_length - left;
	proc_msg_read_pos = 0;

	if (left) {
		printk(KERN_ERR MODULE_TAG "failed to write %u from %u chars\n",
		       left, msg_length);
	} else {
		printk(KERN_NOTICE MODULE_TAG "written %u chars\n", msg_length);
		_buff[msg_length] = '\0';
		err = kstrtouint(_buff, 0, &in_value);
		if (err) {
			proc_buffer = 0;
			pr_err("Error %d _buff %s in_value %u", err, _buff,
			       in_value);
			return err;
		} else {
			err = t5_convert_from(name, in_value);
			if (err < 0)
				pr_err("Error during convertion '%d'", err);
			proc_buffer = err;
		}
	}

	return length;
}

static int __init t5_init(void)
{
	int err;

	err = create_sfs_t5();
	if (err)
		goto error;

	err = create_proc_t5();
	if (err)
		goto error;

	err = t5_add_currency("uah", 100);
	if (err)
		goto error;

	err = t5_add_currency("eur", 3100);
	if (err)
		goto error;

	printk(KERN_NOTICE MODULE_TAG "loaded\n");
	return 0;

error:
	printk(KERN_ERR MODULE_TAG "failed to load\n");
	cleanup_proc_t5();
	cleanup_sfs_t5();
	return err;
}

static void __exit t5_exit(void)
{
	cleanup_proc_t5();
	cleanup_sfs_t5();
	printk(KERN_NOTICE MODULE_TAG "exited\n");
}

module_init(t5_init);
module_exit(t5_exit);
