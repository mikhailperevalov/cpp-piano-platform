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
        QLinearGradient grad(k.rect.topLeft(), k.rect.bottomLeft());
        if (k.pressed) {
            grad.setColorAt(0.0, QColor("#FFE082")); // светлый сверху
            grad.setColorAt(1.0, QColor("#FFB300")); // насыщенный снизу
        } else {
            grad.setColorAt(0.0, QColor("#FAFAFA"));
            grad.setColorAt(1.0, QColor("#E0E0E0"));
        }

        p.setPen(QColor("#444444"));
        p.setBrush(grad);
        QRect r = k.rect.adjusted(0, 0, -1, -1); // тонкий разделитель справа
        p.drawRect(r);
    }

    // Чёрные клавиши
    for (const Key &k : blackKeys) {
        QLinearGradient grad(k.rect.topLeft(), k.rect.bottomLeft());
        if (k.pressed) {
            grad.setColorAt(0.0, QColor("#424242"));
            grad.setColorAt(1.0, QColor("#00BCD4"));
        } else {
            grad.setColorAt(0.0, QColor("#333333"));
            grad.setColorAt(1.0, QColor("#000000"));
        }

        p.setPen(Qt::NoPen);
        p.setBrush(grad);
        QRect r = k.rect.adjusted(1, 0, -1, -1);
        p.drawRect(r);
    }

    p.setPen(QColor("#303030"));
    p.setBrush(Qt::NoBrush);
    p.drawRect(rect().adjusted(0, 0, -1, -1));

}


void PianoKeyboardWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    layoutKeys();      // переразложить клавиши при изменении размера
}


void PianoKeyboardWidget::pressKey(int midiNote)
{
    bool found = false;
    for (Key &k : whiteKeys) {
        if (k.midiNote == midiNote) {
            k.pressed = true;
            found = true;
        }
    }
    for (Key &k : blackKeys) {
        if (k.midiNote == midiNote) {
            k.pressed = true;
            found = true;
        }
    }
    if (found) {
        update();
    } else {
        qDebug() << "PianoKeyboardWidget: key not found" << midiNote;
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

QRect PianoKeyboardWidget::keyRect(int midiNote) const
{
    for (const Key &k : whiteKeys) {
        if (k.midiNote == midiNote)
            return k.rect;
    }
    for (const Key &k : blackKeys) {
        if (k.midiNote == midiNote)
            return k.rect;
    }
    return QRect();
}
