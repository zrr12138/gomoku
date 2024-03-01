# gomoku

## 简介

在五子棋上实现了极大极小值搜索，实力不如纯人，主要受限于评估函数的准确性。

在五子棋上实现了蒙特卡洛算法，实力远大于纯人，且思考时间越长，算力越强，实力越强。

## 支持的环境

支持在mac和linux上编译运行，windows由于pthread兼容性问题，暂时无法正常运行。

拉取仓库

```bash 
git clone git@github.com:zrr12138/gomoku.git
```

更新子模块

```bash
cd gomoku
git submodule update --init --recursive
```

编译

```bash
cd engine
bash scripts/build.sh
```

运行

```
./ManualTest --thread_num 8 --think_time 2 --human_first=true
```

| 参数名         | 含义         |
|-------------|------------|
| thread_num  | 线程数        |
| think_time  | 思考时间（单位：秒） |
| human_first | 是否人类先手     |

当前性能(e6服务机型)

| 线程数 | 蒙特卡洛搜索次数/秒 |
|-----|------------|
| 1   | 175k       |
| 4   | 1748k      |
| 8   | 4673k      |
| 16  | 10203k     |
