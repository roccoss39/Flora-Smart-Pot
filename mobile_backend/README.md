# Flora Mobile Backend (FastAPI)

Lekki backend pod aplikację Flutter (`mobile_app`) i firmware ESP32.

## Co robi

- API kompatybilne z aplikacją mobilną:
  - `GET /api/flora/{deviceId}/snapshot`
  - `GET /api/flora/{deviceId}/config`
  - `PUT /api/flora/{deviceId}/config`
  - `POST /api/flora/{deviceId}/actions/pump`
- Dodatkowe endpointy dla ESP32:
  - `POST /api/flora/{deviceId}/telemetry` (push odczytów)
  - `GET /api/flora/{deviceId}/commands?after_id=...` (poll komend)

Wszystko trzymane lokalnie w SQLite (dobrze działa na Raspberry Pi Zero 2).

## Szybki start (Linux / Raspberry Pi)

```bash
cd mobile_backend
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt

export FLORA_API_TOKEN='tu_wstaw_mocny_token'
export FLORA_DB_PATH='./flora_backend.db'

uvicorn app:app --host 0.0.0.0 --port 8080
```

## Integracja z aplikacją Flutter

```bash
flutter run -d <android_id> \
  --dart-define=SUPLA_BASE_URL=http://<IP_SERWERA>:8080 \
  --dart-define=SUPLA_BEARER_TOKEN=tu_wstaw_mocny_token \
  --dart-define=SUPLA_DEVICE_ID=flora-1
```

## Przykład testu API

```bash
TOKEN='tu_wstaw_mocny_token'
BASE='http://127.0.0.1:8080'
DEVICE='flora-1'

curl -H "Authorization: Bearer $TOKEN" "$BASE/api/flora/$DEVICE/snapshot"

curl -X POST -H "Authorization: Bearer $TOKEN" -H 'Content-Type: application/json' \
  -d '{"snapshot":{"soilMoisturePercent":41,"waterLevel":3,"batteryVoltage":3.92,"temperature":24.1,"humidity":46.0,"pumpRunning":false,"alarmActive":false,"updatedAt":"2026-03-24T00:00:00Z"}}' \
  "$BASE/api/flora/$DEVICE/telemetry"

curl -H "Authorization: Bearer $TOKEN" "$BASE/api/flora/$DEVICE/snapshot"
```

## Produkcyjnie na Raspberry Pi (systemd)

Utwórz `/etc/systemd/system/flora-backend.service`:

```ini
[Unit]
Description=Flora Mobile Backend
After=network.target

[Service]
User=pi
WorkingDirectory=/home/pi/Flora-Smart-Pot/mobile_backend
Environment=FLORA_API_TOKEN=tu_wstaw_mocny_token
Environment=FLORA_DB_PATH=/home/pi/Flora-Smart-Pot/mobile_backend/flora_backend.db
ExecStart=/home/pi/Flora-Smart-Pot/mobile_backend/.venv/bin/uvicorn app:app --host 0.0.0.0 --port 8080
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
```

Aktywacja:

```bash
sudo systemctl daemon-reload
sudo systemctl enable flora-backend
sudo systemctl start flora-backend
sudo systemctl status flora-backend
```
