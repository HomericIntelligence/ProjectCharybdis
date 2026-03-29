# CLAUDE.md — ProjectCharybdis

## Project Overview

ProjectCharybdis is the chaos and resilience testing service for the HomericIntelligence
distributed agent mesh.

**Role:** Injects faults into the live system via Agamemnon's chaos API, then validates
that the system recovers gracefully. Proves the HomericIntelligence mesh is resilient.

## What Charybdis Tests

- **Network partitions** — split Tailscale nodes, verify NATS JetStream handles reconnects
- **Latency injection** — high-latency links, verify backpressure and rate limiting hold
- **Service kills** — kill Agamemnon/Nestor/Telemachy processes, verify restart/recovery
- **Queue starvation** — exhaust myrmidon pull consumers, verify no message loss
- **Cascade failures** — trigger failure in one component, verify blast radius is contained

## Architecture

Charybdis talks to Agamemnon's chaos API:
- `POST /v1/chaos/network-partition` — partition nodes in the Tailscale mesh
- `POST /v1/chaos/latency` — inject latency on a node
- `POST /v1/chaos/kill` — kill a named service
- `POST /v1/chaos/queue-starve` — stall all pull consumers on a subject
- `DELETE /v1/chaos/*` — remove injected faults

Resilience metrics are reported via `hi.logs.>` → Argus/Prometheus.

## Development Guidelines

- Language: C++20 exclusively
- Build: `cmake --preset debug` / `cmake --build --preset debug`
- Test: `ctest --preset debug`
- All tool invocations via `scripts/` wrappers
- Never `--no-verify`. Never merge with red CI.
