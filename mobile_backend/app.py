from __future__ import annotations

import json
import os
import sqlite3
from contextlib import contextmanager
from datetime import datetime, timezone
from typing import Any

from fastapi import Depends, FastAPI, Header, HTTPException, status
from pydantic import BaseModel, Field

APP_TITLE = "Flora Mobile Backend"
DB_PATH = os.getenv("FLORA_DB_PATH", "./flora_backend.db")
API_TOKEN = os.getenv("FLORA_API_TOKEN", "change-me-token")


class PlantSnapshot(BaseModel):
    soilMoisturePercent: int = 0
    waterLevel: int = 0
    batteryVoltage: float = 0.0
    temperature: float = 0.0
    humidity: float = 0.0
    pumpRunning: bool = False
    alarmActive: bool = False
    updatedAt: str = Field(default_factory=lambda: now_iso())


class PlantConfig(BaseModel):
    pumpDurationMs: int = 3000
    soilThresholdPercent: int = 50
    lowBatteryMilliVolts: int = 3300
    lowSoilPercent: int = 40
    waterLevelThreshold: int = 2000
    continuousMode: bool = True
    alarmSoundEnabled: bool = True
    soilDryAdc: int = 2621
    soilWetAdc: int = 950
    pumpPowerPercent: int = 100
    measurementHour: int = 8
    measurementMinute: int = 0


class PumpAction(BaseModel):
    durationMs: int = Field(ge=500, le=30000)


class TelemetryPush(BaseModel):
    snapshot: PlantSnapshot
    pumpRunning: bool | None = None
    alarmActive: bool | None = None


class CommandItem(BaseModel):
    id: int
    type: str
    payload: dict[str, Any]
    createdAt: str


class CommandsResponse(BaseModel):
    items: list[CommandItem]


def now_iso() -> str:
    return datetime.now(timezone.utc).isoformat()


@contextmanager
def db_conn() -> sqlite3.Connection:
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    try:
        yield conn
        conn.commit()
    finally:
        conn.close()


def init_db() -> None:
    with db_conn() as conn:
        conn.execute(
            """
            CREATE TABLE IF NOT EXISTS devices (
              device_id TEXT PRIMARY KEY,
              snapshot_json TEXT NOT NULL,
              config_json TEXT NOT NULL,
              updated_at TEXT NOT NULL
            )
            """
        )
        conn.execute(
            """
            CREATE TABLE IF NOT EXISTS commands (
              id INTEGER PRIMARY KEY AUTOINCREMENT,
              device_id TEXT NOT NULL,
              type TEXT NOT NULL,
              payload_json TEXT NOT NULL,
              created_at TEXT NOT NULL
            )
            """
        )


app = FastAPI(title=APP_TITLE)


@app.on_event("startup")
def on_startup() -> None:
    init_db()


def require_auth(authorization: str | None = Header(default=None)) -> None:
    if not authorization or not authorization.startswith("Bearer "):
        raise HTTPException(status_code=status.HTTP_401_UNAUTHORIZED, detail="Missing bearer token")

    token = authorization.removeprefix("Bearer ").strip()
    if token != API_TOKEN:
        raise HTTPException(status_code=status.HTTP_401_UNAUTHORIZED, detail="Invalid token")


def get_or_create_device(device_id: str) -> tuple[PlantSnapshot, PlantConfig]:
    with db_conn() as conn:
        row = conn.execute(
            "SELECT snapshot_json, config_json FROM devices WHERE device_id = ?",
            (device_id,),
        ).fetchone()

        if row:
            snapshot = PlantSnapshot(**json.loads(row["snapshot_json"]))
            config = PlantConfig(**json.loads(row["config_json"]))
            return snapshot, config

        snapshot = PlantSnapshot()
        config = PlantConfig()
        conn.execute(
            "INSERT INTO devices(device_id, snapshot_json, config_json, updated_at) VALUES(?, ?, ?, ?)",
            (device_id, snapshot.model_dump_json(), config.model_dump_json(), now_iso()),
        )
        return snapshot, config


@app.get("/health")
def health() -> dict[str, str]:
    return {"status": "ok", "time": now_iso()}


@app.get("/api/flora/{device_id}/snapshot", response_model=PlantSnapshot, dependencies=[Depends(require_auth)])
def get_snapshot(device_id: str) -> PlantSnapshot:
    snapshot, _ = get_or_create_device(device_id)
    return snapshot


@app.get("/api/flora/{device_id}/config", response_model=PlantConfig, dependencies=[Depends(require_auth)])
def get_config(device_id: str) -> PlantConfig:
    _, config = get_or_create_device(device_id)
    return config


@app.put("/api/flora/{device_id}/config", response_model=PlantConfig, dependencies=[Depends(require_auth)])
def put_config(device_id: str, payload: PlantConfig) -> PlantConfig:
    get_or_create_device(device_id)
    with db_conn() as conn:
        conn.execute(
            "UPDATE devices SET config_json = ?, updated_at = ? WHERE device_id = ?",
            (payload.model_dump_json(), now_iso(), device_id),
        )
    return payload


@app.post("/api/flora/{device_id}/actions/pump", status_code=status.HTTP_202_ACCEPTED, dependencies=[Depends(require_auth)])
def post_pump(device_id: str, payload: PumpAction) -> dict[str, Any]:
    get_or_create_device(device_id)
    created_at = now_iso()
    with db_conn() as conn:
        cur = conn.execute(
            "INSERT INTO commands(device_id, type, payload_json, created_at) VALUES(?, ?, ?, ?)",
            (device_id, "pump", json.dumps(payload.model_dump()), created_at),
        )
        cmd_id = int(cur.lastrowid)

    return {"accepted": True, "commandId": cmd_id, "createdAt": created_at}


@app.post("/api/flora/{device_id}/telemetry", dependencies=[Depends(require_auth)])
def post_telemetry(device_id: str, payload: TelemetryPush) -> dict[str, Any]:
    _, cfg = get_or_create_device(device_id)

    incoming = payload.snapshot.model_copy(deep=True)
    if payload.pumpRunning is not None:
        incoming.pumpRunning = payload.pumpRunning
    if payload.alarmActive is not None:
        incoming.alarmActive = payload.alarmActive
    incoming.updatedAt = now_iso()

    with db_conn() as conn:
        conn.execute(
            "UPDATE devices SET snapshot_json = ?, updated_at = ? WHERE device_id = ?",
            (incoming.model_dump_json(), now_iso(), device_id),
        )

    return {"stored": True, "config": cfg.model_dump()}


@app.get("/api/flora/{device_id}/commands", response_model=CommandsResponse, dependencies=[Depends(require_auth)])
def get_commands(device_id: str, after_id: int = 0, limit: int = 20) -> CommandsResponse:
    get_or_create_device(device_id)

    limit = max(1, min(limit, 200))
    with db_conn() as conn:
        rows = conn.execute(
            """
            SELECT id, type, payload_json, created_at
            FROM commands
            WHERE device_id = ? AND id > ?
            ORDER BY id ASC
            LIMIT ?
            """,
            (device_id, after_id, limit),
        ).fetchall()

    return CommandsResponse(
        items=[
            CommandItem(
                id=row["id"],
                type=row["type"],
                payload=json.loads(row["payload_json"]),
                createdAt=row["created_at"],
            )
            for row in rows
        ]
    )
