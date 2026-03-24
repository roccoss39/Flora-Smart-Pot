import 'package:flutter/material.dart';

import '../models/plant_config.dart';
import '../models/plant_snapshot.dart';
import '../services/pot_repository.dart';
import 'widgets/metric_tile.dart';

class DashboardPage extends StatefulWidget {
  const DashboardPage({super.key, required this.repository});

  final PotRepository repository;

  @override
  State<DashboardPage> createState() => _DashboardPageState();
}

class _DashboardPageState extends State<DashboardPage> {
  PlantSnapshot? _snapshot;
  PlantConfig? _config;
  bool _loading = true;
  bool _saving = false;
  String? _error;

  @override
  void initState() {
    super.initState();
    _loadAll();
  }

  Future<void> _loadAll() async {
    setState(() {
      _loading = true;
      _error = null;
    });

    try {
      final snapshot = await widget.repository.getSnapshot();
      final config = await widget.repository.getConfig();
      if (!mounted) return;
      setState(() {
        _snapshot = snapshot;
        _config = config;
        _loading = false;
      });
    } catch (e) {
      if (!mounted) return;
      setState(() {
        _error = e.toString();
        _loading = false;
      });
    }
  }

  Future<void> _saveConfig(PlantConfig next) async {
    setState(() {
      _config = next;
      _saving = true;
      _error = null;
    });

    try {
      await widget.repository.saveConfig(next);
      if (!mounted) return;
      setState(() => _saving = false);
    } catch (e) {
      if (!mounted) return;
      setState(() {
        _saving = false;
        _error = e.toString();
      });
    }
  }

  Future<void> _triggerPump() async {
    final config = _config;
    if (config == null) return;

    setState(() {
      _saving = true;
      _error = null;
    });

    try {
      await widget.repository.runPump(config.pumpDurationMs);
      await _loadAll();
      if (!mounted) return;
      setState(() => _saving = false);
    } catch (e) {
      if (!mounted) return;
      setState(() {
        _saving = false;
        _error = e.toString();
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Flora Smart Pot'),
        actions: [
          IconButton(
            onPressed: _loading ? null : _loadAll,
            icon: const Icon(Icons.refresh),
            tooltip: 'Odśwież',
          ),
        ],
      ),
      body: _loading
          ? const Center(child: CircularProgressIndicator())
          : _error != null
              ? _ErrorView(message: _error!, onRetry: _loadAll)
              : _DashboardContent(
                  snapshot: _snapshot,
                  config: _config,
                  isSaving: _saving,
                  onTriggerPump: _triggerPump,
                  onConfigChanged: _saveConfig,
                ),
    );
  }
}

class _DashboardContent extends StatelessWidget {
  const _DashboardContent({
    required this.snapshot,
    required this.config,
    required this.isSaving,
    required this.onTriggerPump,
    required this.onConfigChanged,
  });

  final PlantSnapshot? snapshot;
  final PlantConfig? config;
  final bool isSaving;
  final VoidCallback onTriggerPump;
  final ValueChanged<PlantConfig> onConfigChanged;

