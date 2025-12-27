// MidiParser.h
#ifndef MIDIPARSER_H
#define MIDIPARSER_H

#include <QString>
#include <QVector>
#include <cstdint>

struct MidiNote {
    uint8_t pitch;
    uint8_t velocity;
    qint64 startTime;   // ms
    qint64 duration;    // ms
    uint8_t channel;
};

class MidiParser {
public:
    MidiParser();
    ~MidiParser();

    bool parseFile(const QString &filePath);
    bool isLoaded() const { return loaded; }

    qint64 getDuration() const { return durationMs; }
    const QVector<MidiNote>& getNotes() const { return notes; }

private:
    QVector<MidiNote> notes;
    qint64 durationMs = 0;
    bool loaded = false;
};

#endif
