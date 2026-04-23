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

  Future<void> _loadAll({bool silent = false}) async {
    // Jeśli nie jesteśmy w trybie cichym, pokaż kółko na środku ekranu
    if (!silent) {
      setState(() {
        _loading = true;
        _error = null;
      });
    }

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
      _saving = true; // Przycisk pompy zrobi się szary (zablokowany)
      _error = null;
    });

    try {
      await widget.repository.runPump(config.pumpDurationMs);
      
      // ZMIANA TUTAJ: Pobieramy dane w tle, bez resetowania scrolla!
      await _loadAll(silent: true); 
      
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
              : RefreshIndicator(
                  onRefresh: () => _loadAll(silent: true),
                  child: _DashboardContent(
                    snapshot: _snapshot,
                    config: _config,
                    isSaving: _saving,
                    onTriggerPump: _triggerPump,
                    onConfigChanged: _saveConfig,
                  ),
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
      return ListView(
        children: const [
          Center(child: Padding(
            padding: EdgeInsets.all(32.0),
            child: Text('Brak danych.'),
          ))
        ],
      );
    }

    final s = snapshot!;
    final c = config!;

    final pumpSecondsStr = (c.pumpDurationMs / 1000).toStringAsFixed(1);

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
            // --- TYMCZASOWO UKRYTE CZUJNIKI SRODOWISKOWE ---
            // Odkomentuj poniższe dwie linijki (usuń '//'), gdy podepniesz czujnik
            // SizedBox(width: 280, child: MetricTile(label: 'Temperatura', value: '${s.temperature.toStringAsFixed(1)} °C', icon: Icons.thermostat)),
            // SizedBox(width: 280, child: MetricTile(label: 'Wilgotność powietrza', value: '${s.humidity.toStringAsFixed(1)} %', icon: Icons.air)),
            // -----------------------------------------------
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
                Text('Sterowanie', style: Theme.of(context).textTheme.titleMedium),
                const SizedBox(height: 10),
                FilledButton.icon(
                  onPressed: isSaving ? null : onTriggerPump,
                  icon: const Icon(Icons.play_arrow),
                  label: Text('Uruchom pompę ($pumpSecondsStr s)'),
                ),
                const SizedBox(height: 8),
                
                // Poniżej używamy naszego nowego, inteligentnego suwaka:
                _SmartConfigSlider(
                  label: 'Czas pracy pompy',
                  value: c.pumpDurationMs,
                  min: 500,
                  max: 30000,
                  disabled: isSaving,
                  valueFormatter: (v) => '${(v / 1000).toStringAsFixed(1)} s',
                  onSave: (v) => onConfigChanged(c.copyWith(pumpDurationMs: v)),
                ),
                _SmartConfigSlider(
                  label: 'Próg wilgotności gleby',
                  value: c.soilThresholdPercent,
                  min: 0,
                  max: 100,
                  disabled: isSaving,
                  valueFormatter: (v) => '$v %',
                  onSave: (v) => onConfigChanged(c.copyWith(soilThresholdPercent: v)),
                ),
                _SmartConfigSlider(
                  label: 'Próg baterii',
                  value: c.lowBatteryMilliVolts,
                  min: 2500,
                  max: 4200,
                  disabled: isSaving,
                  valueFormatter: (v) => '${(v / 1000).toStringAsFixed(2)} V',
                  onSave: (v) => onConfigChanged(c.copyWith(lowBatteryMilliVolts: v)),
                ),
                _SmartConfigSlider(
                  label: 'Próg alarmu wilg. gleby',
                  value: c.lowSoilPercent,
                  min: 0,
                  max: 100,
                  disabled: isSaving,
                  valueFormatter: (v) => '$v %',
                  onSave: (v) => onConfigChanged(c.copyWith(lowSoilPercent: v)),
                ),
                _SmartConfigSlider(
                  label: 'Próg poziomu wody',
                  value: c.waterLevelThreshold,
                  min: 0,
                  max: 4095,
                  disabled: isSaving,
                  valueFormatter: (v) => '$v ADC',
                  onSave: (v) => onConfigChanged(c.copyWith(waterLevelThreshold: v)),
                ),
                _SmartConfigSlider(
                  label: 'Kalibracja gleby sucho',
                  value: c.soilDryAdc,
                  min: 0,
                  max: 4095,
                  disabled: isSaving,
                  valueFormatter: (v) => '$v ADC',
                  onSave: (v) => onConfigChanged(c.copyWith(soilDryAdc: v)),
                ),
                _SmartConfigSlider(
                  label: 'Kalibracja gleby mokro',
                  value: c.soilWetAdc,
                  min: 0,
                  max: 4095,
                  disabled: isSaving,
                  valueFormatter: (v) => '$v ADC',
                  onSave: (v) => onConfigChanged(c.copyWith(soilWetAdc: v)),
                ),
                _SmartConfigSlider(
                  label: 'Moc pompy',
                  value: c.pumpPowerPercent,
                  min: 0,
                  max: 100,
                  disabled: isSaving,
                  valueFormatter: (v) => '$v %',
                  onSave: (v) => onConfigChanged(c.copyWith(pumpPowerPercent: v)),
                ),
                
                SwitchListTile(
                  contentPadding: EdgeInsets.zero,
                  title: const Text('Tryb ciągły (bez Deep Sleep)'),
                  value: c.continuousMode,
                  onChanged: isSaving ? null : (v) => onConfigChanged(c.copyWith(continuousMode: v)),
                ),
                SwitchListTile(
                  contentPadding: EdgeInsets.zero,
                  title: const Text('Dźwięk alarmu'),
                  value: c.alarmSoundEnabled,
                  onChanged: isSaving ? null : (v) => onConfigChanged(c.copyWith(alarmSoundEnabled: v)),
                ),
                ListTile(
                  contentPadding: EdgeInsets.zero,
                  title: const Text('Godzina pomiaru'),
                  subtitle: Text('${c.measurementHour.toString().padLeft(2, '0')}:${c.measurementMinute.toString().padLeft(2, '0')}'),
                  trailing: OutlinedButton(
                    onPressed: isSaving ? null : () async {
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
}

// ----------------------------------------------------------------------
// OSOBNA KLASA: Inteligentny Suwak z wbudowanym stanem
// ----------------------------------------------------------------------
class _SmartConfigSlider extends StatefulWidget {
  const _SmartConfigSlider({
    required this.label,
    required this.value,
    required this.min,
    required this.max,
    required this.onSave,
    this.valueFormatter,
    this.disabled = false,
  });

  final String label;
  final int value;
  final int min;
  final int max;
  final ValueChanged<int> onSave;
  final String Function(int)? valueFormatter;
  final bool disabled;

  @override
  State<_SmartConfigSlider> createState() => _SmartConfigSliderState();
}

class _SmartConfigSliderState extends State<_SmartConfigSlider> {
  late double _currentValue;

  @override
  void initState() {
    super.initState();
    _currentValue = widget.value.toDouble();
  }

  @override
  void didUpdateWidget(covariant _SmartConfigSlider oldWidget) {
    super.didUpdateWidget(oldWidget);
    if (oldWidget.value != widget.value) {
      _currentValue = widget.value.toDouble();
    }
  }

  @override
  Widget build(BuildContext context) {
    final displayValue = widget.valueFormatter != null 
        ? widget.valueFormatter!(_currentValue.round()) 
        : '${_currentValue.round()}';

    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text('${widget.label}: $displayValue', style: const TextStyle(fontWeight: FontWeight.w500)),
        Slider(
          // Naprawiony błąd num vs double!
          value: _currentValue.clamp(widget.min.toDouble(), widget.max.toDouble()).toDouble(),
          min: widget.min.toDouble(),
          max: widget.max.toDouble(),
          divisions: (widget.max - widget.min).clamp(1, 200),
          label: displayValue,
          onChanged: widget.disabled
              ? null
              : (v) {
                  setState(() {
                    _currentValue = v;
                  });
                },
          onChangeEnd: widget.disabled
              ? null
              : (v) {
                  widget.onSave(v.round());
                },
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