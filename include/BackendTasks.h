#ifndef BACKEND_TASKS_H
#define BACKEND_TASKS_H

#include <Arduino.h>

// Funkcja inicjalizująca (np. do pobrania początkowego ID komendy z Flash)
void backendTasksSetup();

// Pobiera ustawienia z serwera i nadpisuje lokalne
void fetchAndApplyConfiguration();

// Pobiera i wykonuje komendy (np. "podlej")
// Przekazujemy poziom wody jako argument, żeby ten plik nie musiał znać struktury czujników
void fetchAndExecuteCommands(int currentWaterLevel);

#endif