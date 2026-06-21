#!/usr/bin/env python3

import random

N = 1_000_000
random.seed(42)

with open("data/book_growth.txt", "w") as f:
    for i in range(1, N + 1):
        # Mostly passive orders that do not cross.
        # Buys stay below 10000, sells stay above 10000.
        if random.random() < 0.5:
            side = "buy"
            price = random.randint(9500, 9999)
        else:
            side = "sell"
            price = random.randint(10001, 10500)

        qty = random.randint(1, 100)

        f.write(f"ADD GTC {qty} {side} {price}\n")
