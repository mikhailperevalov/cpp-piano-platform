#include "MidiPlayer.h"
#include "MidiParser.h"
#include <QTimer>
#include <QDebug>
#include <QFileInfo>

MidiPlayer::MidiPlayer(QObject *parent)
    : QObject(parent),
      parser(std::make_unique<MidiParser>()),
      currentPosition(0),
      totalDuration(0),
      isPlaying(false),
      currentTempo(120) {
    
    playbackTimer = new QTimer(this);
    connect(playbackTimer, &QTimer::timeout, this, &MidiPlayer::onTimerTick);
}

MidiPlayer::~MidiPlayer() {
}

bool MidiPlayer::loadFile(const QString &filePath) {
    if (parser->parseFile(filePath)) {
        totalDuration = parser->getDuration();
        currentPosition = 0;
        eventIndex = 0;
        emit durationChanged(totalDuration);
        emit fileLoaded(QFileInfo(filePath).fileName());
        return true;
    }
    
    emit error("Ошибка при загрузке MIDI файла: " + filePath);
    return false;
}

void MidiPlayer::play() {
    if (!parser->isLoaded()) {
        emit error("Файл не загружен");
        return;
    }
    
    isPlaying = true;
    playbackTimer->start(50); // Обновляем каждые 50ms
    emit playbackStarted();
}

void MidiPlayer::pause() {
    isPlaying = false;
    playbackTimer->stop();
    emit playbackPaused();
}

void MidiPlayer::stop() {
    isPlaying = false;
    playbackTimer->stop();
    currentPosition = 0;
    eventIndex = 0;
    emit positionChanged(0);
    emit playbackStopped();
}

void MidiPlayer::setPosition(qint64 position) {
    if (position >= 0 && position <= totalDuration) {
        currentPosition = position;
        emit positionChanged(currentPosition);
    }
}

void MidiPlayer::setTempo(int bpm) {
    currentTempo = bpm;
}

void MidiPlayer::onTimerTick()
{
    if (!isPlaying || !parser || !parser->isLoaded())
        return;

    int baseTempo = 120;
    double tempoFactor = static_cast<double>(currentTempo) / static_cast<double>(baseTempo);

    qint64 deltaMs = static_cast<qint64>(50 * tempoFactor);
    currentPosition += deltaMs;

    if (currentPosition >= totalDuration) {
        stop();
        return;
    }

    emit positionChanged(currentPosition);

    const auto &events = parser->getEvents();
    if (eventIndex < 0 || eventIndex >= events.size())
        return;

    while (eventIndex < events.size()) {
        const auto &ev = events[eventIndex];

        // ev.time уже в миллисекундах
        if (ev.time > currentPosition)
            break;

        if (ev.type == 0x90 && ev.data2 > 0) {            // Note On
            emit noteOn(static_cast<int>(ev.data1), static_cast<int>(ev.data2));
        } else if (ev.type == 0x80 || (ev.type == 0x90 && ev.data2 == 0)) { // Note Off
            emit noteOff(static_cast<int>(ev.data1));
        }

        ++eventIndex;
    }
}
