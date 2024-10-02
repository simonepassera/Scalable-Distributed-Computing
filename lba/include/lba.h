#ifndef __LBA_H_
#define __LBA_H_

typedef struct {
  volatile int *choosing;
  volatile int *number;
} lba_t;

extern void lba_lock(lba_t *data, int rank);
extern void lba_unlock(lba_t *data, int rank);
extern int lba_init(lba_t *data);
extern void lba_destroy(lba_t *data);


#endif /* __LBA_H__ */
