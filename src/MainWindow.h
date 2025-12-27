#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSlider>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include "MidiPlayer.h"
#include "PianoKeyboardWidget.h"
#include "PianoRollWidget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onOpenMidiFile();
    void onPlay();
    void onPause();
    void onStop();
    void onSliderMoved(int position);
    void onPositionChanged(qint64 position);
    void onDurationChanged(qint64 duration);
    void onTempoChanged(int value);
    void onResyncNotes(qint64 position);

private:
    void setupUI();
    void connectSignals();

    MidiPlayer *midiPlayer;

    PianoKeyboardWidget *pianoWidget;
    PianoRollWidget     *pianoRoll;
    
    // UI элементы
    QPushButton *btnOpenFile;
    QPushButton *btnPlay;
    QPushButton *btnPause;
    QPushButton *btnStop;
    
    QSlider *sliderPosition;
    QSlider *sliderTempo;
    
    QLabel *lblCurrentTime;
    QLabel *lblDuration;
    QLabel *lblFileName;
    QLabel *lblTempo;
    QComboBox *cbInstruments;
};

#endif // MAINWINDOW_H
