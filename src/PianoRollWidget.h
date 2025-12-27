#ifndef PIANOROLLWIDGET_H
#define PIANOROLLWIDGET_H

#include <QWidget>
#include <QVector>
#include "MidiParser.h"   // для MidiNote

class PianoKeyboardWidget;   // forward

class PianoRollWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PianoRollWidget(QWidget *parent = nullptr);

    void setNotes(const QVector<MidiNote> &notes);
    void setCurrentTime(qint64 ms);

    void setKeyboard(PianoKeyboardWidget *keyboard);

protected:
    void paintEvent(QPaintEvent *event) override;
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

private:
    QVector<MidiNote> m_notes;
    qint64 m_currentTimeMs = 0;
    PianoKeyboardWidget *m_keyboard = nullptr;
};

#endif // PIANOROLLWIDGET_H
