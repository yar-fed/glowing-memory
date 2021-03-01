
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/kthread.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Yaroslav Fedoriachenko <yar.fed99@gmail.com>");
MODULE_DESCRIPTION("GLKH-BC task09");
MODULE_VERSION("0.1");

#define MODNAME "task09"
#define MODULE_TAG "t9_module "

#define TEST_MSG "TEST MESSAGE"
#define BTN_JITTER_FILTER_NS 100000
#define KTHREAD_POOL_SIZE 2
#define TMP_BUF_SIZE 256
#define PROC_DIRECTORY "t9"

static struct gpio g_out[4];
static struct gpio g_in[4];
static int irq_num;
static struct timespec btn_change_time;

static bool start_flag = false;

static DEFINE_MUTEX(wq_mutex);
LIST_HEAD(wq_head);
struct wq_list {
	u32 delay;
	u32 interval;
	char *msg;
	struct kthread_worker *worker;
	struct kthread_work work_meta;
	struct list_head list_meta;
};

static struct kthread_worker *kt_pool[KTHREAD_POOL_SIZE];

static int t9_write_msg(struct file *file_p, const char __user *buffer,
			size_t length, loff_t *offset);
static struct file_operations proc_msg_fops = {
	.write = t9_write_msg,
};

static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_msg;

static int create_proc_t9(void);
static void cleanup_proc_t9(void);
static void cleanup_heap(void);
static void job_function(struct kthread_work *work_arg);

struct task_struct *start_fnc_thread;
static int start_function(void *data)
{
	int i;
	struct list_head *pos;
	struct wq_list *next_job;
	while (!start_flag) {
		ssleep(5);
	}
	if (kthread_should_stop())
		return 0;

	mutex_lock(&wq_mutex);
	pos = wq_head.next;
	for (i = 0; i < KTHREAD_POOL_SIZE; ++i) {
		if (pos != &wq_head) {
			next_job = list_entry(pos, struct wq_list, list_meta);
			next_job->worker = kt_pool[i];
			kthread_queue_work(kt_pool[i], &next_job->work_meta);
			pos = pos->next;
			list_del(&next_job->list_meta);
			pr_notice(MODULE_TAG "Message queued");
		} else {
			break;
		}
	}
	mutex_unlock(&wq_mutex);
	return 0;
}

static void cleanup_heap(void)
{
	struct list_head *pos;
	struct list_head *n;
	struct wq_list *_wq;

	if (wq_head.next == &wq_head)
		return;
	list_for_each_safe (pos, n, &wq_head) {
		_wq = list_entry(pos, struct wq_list, list_meta);
		list_del(pos);
		kfree(_wq->msg);
		kthread_flush_work(&_wq->work_meta);
		kfree(_wq);
	}
}

static irqreturn_t gpio_isr(int irq, void *data)
{
	static struct timespec _now;
	static bool state = false;
	getnstimeofday(&_now);
	if (_now.tv_nsec - btn_change_time.tv_nsec > BTN_JITTER_FILTER_NS) {
		if (state)
			start_flag = true;
		state = !state;
	}
	btn_change_time = _now;
	return IRQ_HANDLED;
}

static int t9_write_msg(struct file *file_p, const char __user *buffer,
			size_t length, loff_t *offset)
{
	int err;
	char _buff[TMP_BUF_SIZE];
	struct wq_list *_new_msg;
	char *_delim_ptr1;
	char *_delim_ptr2;

	if (length > TMP_BUF_SIZE || *offset > 0)
		return 0;

	if (raw_copy_from_user(_buff, buffer, length)) {
		printk(KERN_ERR MODULE_TAG ":%s: copy_from_user failed",
		       __FUNCTION__);
		err = -EFAULT;
		goto error;
	}
	_new_msg = kmalloc(sizeof(*_new_msg), GFP_KERNEL);
	if (NULL == _new_msg) {
		err = -ENOMEM;
		goto error;
	}

	_delim_ptr1 = strchr(_buff, ':');
	if (NULL == _delim_ptr1) {
		err = -EINVAL;
		pr_err("errwf1\n");
		goto err_wrong_format1;
	}
	*_delim_ptr1 = '\0';

	_new_msg->msg = kmalloc(strlen(_buff) + 1, GFP_KERNEL);
	if (NULL == _new_msg->msg) {
		err = -ENOMEM;
		goto err_wrong_format1;
	}
	strcpy(_new_msg->msg, _buff);

	_delim_ptr2 = strchr(_delim_ptr1 + 1, ':');
	if (NULL == _delim_ptr2) {
		err = -EINVAL;
		pr_err("errwf2\n");
		goto err_wrong_format2;
	}
	*_delim_ptr2 = '\0';

	err = kstrtouint(_delim_ptr1 + 1, 0, &_new_msg->delay);
	if (err) {
		pr_err("errwf3\n");
		goto err_wrong_format2;
	}

	_delim_ptr1 = strchr(_delim_ptr2 + 1, '\n');
	if (NULL != _delim_ptr1)
		*_delim_ptr1 = '\0';
	err = kstrtouint(_delim_ptr2 + 1, 0, &_new_msg->interval);
	if (err) {
		pr_err("errwf4 '%s'\n", _delim_ptr2 + 1);
		goto err_wrong_format2;
	}

	kthread_init_work(&_new_msg->work_meta, job_function);
	INIT_LIST_HEAD(&_new_msg->list_meta);
	list_add_tail(&_new_msg->list_meta, &wq_head);

	*offset = length;

	return length;

err_wrong_format2:
	kfree(_new_msg->msg);
err_wrong_format1:
	kfree(_new_msg);
error:
	return err;
}

