#pragma once

// Физические и тарифные константы, используемые в расчёте стоимости электроэнергии (§2.1, §3.2).

namespace lab {

// Модуль упругости стали (Юнг), ГПа (ГОСТ 14249-89, §3.4, сталь конструкционная).
constexpr double E_STEEL_GPA = 200.0;

// Плотность стали, кг/м³ (ср. для конструкционных сталей).
constexpr double RHO_STEEL_KG_M3 = 7850.0;

// Удельная теплоёмкость стали, Дж/(кг·К) (ср. значение в диапазоне 20–600 °C).
constexpr double CP_STEEL_J_KG_K = 500.0;

// Тариф на электроэнергию по умолчанию, руб/кВт·ч (промышленный, базовый уровень 2024 г.).
constexpr double ENERGY_TARIFF_DEFAULT_RUB_KWH = 6.5;

// Коэффициент мощности при переналадке: P_setup = K × P_nom.
constexpr double K_POWER_SETUP = 0.2;

// Аренда площади лаборатории по умолчанию, руб/(м²·ч) (global_fix.md §6).
constexpr double RENT_RATE_DEFAULT_RUB_M2_HOUR = 2.8;

}  // namespace lab
