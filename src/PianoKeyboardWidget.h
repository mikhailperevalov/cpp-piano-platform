#ifndef PIANOKEYBOARDWIDGET_H
#define PIANOKEYBOARDWIDGET_H

#include <QWidget>
#include <QVector>

class PianoKeyboardWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PianoKeyboardWidget(QWidget *parent = nullptr);

    // MIDI ноты: 21 (A0) .. 108 (C8)
    void pressKey(int midiNote);
    void releaseKey(int midiNote);
    QRect keyRect(int midiNote) const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

private:
    struct Key {
        QRect rect;
        bool isBlack;
        bool pressed;
        int midiNote;
    };

    QVector<Key> whiteKeys;
    QVector<Key> blackKeys;

    void layoutKeys();
    bool isBlackKey(int midiNote) const;
};

#endif // PIANOKEYBOARDWIDGET_H
