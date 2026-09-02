#include "qt/pages/results_page.hpp"

#include "app/scenario_runner.hpp"
#include "engine/layout_area.hpp"
#include "qt/widgets/layout_grid_widget.hpp"

#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPlainTextEdit>
#include <QScrollArea>
#include <QTabWidget>
#include <QTableWidget>
#include <QVBoxLayout>

#include <utility>

namespace lab::qtui {

ResultsPage::ResultsPage(QWidget* parent) : QWidget(parent) { setupUi(); }

void ResultsPage::setupUi() {
    auto* root = new QVBoxLayout(this);
    tabs_ = new QTabWidget(this);

    summaryPanel_ = new QWidget(tabs_);
    auto* summaryScroll = new QScrollArea(tabs_);
    summaryScroll->setWidgetResizable(true);
    summaryScroll->setWidget(summaryPanel_);
    summaryForm_ = new QFormLayout(summaryPanel_);
    summaryForm_->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    layoutNoteLabel_ = new QLabel(summaryPanel_);
    layoutNoteLabel_->setWordWrap(true);
    elapsedLabel_ = new QLabel(summaryPanel_);
    reportPathLabel_ = new QLabel(summaryPanel_);
    reportPathLabel_->setWordWrap(true);
    reportPathLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    csvPathLabel_ = new QLabel(summaryPanel_);
    csvPathLabel_->setWordWrap(true);
    csvPathLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    tabs_->addTab(summaryScroll, QStringLiteral("Сводка"));

    costTable_ = new QTableWidget(0, 2, tabs_);
    costTable_->setHorizontalHeaderLabels(
        {QStringLiteral("Статья"), QStringLiteral("Сумма, руб")});
    costTable_->horizontalHeader()->setStretchLastSection(true);
    costTable_->verticalHeader()->setVisible(false);
    costTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tabs_->addTab(costTable_, QStringLiteral("Стоимость"));

    equipmentTable_ = new QTableWidget(0, 8, tabs_);
    equipmentTable_->setHorizontalHeaderLabels(
        {QStringLiteral("Стенд"), QStringLiteral("t_test"), QStringLiteral("t_setup"),
         QStringLiteral("t_busy"), QStringLiteral("t_idle"), QStringLiteral("η, %"),
         QStringLiteral("C_setup"), QStringLiteral("C_amort")});
    equipmentTable_->horizontalHeader()->setStretchLastSection(true);
    equipmentTable_->verticalHeader()->setVisible(false);
    equipmentTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tabs_->addTab(equipmentTable_, QStringLiteral("Стенды"));

    routeTable_ = new QTableWidget(0, 4, tabs_);
    routeTable_->setHorizontalHeaderLabels(
        {QStringLiteral("#"), QStringLiteral("Образец"), QStringLiteral("Операция"),
         QStringLiteral("Стенд")});
    routeTable_->horizontalHeader()->setStretchLastSection(true);
    routeTable_->verticalHeader()->setVisible(false);
    routeTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tabs_->addTab(routeTable_, QStringLiteral("Маршрут"));

    auto* layoutTab = new QWidget(tabs_);
    auto* layoutLayout = new QVBoxLayout(layoutTab);
    layoutGrid_ = new LayoutGridWidget(layoutTab);
    areaStatsLabel_ = new QLabel(layoutTab);
    areaStatsLabel_->setWordWrap(true);
    layoutLayout->addWidget(layoutGrid_, 1);
    layoutLayout->addWidget(areaStatsLabel_);
    tabs_->addTab(layoutTab, QStringLiteral("Планировка"));

    fullReport_ = new QPlainTextEdit(tabs_);
    fullReport_->setReadOnly(true);
    fullReport_->setPlaceholderText(QStringLiteral("Полный текст отчёта (как в CLI)"));
    tabs_->addTab(fullReport_, QStringLiteral("Полный отчёт"));

    root->addWidget(tabs_);
}

QString ResultsPage::rub(double value) {
    return QString::number(value, 'f', 2);
}

void ResultsPage::clearOutput() {
    while (summaryForm_->rowCount() > 0) {
        summaryForm_->removeRow(0);
    }
    layoutNoteLabel_->clear();
    elapsedLabel_->clear();
    reportPathLabel_->clear();
    csvPathLabel_->clear();
    costTable_->setRowCount(0);
    equipmentTable_->setRowCount(0);
    routeTable_->setRowCount(0);
    layoutGrid_->clearLayout();
    areaStatsLabel_->clear();
    fullReport_->clear();
}

void ResultsPage::showOutput(const PipelineOutput& out, const ScenarioInput* input) {
    clearOutput();
    fillSummary(out);
    fillCostTable(out);
    fillEquipmentTable(out);
    fillRouteTable(out);
    fillLayoutTab(out);
    fullReport_->setPlainText(
        QString::fromStdString(formatScenarioRunOutput(out, input)));

    if (out.layoutEvaluated > 0) {
        layoutNoteLabel_->setText(
            QStringLiteral("Размещение: %1 (вариантов: %2)")
                .arg(QString::fromStdString(out.layoutNote))
                .arg(out.layoutEvaluated));
        summaryForm_->addRow(QStringLiteral("Размещение:"), layoutNoteLabel_);
    }
    if (out.elapsedMs >= 0.0) {
        elapsedLabel_->setText(QStringLiteral("%1 мс").arg(out.elapsedMs, 0, 'f', 2));
        summaryForm_->addRow(QStringLiteral("Время расчёта:"), elapsedLabel_);
    }
    reportPathLabel_->setText(QString::fromStdString(out.reportPath.string()));
    summaryForm_->addRow(QStringLiteral("Отчёт:"), reportPathLabel_);
    csvPathLabel_->setText(QString::fromStdString(out.csvPath.string()));
    summaryForm_->addRow(QStringLiteral("CSV:"), csvPathLabel_);
}

void ResultsPage::fillSummary(const PipelineOutput& out) {
    const auto& m = out.result.metrics;
    const auto add = [&](const QString& name, const QString& value) {
        summaryForm_->addRow(name, new QLabel(value, summaryPanel_));
    };

    add(QStringLiteral("T_sum, мин"), rub(m.T_sum));
    add(QStringLiteral("T_cycle, мин"), rub(m.T_cycle));
    add(QStringLiteral("N (переналадки)"), QString::number(m.N));
    add(QStringLiteral("L (шаги ячеек)"), rub(m.L));
    add(QStringLiteral("η_avg, %"), rub(m.etaAvg * 100.0));
    add(QStringLiteral("C, руб"), rub(m.C));
    if (!m.bottleneckEquipmentId.empty()) {
        add(QStringLiteral("Узкое место"),
            QString::fromStdString(m.bottleneckEquipmentId));
    }
    add(QStringLiteral("Порядок операций"),
        QString::fromStdString(out.result.orderingNote));
    add(QStringLiteral("Суммарный простой, мин"), rub(m.totalIdleMin));

    const auto& pt = out.result.efficiency.time;
    add(QStringLiteral("T_sum (эффективность), мин"), rub(pt.workTimeMin));
    add(QStringLiteral("Индекс параллельности"), rub(pt.parallelismIndex));

    if (!out.result.energyWarnings.empty()) {
        QString warnings;
        for (const auto& w : out.result.energyWarnings) {
            if (!warnings.isEmpty()) warnings += QStringLiteral("\n");
            warnings += QString::fromStdString(w);
        }
        auto* warnLabel = new QLabel(warnings, summaryPanel_);
        warnLabel->setWordWrap(true);
        warnLabel->setStyleSheet(QStringLiteral("color: #c62828;"));
        summaryForm_->addRow(QStringLiteral("Предупреждения:"), warnLabel);
    }
}

void ResultsPage::fillCostTable(const PipelineOutput& out) {
    const auto& c = out.result.metrics.cost;
    const std::pair<QString, double> rows[] = {
        {QStringLiteral("Подготовка образцов (труд)"), c.prepLabor},
        {QStringLiteral("Материалы испытаний"), c.operationMaterials},
        {QStringLiteral("Энергия испытаний (работа)"), c.energyWork},
        {QStringLiteral("Труд по операциям"), c.operationLabor},
        {QStringLiteral("Перенастройки стендов"), c.setup},
        {QStringLiteral("Энергия переналадки"), c.energySetup},
        {QStringLiteral("Амортизация стендов"), c.amortization},
        {QStringLiteral("Транспорт (перемещения)"), c.transport},
        {QStringLiteral("Занятость площади (C_area)"), c.area},
        {QStringLiteral("Итого C"), c.total()},
    };

    costTable_->setRowCount(static_cast<int>(std::size(rows)));
    for (int i = 0; i < costTable_->rowCount(); ++i) {
        costTable_->setItem(i, 0, new QTableWidgetItem(rows[i].first));
        costTable_->setItem(i, 1, new QTableWidgetItem(rub(rows[i].second)));
    }
}

void ResultsPage::fillEquipmentTable(const PipelineOutput& out) {
    const auto& stats = out.result.metrics.equipmentStats;
    equipmentTable_->setRowCount(static_cast<int>(stats.size()));
    for (int i = 0; i < equipmentTable_->rowCount(); ++i) {
        const auto& u = stats[static_cast<size_t>(i)];
        equipmentTable_->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(u.equipmentId)));
        equipmentTable_->setItem(i, 1, new QTableWidgetItem(rub(u.testMin)));
        equipmentTable_->setItem(i, 2, new QTableWidgetItem(rub(u.setupMin)));
        equipmentTable_->setItem(i, 3, new QTableWidgetItem(rub(u.busyMin)));
        equipmentTable_->setItem(i, 4, new QTableWidgetItem(rub(u.idleMin)));
        equipmentTable_->setItem(i, 5, new QTableWidgetItem(rub(u.utilization * 100.0)));
        equipmentTable_->setItem(i, 6, new QTableWidgetItem(rub(u.costSetup)));
        equipmentTable_->setItem(i, 7, new QTableWidgetItem(rub(u.costAmort)));
    }
}

