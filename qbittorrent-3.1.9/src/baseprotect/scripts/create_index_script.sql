CREATE INDEX [hash_index] On [hits] ([hash] Collate NOCASE ASC); CREATE INDEX [date_index] On [hits] ([year, month, day, hour, minute] Collate NOCASE ASC); CREATE INDEX [filename_index] On [hits] ([filename] Collate NOCASE ASC);;