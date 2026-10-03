INSERT INTO products (sku, name) VALUES
    ('MILK-1L', 'Toned Milk 1L'),
    ('BREAD-W', 'White Bread'),
    ('EGGS-12', 'Eggs Dozen')
ON CONFLICT (sku) DO NOTHING;

INSERT INTO stock (sku, location_id, location_type, qty) VALUES
    ('MILK-1L', 'store-wadsa', 'store', 8),
    ('MILK-1L', 'wh-nagpur', 'warehouse', 40),
    ('BREAD-W', 'store-wadsa', 'store', 12),
    ('BREAD-W', 'wh-nagpur', 'warehouse', 60),
    ('EGGS-12', 'store-wadsa', 'store', 6),
    ('EGGS-12', 'wh-nagpur', 'warehouse', 24)
ON CONFLICT (sku, location_id) DO NOTHING;
