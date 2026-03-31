# Simulated Stock Exchange

This is a simulated stock exchange build using only C++. My goal is to build a stock exchange that has some
functionality from a real exchange like, for example, NYSE. That includes an order book, order matching engine, frontend
for simple user UI and data transfer protocol between frontend and backend. The main goals for this project are to
understand more on how financial markets work, create software that is as low-latency as possible, practice C++
programming and gain overall software engineering and designing experience.

## Table of contents

1. [Main Features](#main-features)
2. [Documentation](#documentation)
3. [Benchmarks](#benchmarks)
4. [Build & Run](#build--run)
5. [Current Progress & Future work](#current-progress--future-work)

## Main Features

- Order book supports 5 order types: Market, Fill or Kill (FOK), Fill and Kill (FAK), Good till Cancel (GTC), Good till
  End of Day (GTD).
- Submitted GTC and GTD orders are stored on the bid or ask side, if necessry. Market, FAK and FOK orders are filled (if
  possible) without ever seeing the price ladder

## Documentation

For more technical details, see the [documentation](DOCS.md)

## Benchmarks

Performance testing and profiling results will be published here once the matching engine and multi-participant
simulation are implemented.

## Build & Run

Currently, the project compiles and the developer can simulate actions in [main.cpp](src/main.cpp) manually by importing the
`Orderbook` class and using the public API to call methods. To build and execute, run:

```bash
cmake -B build
cmake --build build
./build/orderbook
```

To run tests, do:
```bash
cmake -B build
cmake --build build
cd build && ctest
```

## Author notes:
- currently working on replay system where a user can feed in file of events documented in `src/replay/DOCS.md` and process / 
inspect this data in many ways
- API has been started but is buggy and is not worked on currently

