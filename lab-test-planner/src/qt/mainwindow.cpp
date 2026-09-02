#include "qt/mainwindow.hpp"

#include "qt/pages/input_page.hpp"
#include "qt/pages/results_page.hpp"

#include "app/paths.hpp"
#include "io/scenario_json.hpp"

#include <QApplication>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QProgressDialog>
#include <QStatusBar>
#include <QTabWidget>
#include <QToolBar>

#include <QtConcurrent/QtConcurrentRun>

namespace lab::qtui {

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    calcWatcher_ = new QFutureWatcher<CalculationJobResult>(this);
    connect(calcWatcher_, &QFutureWatcher<CalculationJobResult>::finished, this,
            &MainWindow::onCalculationFinished);

    progress_ = new QProgressDialog(this);
    progress_->setWindowTitle(QStringLiteral("LabPlanner"));
    progress_->setLabelText(QStringLiteral("Выполняется расчёт…"));
    progress_->setRange(0, 0);
    progress_->setMinimumDuration(0);
    progress_->setCancelButton(nullptr);
    progress_->setWindowModality(Qt::ApplicationModal);

    setupUi();
    setupMenuAndToolbar();
    setWindowTitle(QStringLiteral("LabPlanner — планирование испытательной лаборатории"));
    resize(1280, 860);
    statusBar()->showMessage(QStringLiteral("Готово. Задайте параметры и нажмите «Рассчитать»."));
}

MainWindow::~MainWindow() {
    if (calcWatcher_->isRunning()) {
        calcWatcher_->waitForFinished();
    }
}

void MainWindow::setupUi() {
    mainTabs_ = new QTabWidget(this);
    inputPage_ = new InputPage(mainTabs_);
    resultsPage_ = new ResultsPage(mainTabs_);
    mainTabs_->addTab(inputPage_, QStringLiteral("Задача"));
    mainTabs_->addTab(resultsPage_, QStringLiteral("Результаты"));
    mainTabs_->setTabEnabled(1, false);
    setCentralWidget(mainTabs_);
}

void MainWindow::setupMenuAndToolbar() {
    auto* fileMenu = menuBar()->addMenu(QStringLiteral("Файл"));
    auto* runMenu = menuBar()->addMenu(QStringLiteral("Расчёт"));
    auto* helpMenu = menuBar()->addMenu(QStringLiteral("Справка"));

    openAct_ = fileMenu->addAction(QStringLiteral("Открыть сценарий JSON…"));
    saveAct_ = fileMenu->addAction(QStringLiteral("Сохранить сценарий JSON…"));
    fileMenu->addSeparator();
    exportReportAct_ = fileMenu->addAction(QStringLiteral("Показать путь отчёта"));
    exportCsvAct_ = fileMenu->addAction(QStringLiteral("Показать путь CSV"));
    fileMenu->addSeparator();
    fileMenu->addAction(QStringLiteral("Выход"), this, &QWidget::close);

    calculateAct_ = runMenu->addAction(QStringLiteral("Рассчитать"));
    calculateAct_->setShortcut(Qt::Key_F5);

    helpMenu->addAction(QStringLiteral("О программе"), this, &MainWindow::onAbout);

    auto* toolbar = addToolBar(QStringLiteral("Действия"));
    toolbar->setMovable(false);
    toolbar->addAction(calculateAct_);
    toolbar->addSeparator();
    toolbar->addAction(openAct_);
    toolbar->addAction(saveAct_);
    toolbar->addSeparator();
    toolbar->addAction(exportReportAct_);
    toolbar->addAction(exportCsvAct_);

    saveAct_->setEnabled(false);
    exportReportAct_->setEnabled(false);
    exportCsvAct_->setEnabled(false);

    connect(calculateAct_, &QAction::triggered, this, &MainWindow::onCalculate);
    connect(openAct_, &QAction::triggered, this, &MainWindow::onOpenJson);
    connect(saveAct_, &QAction::triggered, this, &MainWindow::onSaveJson);
    connect(exportReportAct_, &QAction::triggered, this, &MainWindow::onExportReport);
    connect(exportCsvAct_, &QAction::triggered, this, &MainWindow::onExportCsv);
}

void MainWindow::setBusy(bool busy, const QString& status) {
    calculateAct_->setEnabled(!busy);
    openAct_->setEnabled(!busy);
    mainTabs_->setEnabled(!busy);

    if (busy) {
        progress_->show();
        statusBar()->showMessage(status.isEmpty()
                                     ? QStringLiteral("Расчёт выполняется в фоне…")
                                     : status);
    } else {
        progress_->hide();
    }
}

