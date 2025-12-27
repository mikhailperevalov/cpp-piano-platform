#include "PianoRollWidget.h"
#include "PianoKeyboardWidget.h"
#include <QPainter>
#include <algorithm>


PianoRollWidget::PianoRollWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(200);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void PianoRollWidget::setNotes(const QVector<MidiNote> &notes)
{
    m_notes = notes;
    update();
}

void PianoRollWidget::setCurrentTime(qint64 ms)
{
    m_currentTimeMs = ms;
    update();
}

QSize PianoRollWidget::minimumSizeHint() const
{
    return QSize(400, 200);
}

QSize PianoRollWidget::sizeHint() const
{
    return QSize(800, 300);
}

void PianoRollWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);

    int w = width();
    int h = height();

    // 1) Фон и градиент
    QLinearGradient bg(0, 0, 0, h);
    bg.setColorAt(0.0, QColor("#202020"));
    bg.setColorAt(1.0, QColor("#151515"));
    p.fillRect(rect(), bg);

    if (m_notes.isEmpty() || !m_keyboard)
        return;

    const qint64 windowLength = 8000;
    const qint64 tNow = m_currentTimeMs;

    p.setRenderHint(QPainter::Antialiasing, false);

    // 2) Цвета для нот
    QColor mainColor(0, 188, 212);      // #00BCD4
    QColor nearLineColor(255, 152, 0);  // #FF9800

    // 3) Цикл по нотам
    for (const auto &n : m_notes) {
        qint64 start = n.startTime;
        qint64 end   = n.startTime + n.duration;

        if (end <= tNow || start >= tNow + windowLength)
            continue;

        qint64 visibleStart = std::max(start, tNow);
        qint64 visibleEnd   = std::min(end,   tNow + windowLength);

        double tToNowStart = double(visibleStart - tNow);
        double tToNowEnd   = double(visibleEnd   - tNow);

        auto timeToY = [h, windowLength](double tToNow) {
            double ratio = tToNow / double(windowLength);
            if (ratio < 0.0) ratio = 0.0;
            if (ratio > 1.0) ratio = 1.0;
            return h - int(ratio * h);
        };

        int yBottom = timeToY(tToNowStart);
        int yTop    = timeToY(tToNowEnd);
        if (yTop > yBottom)
            std::swap(yTop, yBottom);

        QRect keyR = m_keyboard->keyRect(n.pitch);
        if (!keyR.isValid())
            continue;

        int noteX     = keyR.x();
        int noteWidth = keyR.width();

        QRect r(noteX, yTop, noteWidth, yBottom - yTop);

        // 4) Выбор цвета ноты
        QColor color = mainColor;
        if (visibleStart <= tNow + 150 && visibleEnd >= tNow) {
            // нота прямо над клавиатурой
            color = nearLineColor;
        }

        p.setPen(Qt::NoPen);
        p.setBrush(color);
        p.drawRect(r);
    }

    // 5) Линия текущего времени (у клавиатуры)
    p.setPen(QPen(QColor("#FF9800"), 2));
    p.drawLine(0, h - 1, w, h - 1);
}

void PianoRollWidget::setKeyboard(PianoKeyboardWidget *keyboard)
{
    m_keyboard = keyboard;
    update();
}
