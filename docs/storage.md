## Page Layout
Each page in the storage system has a fixed size (4 KB by default) and is divided into several sections to efficiently manage and store data. The main components of a page layout include:
- **Page Header**: Contains metadata about the page, such as page ID, free space offset, and number of slots currently present in the page.
- **Slot Directory**: A dynamic array of slot pointers that reference the actual data records(contents of a row) stored in the page.
- **Free Space**: The contiguous unused region between the end of the stored rows and the beginning of the slot directory. New rows are appended at the start of the free space, while new slot entries are appended at the end of the page.

the layout looks something like this:

```
+---------------------+
|      Page Header    |
+---------------------+
|      Stored Rows    |
+---------------------+
|      Free Space     |
+---------------------+
|     Slot Directory  |
+---------------------+
```

## Slot Directory
The Slot Directory is an array of slot entries stored at the end of the page. Each slot entry contains the byte offset and length of a row stored within the page.The slot directory enables stable identification of rows within a page, even if rows are moved internally during future operations such as updates or compaction. Slot IDs are assigned incrementally at insertion time and remain stable throughout the lifetime of a row.
## Row Identifier (RID)
The Row Identifier (RID) is a unique identifier for each row in a table in the database. It typically consists of two components:
pageID: The identifier of the page where the row is stored.
slotID: The index of the slot within the page that references the row.

## Insert Path
The insertion of a row follows these steps:

1. The row is first serialized into a binary format.
2. The storage engine selects the last page of the table as the insertion target.
3. If the page does not have enough free space to store the row and a new slot entry, a new page is created.
4. The serialized row is written at the current free space offset within the page.
5. A new slot entry is appended to the slot directory, recording the row’s offset and length.
6. The page header is updated to reflect the new free space offset and slot count.
7. The insert operation returns an RID consisting of the page ID and slot ID.

## Design Invariants

The storage engine maintains the following invariants:

- Pages have a fixed size and are addressed by page ID.
- Rows are stored contiguously within a page and never overlap.
- Slot IDs are stable once assigned. (changes will come in future phases)
- Slot directory entries grow from the end of the page backward, while row data grows forward from the page header.
- The free space region always lies between stored rows and the slot directory.
- A row’s physical location is identified only by its RID, not by byte offsets.
- Pages are append-only in the current phase; deletes, updates, and compaction are not yet supported.
- A valid RID always refers to an existing page and slot entry.

## RID based lookup

The storage engine supports direct row access using a Row Identifier (RID).
An RID uniquely identifies a row using the page ID and slot ID.

Given an RID, the tool performs the following steps:
1. Locate the target page using the page ID from the RID.
2. Access the corresponding slot entry
3. Read the row data using the slot's offset and length.
4. Deserialize the row into column values

RID based lookups avoids full table scans and provides constant time access to rows