void MainWindow::showError(const QString& message) {
    statusBar()->showMessage(message, 8000);
    QMessageBox::critical(this, QStringLiteral("Ошибка"), message);
}

void MainWindow::applyOutput(const PipelineOutput& out, const ScenarioInput* input) {
    lastOutput_ = out;
    if (input) {
        lastInput_ = *input;
    } else {
        lastInput_.reset();
    }

    resultsPage_->showOutput(out, input);
    mainTabs_->setTabEnabled(1, true);
    mainTabs_->setCurrentIndex(1);

    saveAct_->setEnabled(true);
    exportReportAct_->setEnabled(true);
    exportCsvAct_->setEnabled(true);

    statusBar()->showMessage(
        QStringLiteral("Расчёт завершён. C = %1 руб, T_cycle = %2 мин (%3 с)")
            .arg(out.result.metrics.C, 0, 'f', 2)
            .arg(out.result.metrics.T_cycle, 0, 'f', 2)
            .arg(out.elapsedMs / 1000.0, 0, 'f', 2),
        10000);
}

void MainWindow::launchCalculation(const std::function<CalculationJobResult()>& job) {
    if (calcWatcher_->isRunning()) {
        return;
    }
    setBusy(true);
    calcWatcher_->setFuture(QtConcurrent::run(job));
}

void MainWindow::onCalculate() {
    if (calcWatcher_->isRunning()) {
        return;
    }

    const QString presetId = inputPage_->presetId();
    if (inputPage_->isCustomMode()) {
        ScenarioInput input;
        QString error;
        if (!inputPage_->collectInput(input, error)) {
            showError(error);
            return;
        }
        launchCalculation([input]() { return executeCalculation(true, {}, input); });
        return;
    }

    launchCalculation([presetId]() { return executeCalculation(false, presetId, {}); });
}

void MainWindow::onOpenJson() {
    if (calcWatcher_->isRunning()) {
        return;
    }

    const QString path = QFileDialog::getOpenFileName(
        this, QStringLiteral("Открыть сценарий JSON"), QString::fromStdString(dataDir().string()),
        QStringLiteral("JSON (*.json)"));
    if (path.isEmpty()) {
        return;
    }

    launchCalculation([path]() { return executeJsonScenario(path); });
}

void MainWindow::onCalculationFinished() {
    setBusy(false);

    const CalculationJobResult result = calcWatcher_->result();
    if (!result.ok) {
        showError(result.error);
        return;
    }

    const ScenarioInput* inputPtr =
        result.input.has_value() ? &result.input.value() : nullptr;
    applyOutput(result.output, inputPtr);
}

void MainWindow::onSaveJson() {
    if (!lastOutput_) {
        showError(QStringLiteral("Нет результатов для сохранения."));
        return;
    }

    const QString path = QFileDialog::getSaveFileName(
        this, QStringLiteral("Сохранить сценарий JSON"),
        QString::fromStdString((outputDir() / (lastOutput_->bundle.name + "_saved.json")).string()),
        QStringLiteral("JSON (*.json)"));
    if (path.isEmpty()) return;

    try {
        saveScenarioJson(path.toStdString(), lastOutput_->bundle);
        statusBar()->showMessage(QStringLiteral("Сохранено: %1").arg(path), 8000);
    } catch (const std::exception& ex) {
        showError(QString::fromUtf8(ex.what()));
    }
}

void MainWindow::onExportReport() {
    if (!lastOutput_) return;
    QMessageBox::information(
        this, QStringLiteral("Отчёт"),
        QStringLiteral("Текстовый отчёт:\n%1")
            .arg(QString::fromStdString(lastOutput_->reportPath.string())));
}

void MainWindow::onExportCsv() {
    if (!lastOutput_) return;
    QMessageBox::information(
        this, QStringLiteral("CSV"),
        QStringLiteral("Файл плана:\n%1")
            .arg(QString::fromStdString(lastOutput_->csvPath.string())));
}

void MainWindow::onAbout() {
    QMessageBox::about(
        this, QStringLiteral("LabPlanner"),
        QStringLiteral(
            "<b>LabPlanner</b> — планирование испытательной лаборатории.<br><br>"
            "Ввод параметров, оптимальное размещение стендов на сетке, "
            "подбор маршрута испытаний (min C).<br><br>"
            "Расчёт выполняется в фоне — окно не блокируется.<br>"
            "Консоль: <code>LabPlanner --cli</code>"));
}

}  // namespace lab::qtui
