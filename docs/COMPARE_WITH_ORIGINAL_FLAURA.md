# Porównanie projektu z oryginałem Flaura (piny)

Data porównania: 2026-04-22

## Źródła
- Oryginał: https://github.com/FlauraPlantPot/Flaura (plik `Flaura_Blynk.ino`, sekcja `//PIN Configuration`)
- Aktualny projekt: `src/DeviceConfig.cpp` + `docs/HARDWARE_GUIDE.md` + `README.md`

## 1) Mapa pinów — oryginalna Flaura

Z oryginalnego `Flaura_Blynk.ino`:
- `buttonPin = 0`
- `batteryLevelPin = 32`
- `pumpPowerPin = 23`
- `moistureSensorPowerPin = 19`
- `moistureSensorSignalPin = 33`
- `waterLevelGroundPin = 35`
- `waterLevelPin[] = {13, 14, 27, 26, 25}` (kolejność: 100%, 75%, 50%, 25%, 10%)

## 2) Aktualny stan po dostosowaniu

W repo pozostają **2 profile płytek**:

### A) `BOARD_LOLIN_D32` (`env:lolin_d32`) — bez zmian
- Soil signal (ADC): `34`
- Soil VCC: `4`
- Water level pins: `{33, 25, 26, 27, 14}`
- Water ground: `32`
- Pump: `15`
- Battery ADC: `35`
- DHT data/power: `16 / 17`
- MPU INT: `13`
- Button: `0`
- Buzzer: `23`
- LED: `5`

### B) Domyślny `LOLIN32 v1.0.0` (`env:lolin32`) — zgodny z oryginałem
- Soil signal (ADC): `33`
- Soil VCC: `19`
- Water level pins: `{13, 14, 27, 26, 25}`
- Water ground: `35`
- Pump: `23`
- Battery ADC: `32`
- Button: `0`
- DHT: `wyłączony (255)`
- MPU INT: `wyłączony (255)`
- Buzzer: `wyłączony (255)`
- LED: `wyłączona (255)`

## 3) Co to oznacza funkcjonalnie

- Dla `lolin32` używasz teraz dokładnie „oryginalnego” zestawu urządzeń (gleba + poziom wody + pompa + bateria + przycisk).
- Dla `lolin_d32` zostaje pełna funkcjonalność rozszerzona (DHT/buzzer/LED/MPU-pin), bez zmian względem poprzedniej konfiguracji.
- Dokumentacja (`README.md` i `docs/HARDWARE_GUIDE.md`) została dopasowana do tych dwóch profili.
