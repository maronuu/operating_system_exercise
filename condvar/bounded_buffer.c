#include <assert.h>
#include <omp.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

void die(const char *msg) {
  perror(msg);
  exit(1);
}

double cur_time() {
  struct timespec ts[1];
  clock_gettime(CLOCK_REALTIME, ts);
  return ts->tv_nsec * 1.0E-9 + ts->tv_sec;
}

/* 有限バッファ
   要素は必ず >= 0 とする */
typedef struct {
  long n_gets;   /* getされた回数 */
  long n_puts;   /* putされた回数 */
  long capacity; /* 容量 */
  long *a;       /* 中身(capacity要素の配列) */
  pthread_mutex_t m[1];
  pthread_cond_t c_full[1];
  pthread_cond_t c_empty[1];
} bounded_buffer_t;

/* 容量 capacity で初期化 */
void bounded_buffer_init(bounded_buffer_t *bb, long capacity) {
  long *a = (long *)malloc(sizeof(long) * capacity);
  if (!a) die("malloc");
  bb->a = a;
  bb->capacity = capacity;
  bb->n_gets = 0;
  bb->n_puts = 0;
  if (pthread_mutex_init(bb->m, 0)) {
    die("pthread_mutex_init");
  }
  if (pthread_cond_init(bb->c_empty, 0)) {
    die("pthread_cond_init");
  }
  if (pthread_cond_init(bb->c_full, 0)) {
    die("pthread_cond_init");
  }
}

/* 要素を追加
 * 満杯だったら待つようにするのが課題
 * 以下はそのままだと満杯の場合は0を返す(間違い)
 */
int bounded_buffer_put(bounded_buffer_t *bb, long x) {
  pthread_mutex_lock(bb->m);
  long g, p, cap;
  assert(x >= 0);
  while (1) {
    g = bb->n_gets;
    p = bb->n_puts;
    cap = bb->capacity;
    if (p - g < cap) break;
    pthread_cond_wait(bb->c_full, bb->m);
  }
  bb->a[p % cap] = x;
  bb->n_puts = p + 1;
  if (p - g == 0) {
    pthread_cond_broadcast(bb->c_empty);
  }
  pthread_mutex_unlock(bb->m);

  return 1; /* OK */
}

/* 要素を取り出す
 * 空だったら待つようにするのが課題
 * 以下はそのままだと空の場合は-1を返す(間違い)
 */
long bounded_buffer_get(bounded_buffer_t *bb) {
  pthread_mutex_lock(bb->m);
  long g, p, cap;
  while (1) {
    g = bb->n_gets;
    p = bb->n_puts;
    cap = bb->capacity;
    if (p - g > 0) break;
    pthread_cond_wait(bb->c_empty, bb->m);
  }
  long x = bb->a[g % cap];
  bb->n_gets = g + 1;
  if (p - g == cap) {
    pthread_cond_broadcast(bb->c_full);
  }
  pthread_mutex_unlock(bb->m);
  assert(x >= 0);
  return x;
}

int main(int argc, char **argv) {
  long i = 1;
  /* putするスレッド数 */
  int n_putter_threads = (argc > i ? atoi(argv[i]) : 1);
  i++;
  /* getするスレッド数 */
  int n_getter_threads = (argc > i ? atoi(argv[i]) : 1);
  i++;
  /* putとgetの間にbarrierを入れるか? */
  int barrier_between_puts_gets = (argc > i ? atoi(argv[i]) : 1);
  i++;
  /* put (get)される回数 */
  long n = (argc > i ? atol(argv[i]) : 1000000);
  i++;
  /* 容量 */
  long capacity = (argc > i ? atol(argv[i]) : 1000);
  i++;
  /* 検証用(validate[x] == 1 iff getでxが取り出された) */
  char *validate = (char *)calloc(n, 1);

  bounded_buffer_t bb[1];
  bounded_buffer_init(bb, capacity);

  int nthreads = n_putter_threads + n_getter_threads;
  pthread_barrier_t barrier[1];
  pthread_barrier_init(barrier, 0, nthreads);

  double t0 = cur_time();
#pragma omp parallel num_threads(n_putter_threads + n_getter_threads)
  {
    int idx = omp_get_thread_num();
    if (idx < n_putter_threads) {
      /* I am a putter thread */
      long a = n * idx / n_putter_threads;
      long b = n * (idx + 1) / n_putter_threads;
      /* 0,1,...,n-1 を1つずつput */
      for (long x = a; x < b; x++) {
        int ok = bounded_buffer_put(bb, x);
        assert(ok);
      }
      if (barrier_between_puts_gets) {
        /* putが全員終わってからget */
        pthread_barrier_wait(barrier);
      }
    } else {
      /* I am a getter thread */
      idx -= n_putter_threads;
      long a = n * idx / n_getter_threads;
      long b = n * (idx + 1) / n_getter_threads;
      if (barrier_between_puts_gets) {
        /* putが全員終わってからget */
        pthread_barrier_wait(barrier);
      }
      /* 合計n回get */
      for (long x = a; x < b; x++) {
        long x = bounded_buffer_get(bb);
        assert(x >= 0);
        assert(x < n);
        assert(validate[x] == 0);
        validate[x] = 1;
      }
    }
  }
  double t1 = cur_time();
  printf("%f sec\n", t1 - t0);
  for (long i = 0; i < n; i++) {
    assert(validate[i]);
  }
  printf("OK\n");
  return 0;
}