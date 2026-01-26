# Architecture Overview

```
+-------------+        +-----------+        +--------------+
|  Producers  |  --->  |   Queue   |  --->  |   Consumers  |
+-------------+        +-----------+        +--------------+
```

* **Producers**: Clients sending requests
* **Queue**: Central message queue
* **Consumers**: Order matching engine / market data handlers

---

# Connections & I/O Model

* Each client connection is identified by a **socket file descriptor (fd)**.
* Connections are managed using **`epoll`**.
* `epoll` notifies when:

  * A connection is ready for **reading**
  * A connection is ready for **writing**

---

# Message Categories

## 1. Connection Management

| Message              | Description                       | Response |
| -------------------- | --------------------------------- | -------- |
| `connect`            | Establish a new client connection | `userID`   |
| `disconnect(userID)` | Remove client state               | `null`     |


---

## 2. Order Management

All order-related messages **require `userID`**.

| Message       | Parameters                  | Description              | Response |
| ------------- | --------------------------- | ------------------------ | -------- |
| `openOrder`   | quantity, price, type, side | Submit a new order       |`orderID`|
| `cancelOrder` | tradeID                     | Cancel an existing order |`null`|
| `modifyOrder` | tradeID, modifications      | Modify an existing order, does not change the orderID |`null`|

`modifications` may include changes to:

* quantity
* price
* order type
* side

---

## 3. Market Data Queries

| Query                         | Response               | Response |
| ----------------------------- | ---------------------- | -------- |
| `bestBid / bestAsk`           | Single price           | `price_t` |
| `fullDepthBid / fullDepthAsk` | `vector<price, count>` | `vector<price_t>`|


---

# Persistent Storage (maybe won't be used)

A database is used for:

* Price snapshots
* User permissions / access control
* (Optionally) historical trade data

⚠️ **Execution-critical paths must NOT depend on the database**.

---

# API Message Format

All messages use a **binary framed format**:

```
| total_len (4B) | callID (4B) | params (KV pairs) |
```

## Field Description

| Field       | Size     | Description                  |
| ----------- | -------- | ---------------------------- |
| `total_len` | 4 bytes  | Total message length         |
| `callID`    | 4 bytes  | Specific operation           |
| `params`    | variable | Key-value encoded parameters |

---

# Parameter Encoding

## Parameter Keys

| Key | Meaning       |
| --- | ------------- |
| 0   | userID        |
| 1   | tradeID       |
| 2   | quantity      |
| 3   | price         |
| 4   | type          |
| 5   | side          |
| 6   | modifications |

`modifications` is a **nested parameter list**, containing:

* orderID
* updated fields

---

## Parameter Value Sizes

| Parameter | Size    |
| --------- | ------- |
| userID    | 8 bytes |
| tradeID   | 8 bytes |
| quantity  | 8 bytes |
| price     | 4 bytes |
| type      | 1 byte  |
| side      | 1 byte  |