static int create_proc_t9(void)
{
	int err;
	proc_dir = proc_mkdir(PROC_DIRECTORY, NULL);
	if (NULL == proc_dir) {
		err = -EFAULT;
		goto error_dir;
	}

	proc_msg = proc_create("msg", 0222, proc_dir, &proc_msg_fops);
	if (NULL == proc_msg) {
		pr_err("ERROR create proc entry failed in %s\n", __FUNCTION__);
		err = -EFAULT;
		goto error_msg;
	}

	return 0;

error_msg:
	remove_proc_entry(PROC_DIRECTORY, NULL);
error_dir:
	return err;
}

static void cleanup_proc_t9(void)
{
	if (proc_dir) {
		remove_proc_entry("msg", proc_dir);
		remove_proc_entry(PROC_DIRECTORY, NULL);
		proc_dir = NULL;
	}
}

static void job_function(struct kthread_work *work_arg)
{
	struct list_head *pos;
	struct wq_list *next_job;
	struct wq_list *q_item =
		container_of(work_arg, struct wq_list, work_meta);

	printk(KERN_INFO "%s [delay: %u, interval: %u]\n", q_item->msg,
	       q_item->delay, q_item->interval);
	while (q_item->interval > q_item->delay) {
		msleep(q_item->delay);
		q_item->interval -= q_item->delay;
		printk(KERN_INFO "%s [delay: %u, interval: %u]\n", q_item->msg,
		       q_item->delay, q_item->interval);
	}

	mutex_lock(&wq_mutex);
	pos = wq_head.next;
	if (pos != &wq_head) {
		next_job = list_entry(pos, struct wq_list, list_meta);
		next_job->worker = q_item->worker;
		kthread_queue_work(q_item->worker, &next_job->work_meta);
		list_del(&next_job->list_meta);
	}
	mutex_unlock(&wq_mutex);
	kfree(q_item->msg);
	kfree(q_item);
}

static int __init t9_init(void)
{
	int err;
	int i;
	struct wq_list *test_msg;
	char *msg_str;

	err = create_proc_t9();
	if (err) {
		pr_err(MODULE_TAG "proc create");
		goto error_create_proc;
	}
	test_msg = kmalloc(sizeof(*test_msg), GFP_KERNEL);
	test_msg->delay = 1000;
	test_msg->interval = 2000;
	msg_str = kmalloc(strlen(TEST_MSG) + 1, GFP_KERNEL);
	strcpy(msg_str, TEST_MSG);
	test_msg->msg = msg_str;
	INIT_LIST_HEAD(&test_msg->list_meta);
	list_add_tail(&test_msg->list_meta, &wq_head);
	kthread_init_work(&test_msg->work_meta, job_function);

	for (i = 0; i < KTHREAD_POOL_SIZE; ++i) {
		kt_pool[i] = kthread_create_worker(0, "t9_worker%d", i);
	}
	start_fnc_thread =
		kthread_create(start_function, NULL, "t9_start_func");
	if (!IS_ERR(start_fnc_thread))
		wake_up_process(start_fnc_thread);

	g_out[0].gpio = 5;
	g_out[0].gpio = 6;
	g_out[0].gpio = 13;
	g_out[0].gpio = 19;
	g_in[0].gpio = 26;
	g_in[0].gpio = 16;
	g_in[0].gpio = 20;
	g_in[0].gpio = 21;
	for (i = 0; i < 4; ++i) {
		g_in[i].flags = GPIOF_IN;
		g_out[i].flags = GPIOF_OUT_INIT_HIGH;
	}

	err = gpio_request(g_out[0].gpio, "OUT");
	if (err) {
		pr_err(MODULE_TAG "g_out alloc");
		goto error_gout;
	}

	err = gpio_request(g_in[0].gpio, "IN");
	if (err) {
		pr_err(MODULE_TAG "g_in alloc");
		goto error_gin;
	}

	err = gpio_to_irq(g_in[0].gpio);
	if (err < 0) {
		pr_err(MODULE_TAG "irq1");
		goto error_irq;
	}

	irq_num = err;
	err = request_irq(irq_num, gpio_isr, IRQF_TRIGGER_RISING | UMH_DISABLED,
			  "gpio_irq", NULL);
	if (err) {
		pr_err(MODULE_TAG "irq2");
		goto error_irq;
	}
	//timer_setup(&t9_button_timer, t9_timer_callback, TIMER_IRQSAFE);
	getnstimeofday(&btn_change_time);

	printk(KERN_INFO MODULE_TAG "loaded\n");
	return err;

error_irq:
	gpio_free(g_in[0].gpio);
error_gin:
	gpio_free(g_out[0].gpio);
error_gout:
	for (i = 0; i < KTHREAD_POOL_SIZE; ++i) {
		kthread_flush_worker(kt_pool[i]);
	}
	cleanup_proc_t9();
error_create_proc:
	printk(KERN_ERR MODULE_TAG "failed to load\n");
	return err;
}

static void __exit t9_exit(void)
{
	int i;
	gpio_free(g_in[0].gpio);
	gpio_free(g_out[0].gpio);
	free_irq(irq_num, NULL);
	cleanup_heap();
	for (i = 0; i < KTHREAD_POOL_SIZE; ++i) {
		kthread_flush_worker(kt_pool[i]);
	}
	if (!start_flag) {
		start_flag = true;
		kthread_stop(start_fnc_thread);
	}
	cleanup_proc_t9();
	return;
}

module_init(t9_init);
module_exit(t9_exit);
