#ifndef MIDIPARSER_H
#define MIDIPARSER_H

#include <QString>
#include <QVector>
#include <cstdint>

struct MidiNote {
    uint8_t pitch;
    uint8_t velocity;
    qint64 startTime;
    qint64 duration;
    uint8_t channel;
};

struct MidiEvent {
    qint64 time;
    uint8_t type;
    uint8_t channel;
    uint8_t data1;
    uint8_t data2;
};

class MidiParser {
public:
    MidiParser();
    ~MidiParser();
    
    bool parseFile(const QString &filePath);
    bool isLoaded() const { return loaded; }
    
    qint64 getDuration() const { return duration; }
    const QVector<MidiNote>& getNotes() const { return notes; }
    const QVector<MidiEvent>& getEvents() const { return events; }
    
    int getTicksPerQuarter() const { return ticksPerQuarter; }
    int getFormat() const { return midiFormat; }

private:
    bool parseHeader(const QByteArray &data, int &pos);
    bool parseTrack(const QByteArray &data, int &pos);
    
    uint32_t readBigEndian32(const unsigned char *data);
    uint16_t readBigEndian16(const unsigned char *data);
    qint64 readVariableLength(const unsigned char *&data);
    
    QVector<MidiNote> notes;
    QVector<MidiEvent> events;
    
    qint64 duration;
    int ticksPerQuarter;
    int midiFormat;
    int numTracks;
    bool loaded;
};

#endif // MIDIPARSER_H
