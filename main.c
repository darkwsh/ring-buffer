#include <linux/module.h>
#include <linux/kthread.h>
#include <asm/atomic.h>
#include <linux/random.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <ring_buffer.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("darkwsh");

#define MAX_WR_THREAD 10
#define MAX_RD_THREAD 10

static atomic_t v = ATOMIC_INIT(0);

static struct task_struct *wr_threads[MAX_WR_THREAD];
static struct task_struct *rd_threads[MAX_RD_THREAD];

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
			printk("Val is : %d .\n",val);
		}
		spin_unlock(&lock);
		msleep(10);
	}while(!kthread_should_stop());
	return 0;
}

static int create_rd_threads(void)
{
	int i;
	for (i = 0; i < MAX_RD_THREAD; i++){
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
	for (i = 0; i < MAX_RD_THREAD; i++){
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
		}
		spin_unlock(&lock);
		msleep(10);
	}while(!kthread_should_stop());
	return 0;
}

static int create_wr_threads(void)
{
	int i;
	for (i = 0; i < MAX_WR_THREAD; i++){
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
	printk("wr_thread clean");
	for ( i = 0; i < MAX_WR_THREAD; i++){
	      kthread_stop(wr_threads[i]);
		wr_threads[i] = NULL;
	}
}

static __init int ring_buf_module_init(void)
{
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
	printk("At last, the number is : %d",atomic_read(&v));
	return;
}

module_init(ring_buf_module_init);
module_exit(ring_buf_module_exit);
