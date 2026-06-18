#include "qt/widgets/layout_grid_widget.hpp"

#include "domain/laboratory.hpp"
#include "domain/stand_catalog.hpp"

#include <QFontMetrics>
#include <QPainter>
#include <QToolTip>

#include <algorithm>
#include <map>

namespace lab::qtui {

namespace {

constexpr int kVoid = 0;
constexpr int kPassage = -3;
constexpr int kBuffer = -2;
constexpr int kRoute = -1;

const LabEquipment* findEquipment(const ProblemDefinition& p, const std::string& id) {
    for (const auto& e : p.equipment) {
        if (e.id == id) return &e;
    }
    return nullptr;
}

}  // namespace

LayoutGridWidget::LayoutGridWidget(QWidget* parent) : QWidget(parent) {
    setMinimumSize(320, 240);
    setMouseTracking(true);
}

void LayoutGridWidget::clearLayout() {
    hasGrid_ = false;
    grid_.clear();
    update();
}

void LayoutGridWidget::setLayoutData(const ProblemDefinition& problem, const TestRoute& route) {
    problem_ = problem;
    route_ = route;
    rebuildGrid();
    update();
}

void LayoutGridWidget::rebuildGrid() {
    hasGrid_ = false;
    grid_.clear();

    const auto& laboratory = problem_.laboratory;
    if (laboratory.cells().empty()) return;

    std::map<StandType, int> typeCode;
    int nextCode = 1;
    for (const auto& e : problem_.equipment) {
        if (typeCode.find(e.standType) == typeCode.end()) {
            typeCode[e.standType] = nextCode++;
        }
    }

    int maxRow = 0, maxCol = 0;
    int minRow = 0, minCol = 0;
    bool first = true;
    for (const auto& c : laboratory.cells()) {
        if (first) {
            minRow = maxRow = c.row;
            minCol = maxCol = c.col;
            first = false;
        } else {
            minRow = std::min(minRow, c.row);
            maxRow = std::max(maxRow, c.row);
            minCol = std::min(minCol, c.col);
            maxCol = std::max(maxCol, c.col);
        }
    }
    minRow_ = minRow;
    minCol_ = minCol;

    const int rows = maxRow - minRow + 1;
    const int cols = maxCol - minCol + 1;
    grid_.assign(rows, std::vector<CellView>(cols, CellView{}));

    for (const auto& c : laboratory.cells()) {
        const int gr = c.row - minRow;
        const int gc = c.col - minCol;
        if (c.kind == LabCellKind::Buffer || c.kind == LabCellKind::Forbidden) {
            grid_[gr][gc].code = kBuffer;
            grid_[gr][gc].tooltip = QStringLiteral("Зона безопасности (X)");
        } else if (c.kind != LabCellKind::Stand) {
            grid_[gr][gc].code = kPassage;
            grid_[gr][gc].tooltip = QStringLiteral("Проход (.)");
        }
    }

    std::map<std::string, std::pair<int, int>> equipmentCoord;
    for (const auto& [eqId, cellId] : laboratory.equipmentPlacements()) {
        if (const auto* c = laboratory.cell(cellId)) {
            equipmentCoord[eqId] = {c->row, c->col};
            if (const auto* e = findEquipment(problem_, eqId)) {
                const int gr = c->row - minRow;
                const int gc = c->col - minCol;
                grid_[gr][gc].code = typeCode[e->standType];
                grid_[gr][gc].tooltip =
                    QString::fromStdString(e->nameRu + " [" + e->id + "]");
            }
        }
    }

    auto markRoute = [&](int ar, int ac, int br, int bc) {
        int r = ar, c = ac;
        while (c != bc) {
            c += (bc > c) ? 1 : -1;
            const int gr = r - minRow, gc = c - minCol;
            if (gr >= 0 && gr < rows && gc >= 0 && gc < cols &&
                grid_[gr][gc].code == kPassage) {
                grid_[gr][gc].code = kRoute;
                grid_[gr][gc].tooltip = QStringLiteral("Маршрут (R)");
            }
        }
        while (r != br) {
            r += (br > r) ? 1 : -1;
            const int gr = r - minRow, gc = c - minCol;
            if (gr >= 0 && gr < rows && gc >= 0 && gc < cols &&
                grid_[gr][gc].code == kPassage) {
                grid_[gr][gc].code = kRoute;
                grid_[gr][gc].tooltip = QStringLiteral("Маршрут (R)");
            }
        }
    };

    const auto& steps = route_.steps();
    for (size_t i = 1; i < steps.size(); ++i) {
        const auto prev = equipmentCoord.find(steps[i - 1].equipmentId);
        const auto cur = equipmentCoord.find(steps[i].equipmentId);
        if (prev == equipmentCoord.end() || cur == equipmentCoord.end()) continue;
        if (prev->second.first == cur->second.first &&
            prev->second.second == cur->second.second) {
            continue;
        }
        markRoute(prev->second.first, prev->second.second, cur->second.first,
                  cur->second.second);
    }

    hasGrid_ = true;
}

QColor LayoutGridWidget::colorForCode(int code) const {
    switch (code) {
        case kPassage:
            return QColor(235, 238, 245);
        case kBuffer:
            return QColor(120, 125, 140);
        case kRoute:
            return QColor(255, 193, 94);
        default:
            break;
    }
    if (code > 0) {
        static const QColor palette[] = {
            QColor(66, 133, 244),   QColor(219, 68, 55),   QColor(244, 180, 0),
            QColor(15, 157, 88),    QColor(171, 71, 188),  QColor(0, 172, 193),
            QColor(255, 112, 67),   QColor(92, 107, 192),
        };
        return palette[(code - 1) % 8];
    }
    return QColor(200, 200, 200);
}

void LayoutGridWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.fillRect(rect(), QColor(248, 249, 252));

