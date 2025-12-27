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
        const auto &notes = parser->getNotes();
        qDebug() << "Loaded MIDI notes:" << notes.size();
        if (!notes.isEmpty()) {
            qDebug() << "First note pitch/start/duration(ms):"
                     << notes[0].pitch
                     << notes[0].startTime
                     << notes[0].duration;
        }

        totalDuration   = parser->getDuration();
        currentPosition = 0;
        eventIndex      = 0;
        noteIndex       = 0;
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
    noteIndex  = 0;
    emit positionChanged(0);
    emit playbackStopped();
}

void MidiPlayer::setPosition(qint64 position)
{
    if (!parser || !parser->isLoaded())
        return;

    if (position < 0)
        position = 0;
    if (position > totalDuration)
        position = totalDuration;

    currentPosition = position;
    emit positionChanged(currentPosition);

    const auto &notes = parser->getNotes();
    noteIndex = 0;
    while (noteIndex < notes.size() && notes[noteIndex].startTime < currentPosition) {
        ++noteIndex;
    }
}


void MidiPlayer::setTempo(int bpm) {
    currentTempo = bpm;
}

void MidiPlayer::onTimerTick()
{
    if (!isPlaying || !parser || !parser->isLoaded())
        return;

    // Темп: пока будем считать, что currentTempo масштабирует время
    int baseTempo = 120;
    double tempoFactor = static_cast<double>(currentTempo) / static_cast<double>(baseTempo);

    qint64 deltaMs = static_cast<qint64>(50 * tempoFactor);
    currentPosition += deltaMs;

    if (currentPosition >= totalDuration) {
        stop();
        return;
    }

    emit positionChanged(currentPosition);

    const auto &notes = parser->getNotes();
    if (notes.isEmpty())
        return;

    // 1) Включаем ноты, старт которых <= currentPosition
    while (noteIndex < notes.size()) {
        const auto &n = notes[noteIndex];

        if (n.startTime > currentPosition)
            break;

        // Старт ноты
        emit noteOn(static_cast<int>(n.pitch), static_cast<int>(n.velocity));
        ++noteIndex;
    }

    // 2) Гасим ноты, у которых закончилась длительность
    //    Проходим по всем нотам и выключаем те, что уже должны быть Off.
    //    (Оптимизация потом; пока можно O(N) — у тебя записи не гигантские.)
    for (const auto &n : notes) {
        qint64 endTime = n.startTime + n.duration;
        // небольшая «задержка» (5 ms), чтобы не мигали от неточностей
        if (endTime <= currentPosition && endTime > currentPosition - deltaMs) {
            emit noteOff(static_cast<int>(n.pitch));
        }
    }
}

const QVector<MidiNote>& MidiPlayer::getNotes() const
{
    static QVector<MidiNote> empty;
    if (!parser || !parser->isLoaded())
        return empty;
    return parser->getNotes();
}
