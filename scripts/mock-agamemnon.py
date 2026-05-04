#!/usr/bin/env python3
"""
Minimal Agamemnon stub for integration tests in CI.

Implements just enough of the Agamemnon REST API so that the chaos/resilience
integration tests pass without a live HomericIntelligence mesh:

  GET  /v1/health                  → {"status":"ok"}
  POST /v1/chaos/<type>            → creates a fault record
  GET  /v1/chaos                   → lists active faults
  DEL  /v1/chaos/<id>              → removes a fault
  POST /v1/teams                   → creates a team record
  POST /v1/agents                  → creates an agent record
  POST /v1/teams/<id>/tasks        → creates a task (pending)
  GET  /v1/tasks                   → lists tasks (immediately marks them completed)
  POST /v1/chaos/test-empty        → accepts any body, returns 400 on empty JSON
  POST /v1/agents (malformed)      → returns 400 on bad content-type or unparseable body

All state is in-process (no persistence); the stub is single-threaded but
that is fine for the sequential test suite.
"""

import json
import uuid
import sys
import time
from http.server import BaseHTTPRequestHandler, HTTPServer
from typing import Optional
from urllib.parse import urlparse

_faults: dict = {}
_teams: dict = {}
_agents: dict = {}
_tasks: dict = {}

CHAOS_TYPES = {"network-partition", "latency", "kill", "queue-starve", "test-empty"}


def _parse_body(handler: "AgamemnonHandler") -> "tuple[Optional[dict], bool]":
    """Return (parsed_json_or_None, body_was_empty)."""
    length = int(handler.headers.get("Content-Length", 0))
    raw = handler.rfile.read(length) if length else b""
    if not raw:
        return None, True
    try:
        return json.loads(raw), False
    except (json.JSONDecodeError, UnicodeDecodeError):
        return None, False


class AgamemnonHandler(BaseHTTPRequestHandler):
    def log_message(self, fmt: str, *args: object) -> None:  # silence access log
        pass

    def _send(self, status: int, body: dict) -> None:
        payload = json.dumps(body).encode()
        self.send_response(status)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(payload)))
        self.end_headers()
        self.wfile.write(payload)

    def do_GET(self) -> None:
        path = urlparse(self.path).path.rstrip("/")

        if path == "/v1/health":
            self._send(200, {"status": "ok"})

        elif path == "/v1/chaos":
            self._send(200, {"faults": list(_faults.values())})

        elif path == "/v1/tasks":
            # Immediately mark every pending task as completed so polling tests terminate.
            for task in _tasks.values():
                task["status"] = "completed"
            self._send(200, {"tasks": list(_tasks.values())})

        else:
            self._send(404, {"error": "not found"})

    def do_POST(self) -> None:
        path = urlparse(self.path).path.rstrip("/")
        ct = self.headers.get("Content-Type", "")
        body, empty = _parse_body(self)

        # /v1/chaos/<type>
        if path.startswith("/v1/chaos/"):
            chaos_type = path[len("/v1/chaos/"):]
            if chaos_type not in CHAOS_TYPES:
                self._send(404, {"error": "unknown chaos type"})
                return
            if empty:
                # test-empty and other endpoints: empty body → 400
                self._send(400, {"error": "empty body"})
                return
            fault_id = str(uuid.uuid4())
            fault = {"id": fault_id, "type": chaos_type, "active": True}
            if body:
                fault.update({k: v for k, v in body.items() if k not in fault})
            _faults[fault_id] = fault
            self._send(201, fault)
            return

        # /v1/teams
        if path == "/v1/teams":
            if not _is_json(ct) or body is None:
                self._send(400, {"error": "bad request"})
                return
            team_id = str(uuid.uuid4())
            team = {"id": team_id, "name": body.get("name", "unnamed")}
            _teams[team_id] = team
            self._send(201, {"team": team})
            return

        # /v1/teams/<id>/tasks
        if path.startswith("/v1/teams/") and path.endswith("/tasks"):
            parts = path.split("/")
            if len(parts) >= 4:
                team_id = parts[3]
            else:
                team_id = "unknown"
            if not _is_json(ct) or body is None:
                self._send(400, {"error": "bad request"})
                return
            task_id = str(uuid.uuid4())
            task = {
                "id": task_id,
                "team_id": team_id,
                "subject": body.get("subject", ""),
                "status": "pending",
            }
            _tasks[task_id] = task
            self._send(201, {"task": task})
            return

        # /v1/agents
        if path == "/v1/agents":
            if not _is_json(ct) or body is None:
                self._send(400, {"error": "bad request"})
                return
            agent_id = str(uuid.uuid4())
            agent = {"id": agent_id, "name": body.get("name", "unnamed")}
            _agents[agent_id] = agent
            self._send(201, {"agent": agent, "id": agent_id})
            return

        self._send(404, {"error": "not found"})

    def do_DELETE(self) -> None:
        path = urlparse(self.path).path.rstrip("/")

        if path.startswith("/v1/chaos/"):
            fault_id = path[len("/v1/chaos/"):]
            if fault_id in _faults:
                del _faults[fault_id]
                self._send(200, {"deleted": fault_id})
            else:
                self._send(404, {"error": "fault not found"})
            return

        self._send(404, {"error": "not found"})


def _is_json(content_type: str) -> bool:
    return "application/json" in content_type or content_type == ""


def main() -> None:
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 8080
    server = HTTPServer(("0.0.0.0", port), AgamemnonHandler)
    print(f"mock-agamemnon listening on :{port}", flush=True)
    server.serve_forever()


if __name__ == "__main__":
    main()
