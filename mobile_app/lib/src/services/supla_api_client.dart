import 'dart:convert';

import 'package:http/http.dart' as http;

import '../models/plant_config.dart';
import '../models/plant_snapshot.dart';

class SuplaApiClient {
  SuplaApiClient({
    required this.baseUrl,
    required this.bearerToken,
    required this.deviceId,
    http.Client? client,
  }) : _client = client ?? http.Client();

  final String baseUrl;
  final String bearerToken;
  final String deviceId;
  final http.Client _client;

  Map<String, String> get _headers => {
        'Authorization': 'Bearer $bearerToken',
        'Content-Type': 'application/json',
      };

  Future<PlantSnapshot> fetchSnapshot() async {
    final uri = Uri.parse('$baseUrl/api/flora/$deviceId/snapshot');
    final response = await _client.get(uri, headers: _headers);
    _throwIfNotOk(response, 'snapshot');
    return PlantSnapshot.fromJson(jsonDecode(response.body) as Map<String, dynamic>);
  }

  Future<PlantConfig> fetchConfig() async {
    final uri = Uri.parse('$baseUrl/api/flora/$deviceId/config');
    final response = await _client.get(uri, headers: _headers);
    _throwIfNotOk(response, 'config');
    return PlantConfig.fromJson(jsonDecode(response.body) as Map<String, dynamic>);
  }

  Future<void> updateConfig(PlantConfig config) async {
    final uri = Uri.parse('$baseUrl/api/flora/$deviceId/config');
    final response = await _client.put(
      uri,
      headers: _headers,
      body: jsonEncode(config.toJson()),
    );
    _throwIfNotOk(response, 'update config');
  }

  Future<void> triggerPump({required int durationMs}) async {
    final uri = Uri.parse('$baseUrl/api/flora/$deviceId/actions/pump');
    final response = await _client.post(
      uri,
      headers: _headers,
      body: jsonEncode({'durationMs': durationMs}),
    );
    _throwIfNotOk(response, 'trigger pump');
  }

  void _throwIfNotOk(http.Response response, String operation) {
    if (response.statusCode < 200 || response.statusCode >= 300) {
      throw Exception('SUPLA API error ($operation): ${response.statusCode} ${response.body}');
    }
  }
}
