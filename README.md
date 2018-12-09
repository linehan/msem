# msem

## Overview
Provides a convenient interface for creating and controlling shared
semaphores in the filesystem. There are only three primary operations,
lock, unlock, and relax (unlock all).

Requires System V UNIX semaphores, which are not POSIX-compliant but
which ship with the Linux kernel.

## Semaphore files
A semaphore file can locate multiple semaphores, each referenced
by a unique identifier (tag), usually an ASCII character. If the
file does not exist, it will be created. If the file does exist,
and does not contain a semaphore with the specified tag, a new
semaphore will be created inside the file.

Semaphore files are created and stored in `/tmp/sem_*`.

## Usage:
        SYNOPSIS
        msem

        -c, --create <path> [uid] [initial_value]
                Create a new semaphore

        -d, --delete <path> [uid]
                Delete an existing semaphore

        -l, -p, --lock <path> [uid] [timeout]
                Lock a semaphore by decrementing its value by 1

        -u, -v, --unlock <path> [uid]
                Unlock a semaphore by incrementing its value by 1.
                The next process in the waiting queue will be able
                to proceed.

        -u+, -v+, --relax <path> [uid]
                Relax a semaphore. Any processes which have locked
                the semaphore will be released, as though the semaphore
                had been incremented for all of them,

        -f --follow <path> [uid]
                Continuously print the status of a semaphore to stdout
                (similar to tail -f)


        FILES
                /tmp/sem_*

