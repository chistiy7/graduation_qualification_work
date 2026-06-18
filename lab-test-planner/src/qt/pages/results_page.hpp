#pragma once

#include "app/pipeline.hpp"
#include "model/scenario_input.hpp"

#include <QWidget>

class QFormLayout;
class QLabel;
class QPlainTextEdit;
class QScrollArea;
class QTabWidget;
class QTableWidget;

namespace lab::qtui {

class LayoutGridWidget;

class ResultsPage : public QWidget {
    Q_OBJECT

public:
    explicit ResultsPage(QWidget* parent = nullptr);

    void showOutput(const PipelineOutput& out, const ScenarioInput* input);
    void clearOutput();

private:
    void setupUi();
    void fillSummary(const PipelineOutput& out);
    void fillCostTable(const PipelineOutput& out);
    void fillEquipmentTable(const PipelineOutput& out);
    void fillRouteTable(const PipelineOutput& out);
    void fillLayoutTab(const PipelineOutput& out);
    static QString rub(double value);

    QTabWidget* tabs_{nullptr};
    QWidget* summaryPanel_{nullptr};
    QFormLayout* summaryForm_{nullptr};
    QLabel* layoutNoteLabel_{nullptr};
    QLabel* elapsedLabel_{nullptr};
    QLabel* reportPathLabel_{nullptr};
    QLabel* csvPathLabel_{nullptr};
    QTableWidget* costTable_{nullptr};
    QTableWidget* equipmentTable_{nullptr};
    QTableWidget* routeTable_{nullptr};
    LayoutGridWidget* layoutGrid_{nullptr};
    QLabel* areaStatsLabel_{nullptr};
    QPlainTextEdit* fullReport_{nullptr};
};

}  // namespace lab::qtui
