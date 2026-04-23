# Flora SUPLA App (Flutter)

Aplikacja Flutter (Android + iOS), która odtwarza kluczowe widgety i sterowanie z obecnej konfiguracji Blynk w projekcie Flora Smart Pot.

## Co odwzorowuje z firmware

Mapowanie bazuje na `src/BlynkManager.cpp`:

- **Dane live:** wilgotność gleby, poziom wody, bateria, temperatura, wilgotność powietrza.
- **Stany:** pompa, alarm.
- **Sterowanie:** ręczne uruchomienie pompy, czas pracy pompy, próg wilgotności, próg baterii, próg alarmu gleby, próg poziomu wody, tryb ciągły/deep sleep, dźwięk alarmu, kalibracja ADC mokro/sucho, moc pompy, godzina i minuta pomiaru.

## Uruchomienie

```bash
cd mobile_app 
flutter pub get
flutter run
```
cd /home/dawid/Documents/PlatformIO/Projects/Flora/mobile_app

## Konfiguracja SUPLA API

W `lib/main.dart` ustaw:

- `suplaBaseUrl` – URL Twojego backendu (np. proxy na SUPLA Cloud).
- `bearerToken` – token dostępowy użytkownika.
- `deviceId` – identyfikator urządzenia/doniczki.

## Uwaga integracyjna

SUPLA i Blynk mają inny model kanałów i akcji. Dlatego endpointy w `SuplaApiClient` są przygotowane jako **czytelny adapter**, który łatwo dopasujesz do swojego backendu SUPLA (lub własnego middleware pomiędzy SUPLA a aplikacją).
