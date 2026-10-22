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

Needs CMake 3.16 or newer, a C++17 compiler, and the libpq development headers, which CMake locates with `find_package(PostgreSQL REQUIRED)`.

```bash
createdb shelflock
cmake -S . -B build
cmake --build build
# from the repo root so sql/ paths resolve
./build/shelflock
```

Create the database only. On start the program applies `sql/schema.sql` and then `sql/seed.sql` itself, so there is no manual `psql -f` step.

Windows: install PostgreSQL, then set `PostgreSQL_ROOT` before CMake. Override the database with:

```bash
set SHELFLOCK_DSN=host=localhost dbname=shelflock user=postgres password=YOUR_PASSWORD
```

An empty `SHELFLOCK_DSN` is ignored, and the program keeps `dbname=shelflock`. Do not commit passwords.

## Design notes

### Why a hold exists
Checkout is not instant. While the buyer pays, stock must not be sold twice. A **hold** lowers available quantity, records a deadline, and either becomes a paid order or expires and restores stock.

### Concurrency
- Stock rows are locked with `SELECT ... FOR UPDATE` inside a transaction so two reserve calls cannot oversell the same SKU.
- An expiry worker watches a min-heap of hold deadlines and releases unpaid holds when their TTL ends.
- Logging and metrics are shared across reserve / pay / expire paths so you can see reserved vs committed vs expired counts.

### Main pieces
| Piece | Role |
| --- | --- |
| `InventoryRepository` | Postgres access for products, stock, holds, orders |
| `FulfillmentStrategy` | Choose store vs warehouse when reserving |
| `ReservationService` | Orchestrates reserve, pay, expire, restock hints |
| `HoldHeap` | Deadline ordering for the expiry worker |
| `OrderState` | Simple state names for order lifecycle |

### Honest limits
This is a teaching / portfolio system: one process, one database, grocery-style seed data. It is not a multi-region inventory platform.
