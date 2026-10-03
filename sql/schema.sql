CREATE TABLE IF NOT EXISTS products (
    sku TEXT PRIMARY KEY,
    name TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS stock (
    sku TEXT NOT NULL REFERENCES products(sku),
    location_id TEXT NOT NULL,
    location_type TEXT NOT NULL CHECK (location_type IN ('store', 'warehouse')),
    qty INTEGER NOT NULL CHECK (qty >= 0),
    PRIMARY KEY (sku, location_id)
);

CREATE INDEX IF NOT EXISTS idx_stock_sku ON stock (sku);

CREATE TABLE IF NOT EXISTS holds (
    hold_id BIGSERIAL PRIMARY KEY,
    sku TEXT NOT NULL,
    location_id TEXT NOT NULL,
    qty INTEGER NOT NULL CHECK (qty > 0),
    status TEXT NOT NULL CHECK (status IN ('reserved', 'committed', 'released', 'expired')),
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    expires_at TIMESTAMPTZ NOT NULL
);

CREATE INDEX IF NOT EXISTS idx_holds_status_expires ON holds (status, expires_at);

CREATE TABLE IF NOT EXISTS orders (
    order_id BIGSERIAL PRIMARY KEY,
    hold_id BIGINT REFERENCES holds (hold_id),
    sku TEXT NOT NULL,
    qty INTEGER NOT NULL,
    state TEXT NOT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE INDEX IF NOT EXISTS idx_orders_sku_state ON orders (sku, state);
