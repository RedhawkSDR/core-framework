# File System

The File System interface defines CORBA operations that exists to provide a runtime abstraction of an OS' real file system. It gives REDHAWK the ability to have a single interface for reading and writing individual files within a file system regardless of the underlying implementation in the OS.

Files that are stored on the File System may be either plain files or directories.

Characters and symbols that are valid in directories and file names consist of:

  - Upper and lowercase letters
  - Numbers
  - "\_" (underscore)
  - "-" (hyphen)
  - "." (period)
      - The file names "." and ".." are invalid in the context of a File System.

Path names are structured according to the POSIX specification where the "/" (forward slash) is a valid character that acts as the separator between directories.

Additionally, the File System interface provides implementation of many standard functions such as:

  - `remove()`
  - `copy()`
  - `exists()`
  - `list()`
  - `create()`
  - `open()`
  - `mkdir()`
  - `rmdir()`
  - `query()`

File System attached to the <abbr title="See Glossary.">Domain Manager</abbr> mounts with `$SDRROOT/dom` as the root. Each <abbr title="See Glossary.">Device Manager</abbr> mounts a File System with `$SDRROOT/dev` as the root.

### File Manager

The <abbr title="See Glossary.">File Manager</abbr> exists to manage multiple distributed file systems. This interface allows these file systems to act as a single entity, though they may span multiple physical file systems on different pieces of hardware. This provides for a distributed file system that functions as a single file system across multiple Device Managers and the Domain Manager.

The file manager inherits the Interface Description Language (IDL) interface of a file system. It then delegates tasks from the Core Framework (CF) based off of the path names to the correct mounted file system, depending on where that file system is mounted. It is also responsible for copying the appropriate <abbr title="See Glossary.">component</abbr> files into the specific Device Manager's file system as <abbr title="See Glossary.">applications</abbr> are installed and launched.
