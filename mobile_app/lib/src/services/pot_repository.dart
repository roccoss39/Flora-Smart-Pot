import '../models/plant_config.dart';
import '../models/plant_snapshot.dart';
import 'supla_api_client.dart';

class PotRepository {
  PotRepository({required SuplaApiClient apiClient}) : _apiClient = apiClient;

  final SuplaApiClient _apiClient;

  Future<PlantSnapshot> getSnapshot() => _apiClient.fetchSnapshot();

  Future<PlantConfig> getConfig() => _apiClient.fetchConfig();

  Future<void> saveConfig(PlantConfig config) => _apiClient.updateConfig(config);

  Future<void> runPump(int durationMs) => _apiClient.triggerPump(durationMs: durationMs);
}
