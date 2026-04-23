import 'package:flutter/material.dart';
import 'package:flutter_dotenv/flutter_dotenv.dart'; // Twój import .env

import 'src/services/pot_repository.dart';
import 'src/services/supla_api_client.dart';
import 'src/ui/dashboard_page.dart';

Future<void> main() async {
  // 1. To jest BARDZO WAŻNE: mówi Flutterowi, żeby przygotował swoje
  // silniki pod spodem, zanim zaczniemy wczytywać pliki asynchronicznie.
  WidgetsFlutterBinding.ensureInitialized();

  // 2. Ładujemy nasz bezpieczny plik .env
  await dotenv.load(fileName: ".env");

  // 3. Wyciągamy zmienne. Używamy ?? (fallback), na wypadek gdybyś
  // kiedyś zapomniał dodać plik .env na nowym komputerze.
  final suplaBaseUrl = dotenv.env['SUPLA_BASE_URL'] ?? 'http://192.168.0.68:8080';
  final bearerToken = dotenv.env['SUPLA_BEARER_TOKEN'] ?? 'replace_me';
  final deviceId = dotenv.env['SUPLA_DEVICE_ID'] ?? 'flora-1';

  final repository = PotRepository(
    apiClient: SuplaApiClient(
      baseUrl: suplaBaseUrl,
      bearerToken: bearerToken,
      deviceId: deviceId,
    ),
  );

  runApp(FloraSuplaApp(repository: repository));
}

class FloraSuplaApp extends StatelessWidget {
  const FloraSuplaApp({super.key, required this.repository});

  final PotRepository repository;

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      title: 'Flora Smart Pot',
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(seedColor: Colors.green),
        useMaterial3: true,
      ),
      home: DashboardPage(repository: repository),
    );
  }
}