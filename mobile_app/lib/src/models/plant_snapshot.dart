class PlantSnapshot {
  const PlantSnapshot({
    required this.soilMoisturePercent,
    required this.waterLevel,
    required this.batteryVoltage,
    required this.temperature,
    required this.humidity,
    required this.pumpRunning,
    required this.alarmActive,
    required this.updatedAt,
  });

  final int soilMoisturePercent;
  final int waterLevel;
  final double batteryVoltage;
  final double temperature;
  final double humidity;
  final bool pumpRunning;
  final bool alarmActive;
  final DateTime updatedAt;

  factory PlantSnapshot.fromJson(Map<String, dynamic> json) {
    return PlantSnapshot(
      soilMoisturePercent: json['soilMoisturePercent'] as int? ?? 0,
      waterLevel: json['waterLevel'] as int? ?? 0,
      batteryVoltage: (json['batteryVoltage'] as num?)?.toDouble() ?? 0,
      temperature: (json['temperature'] as num?)?.toDouble() ?? 0,
      humidity: (json['humidity'] as num?)?.toDouble() ?? 0,
      pumpRunning: json['pumpRunning'] as bool? ?? false,
      alarmActive: json['alarmActive'] as bool? ?? false,
      updatedAt: DateTime.tryParse(json['updatedAt'] as String? ?? '') ?? DateTime.now(),
    );
  }
}
