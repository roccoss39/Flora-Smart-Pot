class PlantConfig {
  const PlantConfig({
    required this.pumpDurationMs,
    required this.soilThresholdPercent,
    required this.lowBatteryMilliVolts,
    required this.lowSoilPercent,
    required this.waterLevelThreshold,
    required this.continuousMode,
    required this.alarmSoundEnabled,
    required this.soilDryAdc,
    required this.soilWetAdc,
    required this.pumpPowerPercent,
    required this.measurementHour,
    required this.measurementMinute,
  });

  final int pumpDurationMs;
  final int soilThresholdPercent;
  final int lowBatteryMilliVolts;
  final int lowSoilPercent;
  final int waterLevelThreshold;
  final bool continuousMode;
  final bool alarmSoundEnabled;
  final int soilDryAdc;
  final int soilWetAdc;
  final int pumpPowerPercent;
  final int measurementHour;
  final int measurementMinute;

  PlantConfig copyWith({
    int? pumpDurationMs,
    int? soilThresholdPercent,
    int? lowBatteryMilliVolts,
    int? lowSoilPercent,
    int? waterLevelThreshold,
    bool? continuousMode,
    bool? alarmSoundEnabled,
    int? soilDryAdc,
    int? soilWetAdc,
    int? pumpPowerPercent,
    int? measurementHour,
    int? measurementMinute,
  }) {
    return PlantConfig(
      pumpDurationMs: pumpDurationMs ?? this.pumpDurationMs,
      soilThresholdPercent: soilThresholdPercent ?? this.soilThresholdPercent,
      lowBatteryMilliVolts: lowBatteryMilliVolts ?? this.lowBatteryMilliVolts,
      lowSoilPercent: lowSoilPercent ?? this.lowSoilPercent,
      waterLevelThreshold: waterLevelThreshold ?? this.waterLevelThreshold,
      continuousMode: continuousMode ?? this.continuousMode,
      alarmSoundEnabled: alarmSoundEnabled ?? this.alarmSoundEnabled,
      soilDryAdc: soilDryAdc ?? this.soilDryAdc,
      soilWetAdc: soilWetAdc ?? this.soilWetAdc,
      pumpPowerPercent: pumpPowerPercent ?? this.pumpPowerPercent,
      measurementHour: measurementHour ?? this.measurementHour,
      measurementMinute: measurementMinute ?? this.measurementMinute,
    );
  }

  factory PlantConfig.fromJson(Map<String, dynamic> json) {
    return PlantConfig(
      pumpDurationMs: json['pumpDurationMs'] as int? ?? 3000,
      soilThresholdPercent: json['soilThresholdPercent'] as int? ?? 50,
      lowBatteryMilliVolts: json['lowBatteryMilliVolts'] as int? ?? 3300,
      lowSoilPercent: json['lowSoilPercent'] as int? ?? 40,
      waterLevelThreshold: json['waterLevelThreshold'] as int? ?? 2000,
      continuousMode: json['continuousMode'] as bool? ?? true,
      alarmSoundEnabled: json['alarmSoundEnabled'] as bool? ?? true,
      soilDryAdc: json['soilDryAdc'] as int? ?? 2621,
      soilWetAdc: json['soilWetAdc'] as int? ?? 950,
      pumpPowerPercent: json['pumpPowerPercent'] as int? ?? 100,
      measurementHour: json['measurementHour'] as int? ?? 8,
      measurementMinute: json['measurementMinute'] as int? ?? 0,
    );
  }

  Map<String, dynamic> toJson() => {
        'pumpDurationMs': pumpDurationMs,
        'soilThresholdPercent': soilThresholdPercent,
        'lowBatteryMilliVolts': lowBatteryMilliVolts,
        'lowSoilPercent': lowSoilPercent,
        'waterLevelThreshold': waterLevelThreshold,
        'continuousMode': continuousMode,
        'alarmSoundEnabled': alarmSoundEnabled,
        'soilDryAdc': soilDryAdc,
        'soilWetAdc': soilWetAdc,
        'pumpPowerPercent': pumpPowerPercent,
        'measurementHour': measurementHour,
        'measurementMinute': measurementMinute,
      };
}
