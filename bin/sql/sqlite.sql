CREATE TABLE IF NOT EXISTS server_data
(
    field TEXT NOT NULL UNIQUE,
    content TEXT NOT NULL,
    last_time INTEGER NOT NULL
);