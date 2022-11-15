#include <assert.h>
#include <omp.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void die(char* msg) {
  perror(msg);
  exit(1);
}

double cur_time() {
  struct timespec ts[1];
  clock_gettime(CLOCK_REALTIME, ts);
  return ts->tv_sec + ts->tv_nsec * 1.0e-9;
}

/* 飽和カウンタ */
typedef struct {
  long x;
  long capacity;
  pthread_mutex_t m[1];
  pthread_cond_t c_lower[1];
  pthread_cond_t c_upper[1];
} scounter_t;

/* 初期化(値を0にする) */
void scounter_init(scounter_t* s, long capacity) {
  s->x = 0;
  s->capacity = capacity;
  if (pthread_mutex_init(s->m, 0)) {
    die("pthread_mutex_init");
  }
  if (pthread_cond_init(s->c_lower, 0)) {
    die("pthread_cond_init");
  }
  if (pthread_cond_init(s->c_upper, 0)) {
    die("pthread_cond_init");
  }
}

/* +1 ただしcapacityに達していたら待つ */
long scounter_inc(scounter_t* s) {
  pthread_mutex_lock(s->m);
  long x;
  while (1) {
    x = s->x;
    if (x < s->capacity) break;
    pthread_cond_wait(s->c_upper, s->m);
  }
  s->x = x + 1;
  if (x == 0) {
    pthread_cond_broadcast(s->c_lower);
  }
  pthread_mutex_unlock(s->m);
  assert(x < s->capacity);
  return x;
}

/* -1 */
long scounter_dec(scounter_t* s) {
  pthread_mutex_lock(s->m);
  long x;
  while (1) {
    x = s->x;
    if (x > 0) break;
    pthread_cond_wait(s->c_lower, s->m);
  }
  s->x = x - 1;
  if (x == s->capacity) {
    pthread_cond_broadcast(s->c_upper);
  }
  pthread_mutex_unlock(s->m);
  assert(x >= 0);
  return x;
}

/* 現在の値を返す */
long scounter_get(scounter_t* s) { return s->x; }

int main(int argc, char** argv) {
  int i = 1;
  /* incを呼ぶスレッド数 */
  int n_inc_threads = (argc > i ? atoi(argv[i]) : 3);
  i++;
  /* decを呼ぶスレッド数 */
  int n_dec_threads = (argc > i ? atoi(argv[i]) : 2);
  i++;
  /* incとdecが呼ばれる回数(全スレッドの合計) */
  long n = (argc > i ? atol(argv[i]) : 10000);
  i++;
  /* 飽和する値 */
  long capacity = (argc > i ? atol(argv[i]) : 10000);
  i++;

  scounter_t s[1];
  scounter_init(s, capacity);

  printf("increment threads : %d\n", n_inc_threads);
  printf("decrement threads : %d\n", n_dec_threads);
  printf("increments/decrements : %ld\n", n);
  printf("capacity : %ld\n", capacity);

  double t0 = cur_time();
#pragma omp parallel num_threads(n_inc_threads + n_dec_threads)
  {
    int idx = omp_get_thread_num();
    if (idx < n_inc_threads) {
      /* increment */
      long a = n * idx / n_inc_threads;
      long b = n * (idx + 1) / n_inc_threads;
      for (long i = a; i < b; i++) {
        long x = scounter_inc(s);
        assert(x < capacity);
        assert(x >= 0);
      }
    } else {
      /* decrement */
      idx -= n_dec_threads;
      long a = n * idx / n_dec_threads;
      long b = n * (idx + 1) / n_dec_threads;
      for (long i = a; i < b; i++) {
        long x = scounter_dec(s);
        assert(x <= capacity);
        assert(x > 0);
      }
    }
  }
  double t1 = cur_time();
  printf("took %.9f sec\n", t1 - t0);
  long x = scounter_get(s);
  printf("%s : value at the end = %ld\n", (x == 0 ? "OK" : "NG"), x);
  return (x == 0 ? 0 : 1);
}