void ResultsPage::fillRouteTable(const PipelineOutput& out) {
    const auto& steps = out.result.route.steps();
    routeTable_->setRowCount(static_cast<int>(steps.size()));
    for (int i = 0; i < routeTable_->rowCount(); ++i) {
        const auto& s = steps[static_cast<size_t>(i)];
        routeTable_->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1)));
        routeTable_->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(s.specimenId)));
        routeTable_->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(s.operationId)));
        routeTable_->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(s.equipmentId)));
    }
}

void ResultsPage::fillLayoutTab(const PipelineOutput& out) {
    layoutGrid_->setLayoutData(out.bundle.problem, out.result.route);

    const auto stats = computeLayoutAreaStats(out.bundle.problem);
    const double cArea =
        out.result.metrics.T_cycle > 0.0
            ? areaOccupancyCost(out.bundle.problem, out.result.metrics.T_cycle)
            : 0.0;

    areaStatsLabel_->setText(
        QStringLiteral("equipment_cells=%1  reserved=%2  free=%3  used_area=%4 м²  "
                       "T_cycle=%5 ч  C_area=%6 руб")
            .arg(stats.equipmentCells)
            .arg(stats.reservedCells)
            .arg(stats.freeCells)
            .arg(stats.usedAreaM2, 0, 'f', 2)
            .arg(out.result.metrics.T_cycle / 60.0, 0, 'f', 2)
            .arg(cArea, 0, 'f', 2));
}

}  // namespace lab::qtui
