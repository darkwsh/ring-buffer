#ifndef __RING_BUFFER__

#define SIZE_MAX 512

#define IDX_MASK (SIZE_MAX >> 1)
#define MSB_MASK (~IDX_MASK)

struct ring_buf{ 
	unsigned int size; 
	unsigned int start; 
	unsigned int end; 
	int *elems; 
};

typedef struct ring_buf ring_buf_t;
typedef struct ring_buf* ring_buf_p;

void ring_buf_init (ring_buf_p rbuf,  unsigned int size);
void ring_buf_free (ring_buf_p rbuf);
void ring_buf_clear (ring_buf_p rbuf);

int ring_buf_full (ring_buf_p rbuf);
int ring_buf_empty (ring_buf_p rbuf);
int ring_buf_size (ring_buf_p rbuf);

void ring_buf_write(ring_buf_p rbuf, int *elem);
void ring_buf_read(ring_buf_p rbuf, int *elem);

#define __RING_BUFFER__
#endif
