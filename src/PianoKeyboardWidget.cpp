#include "PianoKeyboardWidget.h"
#include <QPainter>
#include <QResizeEvent>

PianoKeyboardWidget::PianoKeyboardWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(120);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layoutKeys();
}

QSize PianoKeyboardWidget::minimumSizeHint() const
{
    return QSize(400, 120);
}

QSize PianoKeyboardWidget::sizeHint() const
{
    return QSize(800, 140);
}

bool PianoKeyboardWidget::isBlackKey(int midiNote) const
{
    // Внутри октавы: C=0, C#=1, ..., B=11
    int n = (midiNote % 12);
    switch (n) {
    case 1:  // C#
    case 3:  // D#
    case 6:  // F#
    case 8:  // G#
    case 10: // A#
        return true;
    default:
        return false;
    }
}

void PianoKeyboardWidget::layoutKeys()
{
    whiteKeys.clear();
    blackKeys.clear();

    int firstNote = 21;  // A0
    int lastNote  = 108; // C8

    int whiteCount = 0;
    for (int n = firstNote; n <= lastNote; ++n) {
        if (!isBlackKey(n))
            ++whiteCount;
    }

    // Размеры по ширине зависят от текущей ширины виджета
    int w = width() > 0 ? width() : 800;
    int h = height() > 0 ? height() : 120;

    int whiteWidth = w / whiteCount;
    int whiteHeight = h;

    int currentX = 0;

    // Сначала создаём белые
    for (int n = firstNote; n <= lastNote; ++n) {
        if (!isBlackKey(n)) {
            Key k;
            k.rect = QRect(currentX, 0, whiteWidth, whiteHeight);
            k.isBlack = false;
            k.pressed = false;
            k.midiNote = n;
            whiteKeys.push_back(k);

            currentX += whiteWidth;
        }
    }

    // Затем создаём чёрные (короче и уже, поверх белых)
    for (int i = 0; i < whiteKeys.size(); ++i) {
        const Key &wk = whiteKeys[i];
        int midi = wk.midiNote;

        // Внутри октавы смотрим следующую белую и вставляем чёрную между ними
        int semitone = (midi % 12);
        if (semitone == 0 || semitone == 2 || semitone == 5 || semitone == 7 || semitone == 9) {
            int blackNote = midi + 1;
            if (blackNote > lastNote || !isBlackKey(blackNote))
                continue;

            Key bk;
            int bw = whiteWidth * 0.6;
            int bh = whiteHeight * 0.6;
            int bx = wk.rect.x() + whiteWidth - bw / 2;
            int by = 0;

            bk.rect = QRect(bx, by, bw, bh);
            bk.isBlack = true;
            bk.pressed = false;
            bk.midiNote = blackNote;

            blackKeys.push_back(bk);
        }
    }
}

void PianoKeyboardWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);

    // Белые клавиши
    for (const Key &k : whiteKeys) {
        p.setPen(Qt::black);
        p.setBrush(k.pressed ? QColor("#ffcc66") : Qt::white);
        p.drawRect(k.rect);
    }

    // Чёрные клавиши
    for (const Key &k : blackKeys) {
        p.setPen(Qt::black);
        p.setBrush(k.pressed ? QColor("#ff9933") : Qt::black);
        p.drawRect(k.rect);
    }
}


void PianoKeyboardWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    layoutKeys();      // переразложить клавиши при изменении размера
}


void PianoKeyboardWidget::pressKey(int midiNote)
{
    for (Key &k : whiteKeys) {
        if (k.midiNote == midiNote) {
            k.pressed = true;
            update();
            return;
        }
    }
    for (Key &k : blackKeys) {
        if (k.midiNote == midiNote) {
            k.pressed = true;
            update();
            return;
        }
    }
}

void PianoKeyboardWidget::releaseKey(int midiNote)
{
    for (Key &k : whiteKeys) {
        if (k.midiNote == midiNote) {
            k.pressed = false;
            update();
            return;
        }
    }
    for (Key &k : blackKeys) {
        if (k.midiNote == midiNote) {
            k.pressed = false;
            update();
            return;
        }
    }
}
