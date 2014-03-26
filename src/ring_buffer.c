#include <ring_buffer.h>
#include <linux/slab.h>

void ring_buf_init(ring_buf_p rbuf, unsigned int size)
{
	rbuf->size = size;
	rbuf->start = 0;
	rbuf->end = 0;
	rbuf->elems = kmalloc(rbuf->size * sizeof(int) ,GFP_KERNEL);
}

void ring_buf_free(ring_buf_p rbuf)
{
	kfree(rbuf->elems);
	rbuf->elems = NULL;
}

void ring_buf_clear(ring_buf_p rbuf)
{
	rbuf->start = 0;
	rbuf->end = 0;
}

int ring_buf_empty(ring_buf_p rbuf)
{
	return rbuf->end == rbuf->start;
}

int ring_buf_full(ring_buf_p rbuf)
{
	return rbuf->end == (rbuf->start ^ rbuf->size);
}

int ring_buf_size(ring_buf_p rbuf)
{
	return rbuf->end - rbuf->start;
}

int ring_buf_incr(ring_buf_p rbuf, unsigned int p)
{
	return (p+1)&(2*rbuf->size-1);
}

void ring_buf_write(ring_buf_p rbuf, int *elem)
{
	rbuf->elems[rbuf->end&(rbuf->size-1)] = *elem;
	if(ring_buf_full(rbuf))
		rbuf->start = ring_buf_incr(rbuf, rbuf->size);
	rbuf->end = ring_buf_incr(rbuf, rbuf->end);
}

void ring_buf_read(ring_buf_p rbuf, int *elem)
{
	*elem = rbuf->elems[rbuf->start & (rbuf->size-1)];
	rbuf->start = ring_buf_incr(rbuf, rbuf->start);
}

