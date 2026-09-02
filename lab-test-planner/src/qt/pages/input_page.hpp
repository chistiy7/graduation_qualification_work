#pragma once

#include "model/scenario_input.hpp"

#include <QWidget>

class QComboBox;
class QDoubleSpinBox;
class QGroupBox;
class QLabel;
class QPushButton;
class QSpinBox;
class QTableWidget;

namespace lab::qtui {

class InputPage : public QWidget {
    Q_OBJECT

public:
    explicit InputPage(QWidget* parent = nullptr);

    [[nodiscard]] bool isCustomMode() const;
    [[nodiscard]] QString presetId() const;
    [[nodiscard]] bool collectInput(ScenarioInput& out, QString& error) const;

signals:
    void inputChanged();

private slots:
    void onPresetChanged(int index);
    void onAddGroup();
    void onRemoveGroup();
    void onGroupsChanged();

private:
    void setupUi();
    void setCustomInputEnabled(bool enabled);
    void updateBatchHint();
    [[nodiscard]] int groupsSum() const;

    QComboBox* presetCombo_{nullptr};
    QDoubleSpinBox* roomArea_{nullptr};
    QSpinBox* batchSize_{nullptr};
    QDoubleSpinBox* laborRate_{nullptr};
    QDoubleSpinBox* specimenVolume_{nullptr};
    QDoubleSpinBox* energyTariff_{nullptr};
    QLabel* gridHint_{nullptr};
    QLabel* batchHint_{nullptr};
    QTableWidget* groupsTable_{nullptr};
    QPushButton* addGroupBtn_{nullptr};
    QPushButton* removeGroupBtn_{nullptr};
    QGroupBox* paramsBox_{nullptr};
    QGroupBox* groupsBox_{nullptr};
};

}  // namespace lab::qtui
