# 並行処理と同期

## 共有メモリと競合状態
### 例
同一プロセス内のスレッドはメモリを共有している。

競合する例: 2 child threads

片方のg += 100 の間にもう一方のスレッドによりgが書き換えられていなければ成功する。
なぜ？: CPUが一度に行えるメモリに対する操作はread, writeどちらかだけである。
```c
int g = 0; // global

void *f(void *arg) {
    g += 100;
    return 0;
}

int main() {
    int err;
    g = 200;
    /* スレッドを作る */
    pthread_t child_thread_id[2];
    for (int i = 0; i < 2; i++)
        pthread_create(&child_thread_id[i], 0, f, 0);
    /* 終了待ち */
    for (int i = 0; i < 2; i++) {
        void * ret = 0;
        pthread_join(child_thread_id[i], &ret);
    }
    printf("g = %d\n", g);
    return 0;
}
```

### 用語の定義
定義: 競合状態とは以下のような状態
- 複数スレッドが
    - (a) 同じ場所を
    - (b) 並行してアクセスしていて
- うち少なくとも１つは書き込みである

定義: 際どい領域 Critical Section
- コード上で競合状態が発生している領域

### 競合状態の分類
- 不可分性(Atomicity)の崩れ
    - 一度にできない一連の操作の途中に他のUpdate処理が挟まるために、意図しない動作になる
- 順序・依存関係(Dependency)の崩れ
    - 複数スレッド間で、Read/Writeの順序保証をする必要がある処理なのに、その保証ができない場合にダメになる。

## 同期 (Synchronization)
### 排他制御 (mutex)
1人しか入れない個室トイレ
- `lock`: トイレが空いてれば入って鍵をかける。空いてなければ空くまで待つ
- `unlock`: 鍵を空けてトイレを空ける。
Atomicに行いたい処理をlock/unlockで挟む

#### API
API: [man page](https://linuxjm.osdn.jp/html/LDP_man-pages/man7/pthreads.7.html)
```c
#include <pthread.h>

pthread_mutex_t m; /* 排他制御オブジェクト */
pthread_mutex_init(&m, attr);
pthread_mutex_destroy(&m);
pthread_mutex_lock(&m); /* lock */
pthread_mutex_try_lock(&m);
pthread_mutex_unlock(&m); /* unlock */
```

