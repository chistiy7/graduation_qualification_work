#include "qt/pages/input_page.hpp"

#include "model/scenario_input.hpp"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QVBoxLayout>

namespace lab::qtui {

namespace {

QComboBox* makeTestTypeCombo(QWidget* parent) {
    auto* combo = new QComboBox(parent);
    for (const auto type : selectableTestTypes()) {
        combo->addItem(QString::fromStdString(testTypeNameRu(type)),
                       static_cast<int>(type));
    }
    return combo;
}

}  // namespace

InputPage::InputPage(QWidget* parent) : QWidget(parent) { setupUi(); }

void InputPage::setupUi() {
    auto* root = new QVBoxLayout(this);

    auto* presetBox = new QGroupBox(QStringLiteral("Сценарий"), this);
    auto* presetLayout = new QFormLayout(presetBox);
    presetCombo_ = new QComboBox(presetBox);
    presetCombo_->addItem(QStringLiteral("Пользовательский ввод (--cli)"),
                          QStringLiteral("custom"));
    presetCombo_->addItem(QStringLiteral("Debug: demo_simple"), QStringLiteral("demo_simple"));
    presetCombo_->addItem(QStringLiteral("Debug: demo_two_specimens"),
                          QStringLiteral("demo_two_specimens"));
    presetCombo_->addItem(QStringLiteral("Debug: demo_8_types_80"),
                          QStringLiteral("demo_8_types_80"));
    presetLayout->addRow(QStringLiteral("Режим:"), presetCombo_);
    root->addWidget(presetBox);

    paramsBox_ = new QGroupBox(QStringLiteral("Параметры помещения и партии"), this);
    auto* paramsLayout = new QFormLayout(paramsBox_);

    roomArea_ = new QDoubleSpinBox(paramsBox_);
    roomArea_->setRange(0.01, 1'000'000.0);
    roomArea_->setDecimals(2);
    roomArea_->setValue(100.0);
    paramsLayout->addRow(QStringLiteral("Площадь помещения, м²"), roomArea_);

    batchSize_ = new QSpinBox(paramsBox_);
    batchSize_->setRange(1, 1'000'000);
    batchSize_->setValue(10);
    paramsLayout->addRow(QStringLiteral("Общее число образцов"), batchSize_);

    laborRate_ = new QDoubleSpinBox(paramsBox_);
    laborRate_->setRange(0.01, 1'000'000.0);
    laborRate_->setDecimals(2);
    laborRate_->setValue(450.0);
    paramsLayout->addRow(QStringLiteral("Ставка труда, руб/ч"), laborRate_);

    specimenVolume_ = new QDoubleSpinBox(paramsBox_);
    specimenVolume_->setRange(1e-12, 1.0);
    specimenVolume_->setDecimals(12);
    specimenVolume_->setValue(3.93e-6);
    paramsLayout->addRow(QStringLiteral("Объём образца, м³"), specimenVolume_);

    energyTariff_ = new QDoubleSpinBox(paramsBox_);
    energyTariff_->setRange(0.01, 1'000.0);
    energyTariff_->setDecimals(2);
    energyTariff_->setValue(6.5);
    paramsLayout->addRow(QStringLiteral("Тариф электроэнергии, руб/кВт·ч"), energyTariff_);

    gridHint_ = new QLabel(paramsBox_);
    gridHint_->setWordWrap(true);
    paramsLayout->addRow(gridHint_);

    auto* volumeHint =
        new QLabel(QStringLiteral("Подсказка: d₀=10 мм, l₀=50 мм → ≈3.93e-6 м³"), paramsBox_);
    volumeHint->setWordWrap(true);
    paramsLayout->addRow(volumeHint);

    root->addWidget(paramsBox_);

    groupsBox_ = new QGroupBox(QStringLiteral("Группы образцов по видам испытаний"), this);
    auto* groupsLayout = new QVBoxLayout(groupsBox_);

    batchHint_ = new QLabel(groupsBox_);
    batchHint_->setWordWrap(true);
    groupsLayout->addWidget(batchHint_);

    groupsTable_ = new QTableWidget(0, 2, groupsBox_);
    groupsTable_->setHorizontalHeaderLabels(
        {QStringLiteral("Число образцов"), QStringLiteral("Вид испытания")});
    groupsTable_->horizontalHeader()->setStretchLastSection(true);
    groupsLayout->addWidget(groupsTable_);

    auto* groupBtnRow = new QHBoxLayout();
    addGroupBtn_ = new QPushButton(QStringLiteral("Добавить группу"), groupsBox_);
    removeGroupBtn_ = new QPushButton(QStringLiteral("Удалить группу"), groupsBox_);
    groupBtnRow->addWidget(addGroupBtn_);
    groupBtnRow->addWidget(removeGroupBtn_);
    groupBtnRow->addStretch();
    groupsLayout->addLayout(groupBtnRow);

    root->addWidget(groupsBox_);
    root->addStretch();

    connect(presetCombo_, &QComboBox::currentIndexChanged, this, &InputPage::onPresetChanged);
    connect(addGroupBtn_, &QPushButton::clicked, this, &InputPage::onAddGroup);
    connect(removeGroupBtn_, &QPushButton::clicked, this, &InputPage::onRemoveGroup);
    connect(batchSize_, &QSpinBox::valueChanged, this, &InputPage::onGroupsChanged);
    connect(roomArea_, &QDoubleSpinBox::valueChanged, this, &InputPage::onGroupsChanged);

    onAddGroup();
    onPresetChanged(0);
}

bool InputPage::isCustomMode() const {
    return presetCombo_->currentData().toString() == QStringLiteral("custom");
}

QString InputPage::presetId() const { return presetCombo_->currentData().toString(); }

void InputPage::setCustomInputEnabled(bool enabled) {
    paramsBox_->setEnabled(enabled);
    groupsBox_->setEnabled(enabled);
}

void InputPage::updateBatchHint() {
    ScenarioInput preview{};
    preview.roomAreaM2 = roomArea_->value();
    preview.cellSizeM = 2.0;
    const int rows = gridRowsFromInput(preview);
    const int cols = gridColsFromInput(preview);
    gridHint_->setText(
        QString::fromStdString(formatGridSetupNote(preview)).replace(QLatin1Char('\n'), QStringLiteral(" ")));

    const int sum = groupsSum();
    const int total = batchSize_->value();
    const int diff = total - sum;
    if (diff == 0) {
        batchHint_->setText(QStringLiteral("Распределено %1 из %2 образцов.")
                                .arg(sum)
                                .arg(total));
        batchHint_->setStyleSheet(QStringLiteral("color: #2e7d32;"));
    } else {
        batchHint_->setText(QStringLiteral("Распределено %1 из %2 (осталось %3).")
                                .arg(sum)
                                .arg(total)
                                .arg(diff));
        batchHint_->setStyleSheet(QStringLiteral("color: #c62828;"));
    }
}

int InputPage::groupsSum() const {
    int sum = 0;
    for (int row = 0; row < groupsTable_->rowCount(); ++row) {
        if (const auto* spin = qobject_cast<QSpinBox*>(groupsTable_->cellWidget(row, 0))) {
            sum += spin->value();
        }
    }
    return sum;
}

void InputPage::onPresetChanged(int index) {
    Q_UNUSED(index);
    setCustomInputEnabled(isCustomMode());
    updateBatchHint();
    emit inputChanged();
}

void InputPage::onAddGroup() {
    const int row = groupsTable_->rowCount();
    groupsTable_->insertRow(row);

    auto* countSpin = new QSpinBox(groupsTable_);
    countSpin->setRange(1, 1'000'000);
    countSpin->setValue(1);
    groupsTable_->setCellWidget(row, 0, countSpin);
    groupsTable_->setCellWidget(row, 1, makeTestTypeCombo(groupsTable_));
    connect(countSpin, &QSpinBox::valueChanged, this, &InputPage::onGroupsChanged);
    onGroupsChanged();
}

void InputPage::onRemoveGroup() {
    const int row = groupsTable_->currentRow();
    if (row >= 0) {
        groupsTable_->removeRow(row);
    } else if (groupsTable_->rowCount() > 0) {
        groupsTable_->removeRow(groupsTable_->rowCount() - 1);
    }
    onGroupsChanged();
}

void InputPage::onGroupsChanged() {
    updateBatchHint();
    emit inputChanged();
}

bool InputPage::collectInput(ScenarioInput& out, QString& error) const {
    out = ScenarioInput{};
    out.roomAreaM2 = roomArea_->value();
    out.batchSize = batchSize_->value();
    out.laborRatePerHour = laborRate_->value();
    out.specimenVolumeM3 = specimenVolume_->value();
    out.energyTariffPerKwh = energyTariff_->value();
    out.cellSizeM = 2.0;

    if (groupsTable_->rowCount() == 0) {
        error = QStringLiteral("Добавьте хотя бы одну группу образцов.");
        return false;
    }

    int sum = 0;
    for (int row = 0; row < groupsTable_->rowCount(); ++row) {
        const auto* countSpin = qobject_cast<QSpinBox*>(groupsTable_->cellWidget(row, 0));
        const auto* typeCombo = qobject_cast<QComboBox*>(groupsTable_->cellWidget(row, 1));
        if (!countSpin || !typeCombo) {
            error = QStringLiteral("Некорректная строка группы образцов.");
            return false;
        }
        SpecimenGroup g;
        g.count = countSpin->value();
        g.testType = static_cast<TestOperationType>(typeCombo->currentData().toInt());
        sum += g.count;
        out.groups.push_back(g);
    }

    if (sum != out.batchSize) {
        error = QStringLiteral("Сумма образцов по группам (%1) не равна общему числу (%2).")
                    .arg(sum)
                    .arg(out.batchSize);
        return false;
    }
    return true;
}

}  // namespace lab::qtui
