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
mkdir build 
cd build
cmake ..
cmake --build .
./orderbook
```

## Current Progress & Future Work

- [x] Implement core backend for the orderbook. That includes bid and ask sides, order types, order placement.
- [x] Implement a matching engine that matches orders by price-time priority.
- [ ] Generate unit and integration tests to prevent bugs
- [ ] Simulate lots of participants to test how the engine performs with a lot of interactions per second. Fill out
  the [benchmarks](#benchmarks) section of this README
- [ ] Create frontend that interacts with the public API from the backend to place orders, see depth of market and
  show a real price ladder.
