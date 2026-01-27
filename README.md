# Time-Travelling File System

## Overview

This assignment implements a simple in memory version control system with support for file creation, editing, snapshots, rollbacks, and history tracking.

Additionally, it provides system-wide analytics such as listing the most recently modified files and the largest version trees.

The program is interactive — it accepts commands line by line, processes them, and immediately outputs the result.

---

## Design Choices and Assumptions

### Command Format

- Commands must be in **UPPERCASE** (for example: `CREATE`, `READ`).
- `CREATE` works but `Create` or `create` do not.
- Each command must be on a separate line.
- Empty input lines are ignored, and execution continues from the next line.
- The command `EXIT` terminates the program.
- The command `HELP` prints the detailed usage instructions.
- If an unknown command is entered, an error message is printed and execution continues.

### Arguments and Input Handling

- Filenames cannot contain spaces.
- Content strings (for `INSERT` / `UPDATE`) and message strings (for `SNAPSHOT`) may contain spaces.
- If a required argument is missing, an error message is printed and the program moves to the next line of input.
- The program never halts on invalid input and always continues processing the next input line.

---

## Versioning Semantics

- Every file starts with a root version (ID = 0), which is an initial snapshot.
- Non-snapshotted versions are mutable.
- Snapshotted versions are immutable.
- For `INSERT` / `UPDATE` commands:
  - If the active version is a snapshot, a new version is created as a child of the active version and set as the active version.
- If `SNAPSHOT` is called on an active version that is already a snapshot:
  - A new version is created with the same content and set as a snapshot.
- `SNAPSHOT` without a message is not allowed and prints an error.

---

## Rollback and History

- When no version ID is provided after `ROLLBACK`, it rolls back to the parent of the current active version.
- `ROLLBACK` at the root prints an error.
- `ROLLBACK` with an invalid version ID prints an error.
- `ROLLBACK` sets the active version pointer to a version that is not a snapshot.
- `HISTORY` prints all snapshots on the path from the current active version back to the root, in chronological order.

---

## System-Wide Analytics

- Only `CREATE`, `UPDATE`, and `INSERT` are considered file modifications.
- `SNAPSHOT` is not considered a file modification.
- If the argument `[num]` of system-wide analytic commands is greater than the number of files:
  - All available files are printed in descending order without an error.
- It is assumed that the user enters an argument less than or equal to the number of files in the filesystem.

---

## Data Structures and Implementation

- STL `vector` is used as a container in the heap implementation.
- STL `string` is used for file names, file content, and messages.
- STL `list` is used for handling collisions via chaining in the hash map implementation.
- Implemented data structures:
  - Max Heap
  - Tree
  - Hash Map
- `<ctime>` is used for timestamps.
- `<sstream>`, `<utility>`, `<functional>`, and `<algorithm>` are used for internal working of the program.

---

## Internal Design

- The `File` class maintains a tree of versions (`TreeNode`).
- An active version pointer is always maintained.
- `FileSystem` is declared as a friend class inside `File`:
  - Its private members `total_versions`, `name`, and `last_modified` are accessed by `FileSystem`.

---

