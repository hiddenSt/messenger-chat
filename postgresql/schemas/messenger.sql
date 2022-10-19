DROP SCHEMA IF EXISTS messenger_schema CASCADE;

CREATE SCHEMA IF NOT EXISTS messenger_schema;

CREATE TABLE IF NOT EXISTS messenger_schema.chat (
    sender_id INTEGER UNSIGNED NOT NULL,
    receiver_id INTEGER UNSIGNED NOT NULL,
    timepoint TIMESTAMP NOT NULL,
    message TEXT NOT NULL
);
