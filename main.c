#include <linux/module.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <asm/atomic.h>
#include <linux/random.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <ring_buffer.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("darkwsh");

static unsigned int max_wr_thread = 10;
module_param(max_wr_thread, uint, S_IRUSR|S_IWUSR);
static unsigned int max_rd_thread = 10;
module_param(max_rd_thread, uint, S_IRUSR|S_IWUSR);

static atomic_t v = ATOMIC_INIT(0);
static atomic_t rd_v = ATOMIC_INIT(0);
static atomic_t wr_v = ATOMIC_INIT(0);

static struct task_struct** wr_threads;
static struct task_struct** rd_threads;

static ring_buf_t rbuf;

DEFINE_SPINLOCK(lock);

static int val = 0;

// Thread about read task
static int rd_thread_do(void *data)
{
	do{
		spin_lock(&lock);
		if(!ring_buf_empty(&rbuf)){
			ring_buf_read(&rbuf,&val);
			atomic_dec(&v);
			atomic_inc(&rd_v);
		}
		spin_unlock(&lock);
		msleep(10);
	}while(!kthread_should_stop());
	return 0;
}

static int create_rd_threads(void)
{
	int i;
	for (i = 0; i < max_rd_thread; i++){
		struct task_struct *thread;
		thread = kthread_run(rd_thread_do,NULL, "rd_thread_%d", i);
		if (IS_ERR(thread))
			return -1;
		rd_threads[i] = thread;
	}
	return 0;
}

static void cleanup_rd_threads(void)
{
	int i;
	for (i = 0; i < max_rd_thread; i++){
		kthread_stop(rd_threads[i]);
		rd_threads[i] = NULL;
	}
}

// Thread about write task
static int wr_thread_do(void *data)
{
	do{
		spin_lock(&lock);
		if(!ring_buf_full(&rbuf)){
			val++;
			ring_buf_write(&rbuf,&val);
			atomic_inc(&v);
			atomic_inc(&wr_v);
		}
		spin_unlock(&lock);
		msleep(10);
	}while(!kthread_should_stop());
	return 0;
}

static int create_wr_threads(void)
{
	int i;
	for (i = 0; i < max_wr_thread; i++){
	      struct task_struct *thread;
	      thread = kthread_run(wr_thread_do, NULL, "wr_thread_%d", i);
	      if (IS_ERR(thread))
		    return -1;
	      wr_threads[i] = thread;
	}
	return 0;
}

static void cleanup_wr_threads(void)
{
	int i;
	for ( i = 0; i < max_wr_thread; i++){
	      kthread_stop(wr_threads[i]);
		wr_threads[i] = NULL;
	}
}

static __init int ring_buf_module_init(void)
{
	wr_threads = (struct task_struct**)kmalloc(sizeof(struct task_struct*)*max_wr_thread, GFP_KERNEL);
	rd_threads = (struct task_struct**)kmalloc(sizeof(struct task_struct*)*max_rd_thread, GFP_KERNEL);
	ring_buf_init(&rbuf, 512);
	if (create_rd_threads())
		goto err;
	if (create_wr_threads())
		goto err;
	return 0;
	
	err:
		cleanup_rd_threads();
		cleanup_wr_threads();
		return -1;
}

static __exit void ring_buf_module_exit(void)
{
	cleanup_rd_threads();
	cleanup_wr_threads();
	printk("At last, the number is : %d\n",atomic_read(&v));
	printk("Total %d reads\n",atomic_read(&rd_v));
	printk("Total %d writes\n", atomic_read(&wr_v));
	printk("Last %d elems not read.\n", ring_buf_size(&rbuf));
	ring_buf_free(&rbuf);
	ring_buf_clear(&rbuf);
	kfree(wr_threads);
	kfree(rd_threads);
	return;
}

module_init(ring_buf_module_init);
module_exit(ring_buf_module_exit);
