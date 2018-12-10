# msem

Provides a convenient interface for creating and controlling shared
semaphores in the filesystem. There are only three primary operations,
lock, unlock, and relax (unlock all).

Requires System V UNIX semaphores, which are not POSIX-compliant but
which ship with the Linux kernel.

## Rationale
A common problem in web development is how to provide synchronized interactive
behavior to a number of distributed clients in real-time. Websockets and `epoll`
are solutions, however the solution given here is ideal for long-polling with
with limited server resources.

System V semaphores can support a form of kernel queue, in which locked
processes are put to sleep until they are popped from the queue. This allows
subscribers to an event feed to remain at the server until a timeout occurs,
or until a controller process (typically another web request which alters
state) flushes the semaphore and unwinds the queue.

This provides a simple way to coordinate clients in a long-poll based solution,
and has been validated in years of production use.

### Caution
While System V semaphores are commonly available, note that they are *not*
POSIX-compliant, and that there may be better solutions depending on your
operating requirements.

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
