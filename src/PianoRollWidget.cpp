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
    p.fillRect(rect(), QColor("#111111"));

    if (m_notes.isEmpty() || !m_keyboard)
        return;

    int w = width();
    int h = height();

    const qint64 windowLength = 8000; // сколько мс видно над клавиатурой
    const qint64 tNow = m_currentTimeMs;

    p.setRenderHint(QPainter::Antialiasing, false);

    for (const auto &n : m_notes) {
        qint64 start = n.startTime;
        qint64 end   = n.startTime + n.duration;

        // Ноты, которые полностью «ниже» текущего момента (уже сыграны) — не рисуем
        if (end <= tNow)
            continue;

        // Ноты, которые начинаются слишком далеко в будущем — тоже не рисуем
        if (start >= tNow + windowLength)
            continue;

        // Ограничиваем интервал отображения окном
        qint64 visibleStart = std::max(start, tNow);
        qint64 visibleEnd   = std::min(end,   tNow + windowLength);

        // Время до текущего момента (0 внизу, windowLength наверху)
        double tToNowStart = double(visibleStart - tNow); // от 0 до windowLength
        double tToNowEnd   = double(visibleEnd   - tNow);

        // Преобразуем во вертикальные координаты:
        // tToNow = 0   → y = h      (у клавиатуры)
        // tToNow = L   → y = 0      (верх окна)
        auto timeToY = [h, windowLength](double tToNow) {
            double ratio = tToNow / double(windowLength); // 0..1
            ratio = std::clamp(ratio, 0.0, 1.0);
            return h - int(ratio * h);
        };

        int yBottom = timeToY(tToNowStart); // ближе к текущему моменту
        int yTop    = timeToY(tToNowEnd);   // выше, дальше в будущее
        if (yTop > yBottom)
            std::swap(yTop, yBottom);

        // Горизонталь: совпадает с клавишей
        QRect keyR = m_keyboard->keyRect(n.pitch);
        if (!keyR.isValid())
            continue;

        int noteX     = keyR.x();
        int noteWidth = keyR.width();

        QRect r(noteX, yTop, noteWidth, yBottom - yTop);

        p.setPen(Qt::NoPen);
        p.setBrush(QColor(80, 180, 255));
        p.drawRect(r);
    }

    // Линия текущего времени у клавиатуры
    p.setPen(QPen(Qt::red, 2));
    p.drawLine(0, h - 1, w, h - 1);
}

void PianoRollWidget::setKeyboard(PianoKeyboardWidget *keyboard)
{
    m_keyboard = keyboard;
    update();
}
