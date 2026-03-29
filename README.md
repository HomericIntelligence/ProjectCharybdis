# ProjectCharybdis

Chaos and resilience testing for the HomericIntelligence distributed agent mesh.

Part of [Odysseus](https://github.com/HomericIntelligence/Odysseus) — the HomericIntelligence meta-repo.

## Role

Charybdis injects faults via Agamemnon's `/v1/chaos/*` REST API and validates that the
HomericIntelligence mesh survives failures gracefully: network partitions, latency, service
kills, and queue starvation.

## Building

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

## License

MIT