  @override
  Widget build(BuildContext context) {
    if (snapshot == null || config == null) {
      return const Center(child: Text('Brak danych.'));
    }

    final s = snapshot!;
    final c = config!;

    return ListView(
      padding: const EdgeInsets.all(12),
      children: [
        Wrap(
          spacing: 8,
          runSpacing: 8,
          children: [
            SizedBox(width: 280, child: MetricTile(label: 'Wilgotność gleby', value: '${s.soilMoisturePercent} %', icon: Icons.grass)),
            SizedBox(width: 280, child: MetricTile(label: 'Poziom wody', value: '${s.waterLevel}/5', icon: Icons.water_drop)),
            SizedBox(width: 280, child: MetricTile(label: 'Bateria', value: '${s.batteryVoltage.toStringAsFixed(2)} V', icon: Icons.battery_charging_full)),
            SizedBox(width: 280, child: MetricTile(label: 'Temperatura', value: '${s.temperature.toStringAsFixed(1)} °C', icon: Icons.thermostat)),
            SizedBox(width: 280, child: MetricTile(label: 'Wilgotność powietrza', value: '${s.humidity.toStringAsFixed(1)} %', icon: Icons.air)),
            SizedBox(width: 280, child: MetricTile(label: 'Pompa', value: s.pumpRunning ? 'WŁĄCZONA' : 'WYŁĄCZONA', icon: Icons.water, color: s.pumpRunning ? Colors.green : null)),
            SizedBox(width: 280, child: MetricTile(label: 'Alarm', value: s.alarmActive ? 'AKTYWNY' : 'BRAK', icon: Icons.warning_amber, color: s.alarmActive ? Colors.red : Colors.green)),
          ],
        ),
        const SizedBox(height: 12),
        Card(
          child: Padding(
            padding: const EdgeInsets.all(16),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text('Sterowanie (odpowiednik Blynk)', style: Theme.of(context).textTheme.titleMedium),
                const SizedBox(height: 10),
                FilledButton.icon(
                  onPressed: isSaving ? null : onTriggerPump,
                  icon: const Icon(Icons.play_arrow),
                  label: Text('Uruchom pompę (${c.pumpDurationMs} ms)'),
                ),
                const SizedBox(height: 8),
                _intSlider(
                  context,
                  label: 'Czas pracy pompy (ms)',
                  value: c.pumpDurationMs,
                  min: 500,
                  max: 30000,
                  onChanged: (v) => onConfigChanged(c.copyWith(pumpDurationMs: v)),
                ),
                _intSlider(
                  context,
                  label: 'Próg wilgotności gleby (%)',
                  value: c.soilThresholdPercent,
                  min: 0,
                  max: 100,
                  onChanged: (v) => onConfigChanged(c.copyWith(soilThresholdPercent: v)),
                ),
                _intSlider(
                  context,
                  label: 'Próg baterii (mV)',
                  value: c.lowBatteryMilliVolts,
                  min: 2500,
                  max: 4200,
                  onChanged: (v) => onConfigChanged(c.copyWith(lowBatteryMilliVolts: v)),
                ),
                _intSlider(
                  context,
                  label: 'Próg alarmu wilg. gleby (%)',
                  value: c.lowSoilPercent,
                  min: 0,
                  max: 100,
                  onChanged: (v) => onConfigChanged(c.copyWith(lowSoilPercent: v)),
                ),
                _intSlider(
                  context,
                  label: 'Próg poziomu wody (ADC)',
                  value: c.waterLevelThreshold,
                  min: 0,
                  max: 4095,
                  onChanged: (v) => onConfigChanged(c.copyWith(waterLevelThreshold: v)),
                ),
                _intSlider(
                  context,
                  label: 'Kalibracja gleby sucho (ADC)',
                  value: c.soilDryAdc,
                  min: 0,
                  max: 4095,
                  onChanged: (v) => onConfigChanged(c.copyWith(soilDryAdc: v)),
                ),
                _intSlider(
                  context,
                  label: 'Kalibracja gleby mokro (ADC)',
                  value: c.soilWetAdc,
                  min: 0,
                  max: 4095,
                  onChanged: (v) => onConfigChanged(c.copyWith(soilWetAdc: v)),
                ),
                _intSlider(
                  context,
                  label: 'Moc pompy (%)',
                  value: c.pumpPowerPercent,
                  min: 0,
                  max: 100,
                  onChanged: (v) => onConfigChanged(c.copyWith(pumpPowerPercent: v)),
                ),
                SwitchListTile(
                  contentPadding: EdgeInsets.zero,
                  title: const Text('Tryb ciągły (bez Deep Sleep)'),
                  value: c.continuousMode,
                  onChanged: (v) => onConfigChanged(c.copyWith(continuousMode: v)),
                ),
                SwitchListTile(
                  contentPadding: EdgeInsets.zero,
                  title: const Text('Dźwięk alarmu'),
                  value: c.alarmSoundEnabled,
                  onChanged: (v) => onConfigChanged(c.copyWith(alarmSoundEnabled: v)),
                ),
                ListTile(
                  contentPadding: EdgeInsets.zero,
                  title: const Text('Godzina pomiaru'),
                  subtitle: Text('${c.measurementHour.toString().padLeft(2, '0')}:${c.measurementMinute.toString().padLeft(2, '0')}'),
                  trailing: OutlinedButton(
                    onPressed: () async {
                      final contextMounted = context.mounted;
                      final picked = await showTimePicker(
                        context: context,
                        initialTime: TimeOfDay(hour: c.measurementHour, minute: c.measurementMinute),
                      );
                      if (picked == null || !contextMounted) return;
                      onConfigChanged(c.copyWith(measurementHour: picked.hour, measurementMinute: picked.minute));
                    },
                    child: const Text('Zmień'),
                  ),
                ),
              ],
            ),
          ),
        ),
      ],
    );
  }

  Widget _intSlider(
    BuildContext context, {
    required String label,
    required int value,
    required int min,
    required int max,
    required ValueChanged<int> onChanged,
  }) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text('$label: $value'),
        Slider(
          value: value.toDouble().clamp(min.toDouble(), max.toDouble()),
          min: min.toDouble(),
          max: max.toDouble(),
          divisions: (max - min).clamp(1, 200),
          label: '$value',
          onChanged: isSaving ? null : (v) => onChanged(v.round()),
        ),
      ],
    );
  }
}

class _ErrorView extends StatelessWidget {
  const _ErrorView({required this.message, required this.onRetry});

  final String message;
  final VoidCallback onRetry;

  @override
  Widget build(BuildContext context) {
    return Center(
      child: Padding(
        padding: const EdgeInsets.all(20),
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            const Icon(Icons.error_outline, size: 40, color: Colors.redAccent),
            const SizedBox(height: 10),
            Text(message, textAlign: TextAlign.center),
            const SizedBox(height: 10),
            FilledButton(onPressed: onRetry, child: const Text('Spróbuj ponownie')),
          ],
        ),
      ),
    );
  }
}
