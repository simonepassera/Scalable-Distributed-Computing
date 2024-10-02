#include <lba.h>
#include <stdlib.h>
#include <errno.h>

extern const int NUM_THREADS;

int lba_init(lba_t *data) {
  if ((data->choosing = (int*) calloc(NUM_THREADS, sizeof(int))) == NULL)
    return errno;
  
  if ((data->number = (int*) calloc(NUM_THREADS, sizeof(int))) == NULL) {
    free((void*) data->choosing);
    return errno;
  }

  return 0;
}

void lba_lock(lba_t *data, int rank) {
  data->choosing[rank] = 1;

  int max_number = 0;

  for (int i = 0; i < NUM_THREADS; i++) {
    if (data->number[i] > max_number) {
      max_number = data->number[i];
    }
  }

  data->number[rank] = max_number + 1;
  data->choosing[rank] = 0;

  for (int j = 0; j < NUM_THREADS; j++) {
    while(data->choosing[j]) {}
    while((data->number[j] != 0) && ((data->number[j] < data->number[rank]) || ((data->number[j] == data->number[rank]) && (j < rank)))) {}
  }
}

void lba_unlock(lba_t *data, int rank) {
  data->number[rank] = 0;
}

void lba_destroy(lba_t *data) {
  free((void*) data->choosing);
  free((void*) data->number);
}