    if (!hasGrid_ || grid_.empty()) {
        painter.setPen(QColor(100, 100, 110));
        painter.drawText(rect(), Qt::AlignCenter,
                         QStringLiteral("Карта размещения: сетка не задана"));
        return;
    }

    const int rows = static_cast<int>(grid_.size());
    const int cols = static_cast<int>(grid_[0].size());
    const int margin = 36;
    const int cellW = std::max(18, (width() - margin * 2) / cols);
    const int cellH = std::max(18, (height() - margin * 2) / rows);
    const int originX = margin;
    const int originY = margin;

    painter.setPen(QColor(80, 80, 90));
    painter.drawText(originX, originY - 18,
                     QStringLiteral("Сетка %1×%2 м, ячеек %3×%4")
                         .arg(problem_.gridCellSizeM)
                         .arg(problem_.gridCellSizeM)
                         .arg(rows)
                         .arg(cols));

    for (int c = 0; c < cols; ++c) {
        painter.drawText(originX + c * cellW + cellW / 3, originY - 4,
                         QString::number((minCol_ + c) % 10));
    }

    for (int r = 0; r < rows; ++r) {
        painter.drawText(8, originY + r * cellH + cellH * 2 / 3,
                         QString::number(minRow_ + r));
        for (int c = 0; c < cols; ++c) {
            const QRect cellRect(originX + c * cellW, originY + r * cellH, cellW - 2,
                                 cellH - 2);
            const int code = grid_[r][c].code;
            painter.fillRect(cellRect, colorForCode(code));
            painter.setPen(QColor(60, 60, 70));
            painter.drawRect(cellRect);

            QString label;
            if (code == kPassage) {
                label = QStringLiteral("·");
            } else if (code == kBuffer) {
                label = QStringLiteral("X");
            } else if (code == kRoute) {
                label = QStringLiteral("R");
            } else if (code > 0) {
                label = QString::number(code);
            }
            if (!label.isEmpty()) {
                painter.setPen(code == kPassage ? QColor(140, 145, 155) : Qt::white);
                painter.drawText(cellRect, Qt::AlignCenter, label);
            }
        }
    }

    int y = originY + rows * cellH + 12;
    painter.setPen(QColor(70, 70, 80));
    const QString legend =
        QStringLiteral(". проход   X зона безопасности   R маршрут   1…K стенды");
    painter.drawText(originX, y, legend);
}

QSize LayoutGridWidget::sizeHint() const { return {640, 480}; }

QSize LayoutGridWidget::minimumSizeHint() const { return {320, 240}; }

}  // namespace lab::qtui
