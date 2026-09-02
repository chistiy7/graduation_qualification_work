#pragma once

#include <string>
#include <utility>
#include <vector>

namespace lab {

// Фаза операции в расписании. Прогон (Run) автономен: стенд занят, оператор свободен
// и может обслуживать другие стенды (настройка/снятие).
enum class SchedulePhase { Setup, Run, Unload };

struct ScheduledPhase {
    std::string specimenId;
    std::string operationId;
    std::string equipmentId;
    SchedulePhase phase{};
    double startMin = 0.0;
    double endMin = 0.0;
    bool operatorBusy = false;  // Setup/Unload — занят оператор; Run — нет
};

// Расписание DES-ядра: одна модель планирования вместо линейного маршрута.
struct Schedule {
    std::vector<ScheduledPhase> phases;
    double makespanMin = 0.0;        // T_cycle — длительность партии по «настенным» часам
    double operatorBusyMin = 0.0;    // занятость единственного оператора (настройка+снятие+ход)
    double operatorIdleMin = 0.0;    // простой оператора = makespan − занятость
    double operatorTravelSteps = 0.0;  // L — суммарный ход оператора, шаги ячеек
    int changeoverCount = 0;         // N — первичные установки + смены программы
    std::vector<std::pair<std::string, double>> standBusyMin;  // занятость по стендам
};

}  // namespace lab
