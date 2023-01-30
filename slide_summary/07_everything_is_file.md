# ファイルディスクリプタと疑似ファイル

## Unixの特徴
### 1. 出力先の変更
Terminalに出力するつもりでプログラムを書けば、それがそのままファイルを出力するプログラムになる

```c
int main() { printf("hello\n"); }
```

```bash
$ ./hello
hello
```
出力先をredirect
```bash
$ ./hello > out.txt
```

### 2. 入力先の変更
同様に入力も。
```bash
$ cat content.txt
hello hiroshi
$ ./say_something < content.txt
hello hiroshi
```

### 3. パイプでプロセス間通信
ex
```bash
ps auxww | grep firefox
```

プロセス外部とのやりとりはすべてファイルディスクリプタを経由して行われる。

everything is file
- プロセスごとにアドレス空間は分離されている
- fdに `read/write`を発行して通信しよう


## リダイレクト・パイプの仕組み
### 子プロセスにfdを継承する
```c
int fd = open(...);
pid_t pid = fork();
if (pid == 0) {
    // child
    read(fd, buf, sz); // OK
}
```
childの中でexecしても有効なので、execした先でもfdの値さえ分かれば openした内容を使える。

### 標準入出力
- fd=0: 標準入力
- fd=1: 標準出力
- fd=2: 標準エラー出力

### ファイルAPI(高水準)
生のfdに対応する高水準APIがC言語では提供されている
```c
FILE *fp = fopen(filename, mode);
fread(buf, size, n, fp);
fwrite(buf, size, n, fp);
```
日本語記事: https://programming-place.net/ppp/contents/c/040.html#rw_open

### `dup2` システムコール
```c
int err = dup2(oldfd, newfd);
```
ファイルディスクリプタ`oldfd`を`newfd`でも使えるようにする(複製)。

#### 入力リダイレクト`cmd < filename`の実現方法
```c
const int fd = open(filename, O_RDONLY);
pid_t pid = fork();
if (pid) {
    // parent
    close(fd); // 親には不要
} else {
    // child: fd -> 0へ付け替えて、0(stdin)で読めるように
    if (fd != 0) {
        dup2(fd, 0);
        close(fd);
    }
    execvp(cmd, ...);
}
```
#### 出力リダイレクト`cmd > filename`の実現方法
```c
const int fd = creat(filename); // 新しいfdを生成
pid_t pid = fork();
if (pid) {
    // parent
    close(fd); // 親には不要
} else {
    // child: fd -> 1へと付け替えて、1(stdout)に吐けるように
    if (fd != 1) {
        close(fd);    // ここの順序は先にcloseしないとバグるのかは不明
        dup2(fd, 1);
    }
    execvp(cmd, ...);
}
```

### `pipe`システムコール
```c
int rw[2]; // fdの配列
int err = pipe(rw);
```
`rw[0]`, `rw[1]`にそれぞれ読み出し・書き出し用のfdが生成される。
`rw[1]`に書いたデータが`rw[0]`から呼び出せる。
これをforkによるfdの継承を使って、親子プロセス間通信を実現。

#### pipe (親 -> 子)
親がwに書き出したものを、子がstdio(0)から読めれば良い
```c
int rw[2]; pipe(rw);
int r = rw[0], w = rw[1];
pid_t pid = fork();
if (pid) { // parent
    close(r); // 親はreadは不要
    // WRITE to `w` here
    close(w);
} else { // child
    close(w); // 子はwrite不要
    dup2(r, 0);
    close(r);
    execvp(...);    // stdio(0)から読むコマンド
}
```
#### pipe (子 -> 親)
子がstdout(1)に書き出したものを、親がrから読めれば良い
```c
int rw[2]; pipe(rw);
int r = rw[0], w = rw[1];
pid_t pid = fork();
if (pid) { // parent
    close(w); // 親はwrite不要
    // READ from `r` here
    close(r);
} else { // child
    close(r); // 子はread不要
    dup2(w, 1);
    close(w);
    execvp(...);    // stdout(1)へ書き出すコマンド
}
```
例: `popen`ライブラリ関数

## 疑似ファイル
2次記憶上のデータに限らず、openしてread/writeできるものは全てファイルとする。

- 名前付きパイプ(FIFO)
    - `int err = mkfifo(pathname, mode);`
- `/proc`ファイルシステム
    - プロセスやOSの内部状態に関するファイル
- `cgroups`ファイルシステム(詳細は05memory.pdf)
    - プロセスの集合に割り当てる資源を制御する機能
    - `sudo mount -t cgroup2 none dir`
- `tmpfs`
    - 実体がメモリ上にある(揮発)ファイルシステム
- デバイスファイル
    - I/O装置(camera, microphone, ...)もファイルとして扱う
    - 単純な例:
        - `/dev/null`, `/dev/zero`, `/dev/urandom`
