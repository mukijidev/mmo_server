# MMO Game Server

A C++ MMORPG server built from scratch on Windows IOCP — custom network library, JPS pathfinding, and a sector-based interest-management system.

> Personal MMO project: **server (this)** · [client](https://github.com/mukijidev/mmo_client) · [load-test dummy](https://github.com/mukijidev/mmo_dummy)

## Architecture

Three independent servers, each with a concurrency model chosen for its workload:

| Server | Role | Concurrency model |
|--------|------|-------------------|
| LoginServer | Authentication, one-time session-token issuance | IOCP workers handle auth directly |
| ChattingServer | Sector-based chat broadcast | Single consumer thread |
| GameServer | Game logic, characters / monsters / fields | 1 thread per field (sharding) |

## Highlights

- **Custom IOCP network library (CNetServer)** — Accept / Worker / Release threads, 20,000 preallocated sessions, race-free disconnect via a packed IoCount + ReleaseFlag CAS.
- **Spatial partitioning (AOI)** — a 12,000 × 12,000 map split into a 15 × 15 grid of 800 × 800 sectors; updates broadcast only to the 3 × 3 neighborhood.
- **JPS pathfinding** — Jump Point Search running asynchronously, off the game loop.
- **Session-token auth** — LoginServer issues a one-time token stored in Redis; GameServer verifies it and deletes it on entry.
- **Async DB** — a dedicated DB thread per field, so MySQL calls never stall the game loop.

## Tech Stack

C++ (MSVC v143) · Windows IOCP · MySQL 8.0 · Redis (cpp_redis) · Visual Studio 2022
