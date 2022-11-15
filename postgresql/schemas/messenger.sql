DROP SCHEMA IF EXISTS messenger_schema CASCADE;

CREATE SCHEMA IF NOT EXISTS messenger_schema;

CREATE TABLE IF NOT EXISTS messenger_schema.chat (
    id SERIAL PRIMARY KEY,
    sender_id INTEGER NOT NULL,
    receiver_id INTEGER NOT NULL,
    timepoint TIMESTAMP NOT NULL,
    message TEXT NOT NULL
);
