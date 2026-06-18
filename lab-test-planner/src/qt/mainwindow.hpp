#pragma once

#include "app/pipeline.hpp"
#include "model/scenario_input.hpp"
#include "qt/calculation_worker.hpp"

#include <QFutureWatcher>
#include <QMainWindow>
#include <functional>
#include <optional>

class QAction;
class QProgressDialog;
class QTabWidget;

namespace lab::qtui {

class InputPage;
class ResultsPage;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onCalculate();
    void onOpenJson();
    void onSaveJson();
    void onExportReport();
    void onExportCsv();
    void onAbout();
    void onCalculationFinished();

private:
    void setupUi();
    void setupMenuAndToolbar();
    void applyOutput(const PipelineOutput& out, const ScenarioInput* input);
    void showError(const QString& message);
    void setBusy(bool busy, const QString& status = QString());
    void launchCalculation(const std::function<CalculationJobResult()>& job);

    QTabWidget* mainTabs_{nullptr};
    InputPage* inputPage_{nullptr};
    ResultsPage* resultsPage_{nullptr};
    QProgressDialog* progress_{nullptr};
    QFutureWatcher<CalculationJobResult>* calcWatcher_{nullptr};

    QAction* calculateAct_{nullptr};
    QAction* openAct_{nullptr};
    QAction* saveAct_{nullptr};
    QAction* exportReportAct_{nullptr};
    QAction* exportCsvAct_{nullptr};

    std::optional<PipelineOutput> lastOutput_;
    std::optional<ScenarioInput> lastInput_;
};

}  // namespace lab::qtui
