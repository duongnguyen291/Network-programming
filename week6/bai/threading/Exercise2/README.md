# Exercise 2 - Authentication Server with Threading

This application implements a TCP-based authentication system with user login/logout functionality using **threading technique** (pthread). The server supports multiple concurrent sessions per account and includes account locking after failed login attempts.


## Requirements

- GCC compiler
- POSIX threads library (pthread)
- Linux/Unix environment

## Compilation

Use the provided Makefile:

```bash
make
```

This creates two executables:
- `server` - Authentication server
- `client` - Client application

To clean:

```bash
make clean
```

## Account File Format

Accounts are stored in `account.txt` with the following format:

```
Username Password Status
```

Where:
- **Username**: User's login name
- **Password**: User's password (plain text for demonstration)
- **Status**: `1` = unlocked, `0` = locked

### Sample account.txt

```
Username Password Status
admin admin123 1
user1 pass1 1
user2 pass2 1
test test123 1
locked locked123 0
alice alice2024 1
bob bob2024 1
charlie charlie2024 1
```

## Running the Application

### Step 1: Start the Server

```bash
./server <Port_Number>
```

**Example:**

```bash
./server 5500
```

**Expected Output:**

```
Loaded 8 accounts from account.txt
Authentication Server started on port 5500
Using threading technique for multiple clients
```

### Step 2: Start the Client(s)

```bash
./client <IP_Address> <Port_Number>
```

**Example:**

```bash
./client 127.0.0.1 5500
```

or for remote server:

```bash
./client 10.0.0.1 5500
```

**Expected Output:**

```
Connected to server 127.0.0.1:5500
Welcome to Authentication Server
```

## Usage Examples

### Example 1: Successful Login

**Client Output:**

```
=== LOGIN ===
Username: admin
Password: admin123

✓ Login successful!

=== MENU ===
1. Show my sessions
2. Show online users
3. Logout
Choice: 
```

### Example 2: Wrong Password

**Client Output:**

```
=== LOGIN ===
Username: user1
Password: wrongpass

✗ Wrong password. Please try again.

=== LOGIN ===
Username: 
```

### Example 3: Account Locked

After 5 failed attempts, the account gets locked:

**Client Output:**

```
=== LOGIN ===
Username: user1
Password: wrongpass

✗ Account is locked due to multiple failed login attempts.
Please contact administrator.
```

**Server Output:**

```
Login attempt for locked account 'user1' from 127.0.0.1:45678
```

The account status in `account.txt` will be updated to `0`.

### Example 4: Show My Sessions

When logged in from multiple terminals:

**Client Output:**

```
=== MENU ===
1. Show my sessions
2. Show online users
3. Logout
Choice: 1

=== MY ACTIVE SESSIONS ===
1. IP: 127.0.0.1:45678, Connected: 2025-11-26 10:30:15
2. IP: 127.0.0.1:45690, Connected: 2025-11-26 10:31:42
3. IP: 192.168.1.100:52341, Connected: 2025-11-26 10:35:20
```

### Example 5: Show Online Users

**Client Output:**

```
=== MENU ===
1. Show my sessions
2. Show online users
3. Logout
Choice: 2

=== ONLINE USERS ===
1. admin
2. user1
3. alice
4. bob
```

### Example 6: Logout

**Client Output:**

```
=== MENU ===
1. Show my sessions
2. Show online users
3. Logout
Choice: 3

✓ Logged out successfully. Goodbye!
```

## Multiple Sessions Demo

The server supports multiple sessions per account. Here's a demonstration:

**Terminal 1 (Server):**

```bash
$ ./server 5500
Loaded 8 accounts from account.txt
Authentication Server started on port 5500
Using threading technique for multiple clients
Client connected from 127.0.0.1:45678
User 'admin' logged in from 127.0.0.1:45678
Client connected from 127.0.0.1:45690
User 'admin' logged in from 127.0.0.1:45690
Client connected from 127.0.0.1:45701
User 'user1' logged in from 127.0.0.1:45701
```

**Terminal 2 (Client 1 - admin):**

```bash
$ ./client 127.0.0.1 5500
Connected to server 127.0.0.1:5500
Welcome to Authentication Server

=== LOGIN ===
Username: admin
Password: admin123

✓ Login successful!

=== MENU ===
1. Show my sessions
2. Show online users
3. Logout
Choice: 1

=== MY ACTIVE SESSIONS ===
1. IP: 127.0.0.1:45678, Connected: 2025-11-26 10:30:15
2. IP: 127.0.0.1:45690, Connected: 2025-11-26 10:31:42
```

