# Performance Log

This document tracks order book benchmark results over time. The goal is to make performance changes measurable and explain why optimizations were made.

## Benchmark Environment

* Machine: My local desktop
* CPU: AMD Ryzen 5 5600X 6-Core Processor, x86_64 architecture
* OS: Linux Mint 22.2
* Kernel: Linux 6.8.0-124-generic
* Compiler: g++ (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0

## Benchmark Setup

The benchmark loads a command file, parses it once, and repeatedly executes the parsed commands against the order book.

Reported metrics:

* Commands per iteration
* Iterations
* Total commands
* Elapsed time
* Nanoseconds per command
* Commands per second
* Optional `perf stat`, `valgrind` or similar outputs, if relevant

Important limitation: the current benchmark repeats a small scripted scenario. This is useful as a smoke benchmark, but it is not yet representative of large realistic order book workloads.

## Baseline: Initial Implementation

Input file:

```txt
data/input.txt
```

Commands per iteration: 14
Iterations (default): 1,000,000
Total commands: 14,000,000

### Result

```txt
elapsed ns: 1,726,093,458
ns/command: 123.29
commands/sec: 8,110,800.68
```

### Notes

This benchmark repeatedly executes a small deterministic scenario. The result is stable enough for tracking regressions, but it likely benefits from predictable branches and a tiny book size.
There also has been an observerd issue - increasing iteration count increases time that is taken for one commnand, here are some outputs:

```bash
$ ./orderbook_benchmark --iterations=100000
```
```text
Benchmark workload
------------------
file: "/home/janis/dev/orderbook/data/input.txt"
commands/iteration: 14
iterations: 100000
total commands: 1400000

Benchmark result
----------------
elapsed ns: 153891906
ns/command: 109.92
commands/sec: 9097294.56
```

```bash
$ ./orderbook_benchmark --iterations=1000000
```
```text
Benchmark workload
------------------
file: "/home/janis/dev/orderbook/data/input.txt"
commands/iteration: 14
iterations: 1000000
total commands: 14000000

Benchmark result
----------------
elapsed ns: 1703930547
ns/command: 121.71
commands/sec: 8216297.33
```

```bash
$ ./orderbook_benchmark --iterations=10000000
```
```text
Benchmark workload
------------------
file: "/home/janis/dev/orderbook/data/input.txt"
commands/iteration: 14
iterations: 10000000
total commands: 140000000

Benchmark result
----------------
elapsed ns: 19424413389
ns/command: 138.75
commands/sec: 7207424.86
```

Investigation:

* Valgrind showed no leaks.
* `/usr/bin/time -v` showed increasing maximum resident set size.

update 1: by creating a file with `scripts/command_generator.py` that actually grows the book, results are fairly consistent when iterations icrease:
```bash
$ ./orderbook_benchmark --iterations=1 --filename=book_growth.txt
```
```text
"/home/janis/dev/orderbook/data/book_growth.txt"
Benchmark workload
------------------
file: book_growth.txt
commands/iteration: 1000000
iterations: 1
total commands: 1000000

Benchmark result
----------------
elapsed ns: 264494143
ns/command: 264.49
commands/sec: 3780802.06
```

```bash
$ ./orderbook_benchmark --iterations=10 --filename=book_growth.txt
```
```text
"/home/janis/dev/orderbook/data/book_growth.txt"
Benchmark workload
------------------
file: book_growth.txt
commands/iteration: 1000000
iterations: 10
total commands: 10000000

Benchmark result
----------------
elapsed ns: 2678720297
ns/command: 267.87
commands/sec: 3733125.86
```

// fix will be documented here

## Optimization Entries
All optimization tries (good or bad) will be documented here. I will explain reasons why I thought this is a good optimization and maybe some sources what i read as well will be documented here. I will explain reasons why I thought this is a good optimization and maybe some sources what i read as well.

