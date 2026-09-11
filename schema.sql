-- ============================================================
-- E-commerce backend schema + seed data + checkout procedure
-- Run this ONCE, manually, from your mysql> prompt:
--     source schema.sql;
-- (or: mysql -u root -p < schema.sql   from a plain terminal)
-- ============================================================

CREATE DATABASE IF NOT EXISTS ecommerce;
USE ecommerce;

DROP TABLE IF EXISTS order_items;
DROP TABLE IF EXISTS orders;
DROP TABLE IF EXISTS products;
DROP TABLE IF EXISTS users;

CREATE TABLE users (
    id            INT AUTO_INCREMENT PRIMARY KEY,
    name          VARCHAR(100) NOT NULL,
    email         VARCHAR(100) UNIQUE NOT NULL,
    salt          VARCHAR(32) NOT NULL,   -- random per-user salt, hex-encoded
    password_hash VARCHAR(64) NOT NULL    -- SHA-256(salt + password), hex-encoded
);

CREATE TABLE products (
    id    INT AUTO_INCREMENT PRIMARY KEY,
    name  VARCHAR(100) NOT NULL,
    price DECIMAL(10,2) NOT NULL,
    stock INT NOT NULL DEFAULT 0
);

CREATE TABLE orders (
    id         INT AUTO_INCREMENT PRIMARY KEY,
    user_id    INT NOT NULL,
    total      DECIMAL(10,2) NOT NULL,
    status     VARCHAR(20) DEFAULT 'PLACED',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id)
);

CREATE TABLE order_items (
    id                 INT AUTO_INCREMENT PRIMARY KEY,
    order_id           INT NOT NULL,
    product_id         INT NOT NULL,
    quantity           INT NOT NULL,
    price_at_purchase  DECIMAL(10,2) NOT NULL,
    FOREIGN KEY (order_id) REFERENCES orders(id),
    FOREIGN KEY (product_id) REFERENCES products(id)
);

INSERT INTO products (name, price, stock) VALUES
('Wireless Mouse',        599.00, 50),
('Mechanical Keyboard',  2499.00, 30),
('USB-C Cable',           199.00, 100),
('Laptop Stand',          999.00, 20),
('Webcam 1080p',         1799.00, 15);

-- ------------------------------------------------------------
-- checkout_order: does the whole "buy 1 product" flow as ONE
-- atomic transaction, so it's safe even if two customers try
-- to buy the last unit of something at the same time.
--
-- SELECT ... FOR UPDATE locks the product row until this
-- transaction commits or rolls back, so a second, concurrent
-- call has to wait its turn instead of reading stale stock and
-- overselling.
-- ------------------------------------------------------------
DELIMITER //

DROP PROCEDURE IF EXISTS checkout_order //

CREATE PROCEDURE checkout_order(
    IN  p_user_id    INT,
    IN  p_product_id INT,
    IN  p_quantity   INT,
    OUT p_status     VARCHAR(20)
)
BEGIN
    DECLARE current_stock  INT DEFAULT NULL;
    DECLARE product_price  DECIMAL(10,2);
    DECLARE new_order_id   INT;

    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;
        SET p_status = 'DB_ERROR';
    END;

    START TRANSACTION;

    SELECT stock, price INTO current_stock, product_price
    FROM products
    WHERE id = p_product_id
    FOR UPDATE;

    IF current_stock IS NULL THEN
        SET p_status = 'PRODUCT_NOT_FOUND';
        ROLLBACK;
    ELSEIF current_stock < p_quantity THEN
        SET p_status = 'INSUFFICIENT_STOCK';
        ROLLBACK;
    ELSE
        UPDATE products
        SET stock = stock - p_quantity
        WHERE id = p_product_id;

        INSERT INTO orders (user_id, total, status)
        VALUES (p_user_id, product_price * p_quantity, 'PLACED');
        SET new_order_id = LAST_INSERT_ID();

        INSERT INTO order_items (order_id, product_id, quantity, price_at_purchase)
        VALUES (new_order_id, p_product_id, p_quantity, product_price);

        COMMIT;
        SET p_status = 'SUCCESS';
    END IF;
END //

DELIMITER ;