**Terminal 3 (Client 2 - admin):**

```bash
$ ./client 127.0.0.1 5500
Connected to server 127.0.0.1:5500
Welcome to Authentication Server

=== LOGIN ===
Username: admin
Password: admin123

✓ Login successful!

=== MENU ===
1. Show my sessions
2. Show online users
3. Logout
Choice: 2

=== ONLINE USERS ===
1. admin
2. user1
```

**Terminal 4 (Client 3 - user1):**

```bash
$ ./client 127.0.0.1 5500
Connected to server 127.0.0.1:5500
Welcome to Authentication Server

=== LOGIN ===
Username: user1
Password: pass1

✓ Login successful!
```

## Technical Details

### Server Implementation

- **Threading**: Uses `pthread_create()` with detached threads for each client
- **Mutex Locks**: Protects shared account and session data with `pthread_mutex_t`
- **Account Management**: Tracks failed login attempts and automatically locks accounts
- **Session Tracking**: Maintains active session information (username, IP, port, connection time)
- **Persistent Storage**: Saves account status changes to `account.txt`

### Client Implementation

- **Command Line Arguments**: IP address and port from command line
- **Connection Error Handling**: Shows clear error messages if server is unreachable
- **Interactive Menu**: User-friendly menu for post-login operations
- **Multiple Terminal Support**: Each client terminal can login independently

### Security Features

1. **Account Locking**: After 5 consecutive failed login attempts, account is locked
2. **Single Login per Terminal**: Each client terminal can only login one account at a time
3. **Multiple Sessions per Account**: Same account can login from different terminals
4. **Failed Attempt Counter**: Resets to 0 upon successful login

## Error Handling

### Server Not Running

```
connect: Connection refused
Failed to connect to server 127.0.0.1:5500
Make sure the server is running!
```

**Solution:** Start the server first.

### Invalid Port Number

```
Invalid port number
```

**Solution:** Use a valid port (1-65535).

### Port Already in Use

```
bind: Address already in use
```

**Solution:** Stop other processes using the port or choose a different port.

### Account File Not Found

```
Warning: account.txt not found. Creating empty account list.
```

**Solution:** Create `account.txt` with the correct format.

## Protocol Design

The server-client communication follows this protocol:

1. **Welcome Phase**: Server sends welcome message
2. **Login Phase**: 
   - Server sends `LOGIN`
   - Client sends username and password
   - Server responds: `OK`, `LOCKED`, `WRONG_PASSWORD`, or `USER_NOT_FOUND`
3. **Menu Phase** (if logged in):
   - Server sends `MENU`
   - Client sends choice (1, 2, or 3)
   - Server sends response data
4. **Logout Phase**:
   - Server sends `LOGOUT`
   - Connection closes

## File Structure

```
Exercise2/
├── server.c        # Authentication server with threading
├── client.c        # Client application
├── account.txt     # User account database
├── Makefile        # Build configuration
└── README.md       # This file
```

## Testing Checklist

- [x] Single client login/logout
- [x] Multiple clients connecting simultaneously
- [x] Same account logging in from multiple terminals
- [x] Wrong password handling
- [x] Account locking after 5 failed attempts
- [x] Viewing active sessions
- [x] Viewing online users
- [x] Server handles client disconnection gracefully
- [x] Thread safety with mutex locks

## Notes

- The server will continue running until manually stopped (Ctrl+C)
- Each client connection runs in a separate thread
- Account information is synchronized across all threads using mutex locks
- Failed login attempts are tracked per account (not per session)
- When an account is locked, it's immediately saved to `account.txt`
- Session information is kept in memory and cleared when client disconnects

## Troubleshooting

### Client shows "User not found"
- Check the username spelling
- Verify the user exists in `account.txt`

### Can't view sessions or online users
- Make sure you're logged in first
- Check server logs for connection issues

### Account automatically locked
- 5 failed password attempts will lock the account
- Edit `account.txt` and change status from `0` to `1` to unlock
- Restart the server after editing `account.txt`

## Future Enhancements

Possible improvements:
- Password encryption (currently plain text)
- Database storage instead of text file
- Admin interface to unlock accounts remotely
- Session timeout functionality
- IP-based access control
- Logging system for audit trail
