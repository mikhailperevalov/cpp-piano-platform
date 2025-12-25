#ifndef MIDIPLAYER_H
#define MIDIPLAYER_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <memory>

class MidiParser;

class MidiPlayer : public QObject {
    Q_OBJECT

public:
    explicit MidiPlayer(QObject *parent = nullptr);
    ~MidiPlayer();
    
    bool loadFile(const QString &filePath);
    void play();
    void pause();
    void stop();
    void setPosition(qint64 position);
    void setTempo(int bpm);

signals:
    void positionChanged(qint64 position);
    void durationChanged(qint64 duration);
    void playbackStarted();
    void playbackPaused();
    void playbackStopped();
    void fileLoaded(const QString &fileName);
    void error(const QString &message);
    void noteOn(int midiNote, int velocity);
    void noteOff(int midiNote);

private slots:
    void onTimerTick();

private:
    std::unique_ptr<MidiParser> parser;
    QTimer *playbackTimer;
    
    qint64 currentPosition;
    qint64 totalDuration;
    bool isPlaying;
    int currentTempo;

    int eventIndex = 0;
};

#endif // MIDIPLAYER_H
