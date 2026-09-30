# ShelfLock

Concurrent C++ and PostgreSQL inventory system that manages catalog, stock holds, and checkout.

A hold reserves stock during checkout. Unpaid holds expire and return the quantity. Paid holds become orders.

## Stack

- C++17
- PostgreSQL (`SELECT ... FOR UPDATE`, transactions, indexes)
- CMake + libpq

## Flow

1. **Reserve** — pick store or warehouse, lock the stock row, reduce qty, insert a hold.
2. **Pay** — mark the hold committed and write a paid order.
3. **Release / expire** — a worker thread uses a min-heap of hold deadlines. If nobody pays in time, stock is put back.

## Layout

- `sql/schema.sql` — products, stock, holds, orders
- `sql/seed.sql` — sample grocery SKUs
- `include/` / `src/` — repository, store-vs-warehouse strategy, order-state factory, logging, metrics, demand restock

## Run

```bash
createdb shelflock
cmake -S . -B build
cmake --build build
# from the repo root so sql/ paths resolve
./build/shelflock
```

Windows: install PostgreSQL, then set `PostgreSQL_ROOT` before CMake. Override the database with:

```bash
set SHELFLOCK_DSN=host=localhost dbname=shelflock user=postgres password=YOUR_PASSWORD
```

Do not commit passwords.
