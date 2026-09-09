# Simple E-Commerce Backend (C++ + MySQL)

A console app that lets a user register, log in, browse products, buy a
product, and view their order history — backed by a real MySQL database.

Checkout runs as one atomic transaction inside a MySQL stored procedure
(`checkout_order` in `schema.sql`), using `SELECT ... FOR UPDATE` to lock
the product row, so it stays correct even if two people try to buy the
last unit of something at the same time.

The C++ side talks to MySQL by running the `mysql` command-line client as
a subprocess (via `popen`) rather than linking a connector library — see
`db.cpp`.

## Files

- `schema.sql` — creates the database, tables, seed products, and the
  `checkout_order` stored procedure. Run this once, manually.
- `db.hpp` / `db.cpp` — the subprocess-based query runner (`runQuery`,
  `parseRows`, `escapeSql`).
- `models.hpp` — `Product` and `User` structs.
- `main.cpp` — the menu-driven app itself.
- `db_config.example.cnf` — template for your MySQL credentials.
- `.vscode/tasks.json` — a VS Code build task (Ctrl+Shift+B).

## One-time setup

**1. Confirm you have a C++ compiler.**
Open a terminal in VS Code (`` Ctrl+` ``) and run:

```
g++ --version
```

If that fails with "not recognized," you need MinGW-w64 installed first
(search "Using GCC with MinGW" in the VS Code C/C++ docs, or install via
MSYS2). Come back once `g++ --version` prints something.

**2. Confirm `mysql` is on your PATH.**
Same terminal:

```
mysql --version
```

If that fails, find where MySQL was installed (commonly
`C:\Program Files\MySQL\MySQL Server 8.0\bin`) and add that folder to
your Windows PATH (search "Edit the system environment variables" in the
Start menu → Environment Variables → edit `Path` → add the folder →
restart VS Code).

**3. Create the database.**
From a plain terminal (not your app), run:

```
mysql -u root -p < schema.sql
```

Enter your MySQL password when prompted. This creates the `ecommerce`
database, its tables, 5 sample products, and the `checkout_order`
procedure.

**4. Set your credentials.**
Copy `db_config.example.cnf` to a new file named `db_config.cnf` in the
same folder, and edit it:

```
[client]
user=root
password=YOUR_ACTUAL_PASSWORD
```

`db_config.cnf` is in `.gitignore` so it won't get committed if you push
this to GitHub — don't remove that line, since it holds a real password.

## Building and running

**Build:** press `Ctrl+Shift+B` in VS Code (uses the task in
`.vscode/tasks.json`), or from the terminal:

```
g++ -std=c++17 main.cpp db.cpp -o app.exe
```

**Run:** from the VS Code integrated terminal, with the project folder as
your current directory (important — the app looks for `db_config.cnf` in
the current folder):

```
.\app.exe
```

You should see the menu. Register a user, log in, browse products, and
buy one to see the transaction succeed. Try buying more of something than
is in stock to see `INSUFFICIENT_STOCK` handled cleanly instead of
crashing or corrupting data.

## What to extend next (optional)

- Add password login (currently email-only, no password check — fine for
  a portfolio project, worth naming as a known simplification).
- Add an admin menu to add/restock products.
- Simulate the race condition on purpose: run two copies of the app at
  once and have both try to buy the last unit of the same product, to see
  the row-locking actually prevent overselling.
- Swap the subprocess approach for a real connector library once you're
  comfortable with the project logic, and compare the two.
