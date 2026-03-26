# xv6-Auth Login & User Management

## Overview
This project extends the inherently single-user xv6 operating system to support multiple users, secure login authentication, and user privilege management. It includes a password hashing utility to ensure user credentials are securely stored.

## Team Members
* CS23B1044 - Nimalan R
* CS23B1065 - Nithyanand R
* CS23B1075 - K.V.S. Jayadheer

## Implemented Functionalities

### 1. Kernel & Process Management
* **Process Structure Modification**: Modified `struct proc` to include a `uid` integer field to track user ownership. The Root user is assigned UID 0, and standard users are assigned UIDs > 0.
* **Process Allocation & Inheritance**: Updated `proc.c` so that newly allocated processes default to root (`p->uid = 0`), and child processes correctly inherit their parent's UID during a `fork()` (`np->uid = curproc->uid`). The first process initialized by `userinit` is explicitly set to UID 0.
* **Boot Sequence Alteration**: Modified the system initialization loop in `init.c`. Instead of dropping directly into a shell (`sh`), `init` now forks and executes the `login` program, forcing system-wide authentication on boot.

### 2. File System & Persistence
* **Persistent Credentials Database**: User credentials are stored in a `users` file. Modified the xv6 file system maker (`mkfs.c`) to read this file from the host OS and burn it directly into the root directory of the file system image (`fs.img`) during compilation.

### 3. Custom System Calls
Introduced new system calls to manage user privileges:
* **`sys_getuid` (SYS_getuid 23)**: Returns the `uid` of the current process.
* **`sys_setuid` (SYS_setuid 22)**: Reads an integer argument and sets the `uid` of the current process, validating that the UID is within an acceptable range (0 to 65535).

### 4. User-Space Utilities & Commands
We implemented several new user-level programs to interact with the authentication system:

* **`login`**: The first program run by `init`. It prompts the user for a username and password, validates the hashed credentials against the `users` database file, calls `setuid`, and then executes the shell (`sh`).
* **`adduser`**: Allows the creation of new users by appending credentials to the `users` file. 
  * *Syntax*: `adduser [username] [password] [uid]`
  * *Security*: Includes kernel-level permission checks. Only the root user (UID 0) can execute it successfully; standard users will receive a "Permission Denied" error.
* **`su` (Switch User)**: Allows a currently logged-in user to switch to another profile (such as root). It verifies the target user's password and changes the active UID accordingly.
* **`whoami`**: Prints the currently logged-in user's name and their associated UID (e.g., `Name: root UID: 0`).
* **`hashpw`**: A standalone C program run on the host machine to generate the initial `users` database file. It securely hashes passwords so they cannot be read in plaintext by other users on the system.

## Building and Running

**1. Generate Password Database**
First, compile the helper tool on your host machine to create the initial `users` file:
```bash
gcc -o hashpw hashpw.c
./hashpw root root 0 > users