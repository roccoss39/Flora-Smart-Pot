import 'package:flutter/material.dart';

import 'src/services/pot_repository.dart';
import 'src/services/supla_api_client.dart';
import 'src/ui/dashboard_page.dart';

void main() {
  const suplaBaseUrl = String.fromEnvironment('SUPLA_BASE_URL', defaultValue: 'https://example.com');
  const bearerToken = String.fromEnvironment('SUPLA_BEARER_TOKEN', defaultValue: 'replace_me');
  const deviceId = String.fromEnvironment('SUPLA_DEVICE_ID', defaultValue: 'flora-1');

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
