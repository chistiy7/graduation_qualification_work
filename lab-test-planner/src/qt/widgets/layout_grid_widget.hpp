#pragma once

#include "domain/test_route.hpp"
#include "model/problem.hpp"

#include <QWidget>

#include <vector>

namespace lab::qtui {

// Визуализация карты размещения (данные из ProblemDefinition + маршрут).
class LayoutGridWidget : public QWidget {
    Q_OBJECT

public:
    explicit LayoutGridWidget(QWidget* parent = nullptr);

    void setLayoutData(const ProblemDefinition& problem, const TestRoute& route);
    void clearLayout();

protected:
    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    struct CellView {
        int code = 0;  // 0 void, -1 route, -2 buffer, -3 passage, >0 stand
        QString tooltip;
    };

    void rebuildGrid();
    [[nodiscard]] QColor colorForCode(int code) const;

    ProblemDefinition problem_;
    TestRoute route_;
    std::vector<std::vector<CellView>> grid_;
    int minRow_ = 0;
    int minCol_ = 0;
    bool hasGrid_ = false;
};

}  // namespace lab::qtui
