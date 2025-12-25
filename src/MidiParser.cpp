#include "MidiParser.h"
#include <QFile>
#include <QDebug>
#include <cstring>

MidiParser::MidiParser()
    : duration(0),
      ticksPerQuarter(480),
      midiFormat(0),
      numTracks(0),
      loaded(false) {
}

MidiParser::~MidiParser() {
}

bool MidiParser::parseFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Не удается открыть файл:" << filePath;
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    if (data.size() < 14) {
        qWarning() << "Файл MIDI слишком короткий";
        return false;
    }
    
    int pos = 0;
    
    // Парсим заголовок
    if (!parseHeader(data, pos)) {
        return false;
    }
    
    notes.clear();
    events.clear();
    duration = 0;
    
    // Парсим треки
    for (int i = 0; i < numTracks && pos < data.size(); ++i) {
        if (!parseTrack(data, pos)) {
            qWarning() << "Ошибка при парсинге трека" << i;
        }
    }
    
    // Вычисляем длительность в миллисекундах
    // Примерная формула: (duration в тиках * 60000) / (ticksPerQuarter * tempo)
    // Используем стандартный темпо 120 BPM
    if (ticksPerQuarter > 0) {
        duration = (duration * 60000) / (ticksPerQuarter * 120);
    }
    
    loaded = true;

    // Переводим время событий из тиков в миллисекунды (принимаем базовый темп 120 BPM)
    if (ticksPerQuarter > 0) {
        for (auto &ev : events) {
            ev.time = (ev.time * 60000) / (ticksPerQuarter * 120);
        }
        // duration уже пересчитали выше
    }

    // После всех треков:
    for (const auto &ev : events) {
        if (ev.time > duration)
            duration = ev.time;
    }

    // ticksPerQuarter уже считан из заголовка
    if (ticksPerQuarter > 0) {
        // Предполагаем базовый темп 120 BPM
        constexpr int baseTempo = 120;
        const qint64 factor = (60000 / baseTempo); // 500 мс на четверть при 120 BPM

        for (auto &ev : events) {
            // ev.time был в тиках
            ev.time = (ev.time * factor) / ticksPerQuarter;
        }

        // duration тоже в тиках — переведём аналогично
        duration = (duration * factor) / ticksPerQuarter;
    }

    return true;
}

bool MidiParser::parseHeader(const QByteArray &data, int &pos) {
    // Проверяем сигнатуру "MThd"
    if (data.mid(pos, 4) != "MThd") {
        qWarning() << "Неверная сигнатура MIDI файла";
        return false;
    }
    pos += 4;
    
    // Читаем длину заголовка (обычно 6)
    uint32_t headerLen = readBigEndian32((unsigned char*)data.data() + pos);
    pos += 4;
    
    // Читаем формат
    midiFormat = readBigEndian16((unsigned char*)data.data() + pos);
    pos += 2;
    
    // Читаем количество треков
    numTracks = readBigEndian16((unsigned char*)data.data() + pos);
    pos += 2;
    
    // Читаем разделение (division)
    int division = readBigEndian16((unsigned char*)data.data() + pos);
    pos += 2;
    
    if (division & 0x8000) {
        // SMPTE режим
        ticksPerQuarter = 960; // Стандартное значение
    } else {
        ticksPerQuarter = division & 0x7FFF;
    }
    
    qDebug() << "MIDI Header: Format" << midiFormat 
             << "Tracks" << numTracks 
             << "Ticks/Quarter" << ticksPerQuarter;
    
    return true;
}

bool MidiParser::parseTrack(const QByteArray &data, int &pos) {
    if (data.mid(pos, 4) != "MTrk") {
        qWarning() << "Не найдена сигнатура MTrk";
        return false;
    }
    pos += 4;
    
    uint32_t trackLen = readBigEndian32((unsigned char*)data.data() + pos);
    pos += 4;
    
    int trackEnd = pos + trackLen;
    qint64 currentTime = 0;
    uint8_t lastStatus = 0;
    
    while (pos < trackEnd && pos < data.size()) {
        // Читаем time delta
        const unsigned char *ptr = (unsigned char*)data.data() + pos;
        qint64 delta = readVariableLength(ptr);
        pos += ptr - (unsigned char*)data.data() - pos;
        
        currentTime += delta;
        
        if (pos >= trackEnd) break;
        
        uint8_t status = data[pos++];
        
        // Обработка meta событий
        if (status == 0xFF) {
            uint8_t metaType = data[pos++];
            ptr = (unsigned char*)data.data() + pos;
            qint64 length = readVariableLength(ptr);
            pos += ptr - (unsigned char*)data.data() - pos;
            
            switch (metaType) {
                case 0x51: { // Set Tempo
                    if (length == 3 && pos + 3 <= trackEnd) {
                        uint32_t microSecondsPerQuarter = 
                            ((unsigned char)data[pos] << 16) |
                            ((unsigned char)data[pos+1] << 8) |
                            (unsigned char)data[pos+2];
                        qDebug() << "Tempo:" << (60000000 / microSecondsPerQuarter) << "BPM";
                    }
                    break;
                }
                case 0x2F: { // End of Track
                    pos = trackEnd;
                    break;
                }
            }
            
            pos += length;
        }
        // MIDI события
        else if (status >= 0x80) {
            lastStatus = status;
            uint8_t channel = status & 0x0F;
            uint8_t msgType = status & 0xF0;
            
            switch (msgType) {
                case 0x90: { // Note On
                    if (pos + 1 < trackEnd) {
                        uint8_t pitch = data[pos++];
                        uint8_t velocity = data[pos++];
                        
                        if (velocity > 0) {
                            // Пока просто сохраняем время начала
                            events.push_back({currentTime, msgType, channel, pitch, velocity});
                            if (currentTime > duration) {
                                duration = currentTime;
                            }
                        }
                    }
                    break;
                }
                case 0x80: { // Note Off
                    if (pos + 1 < trackEnd) {
                        uint8_t pitch = data[pos++];
                        uint8_t velocity = data[pos++];
                        events.push_back({currentTime, msgType, channel, pitch, velocity});
                    }
                    break;
                }
                case 0xB0: { // Control Change
                    if (pos + 1 < trackEnd) {
                        uint8_t controller = data[pos++];
                        uint8_t value = data[pos++];
                    }
                    break;
                }
                case 0xC0: { // Program Change
                    if (pos < trackEnd) {
                        uint8_t program = data[pos++];
                    }
                    break;
                }
                case 0xE0: { // Pitch Bend
                    if (pos + 1 < trackEnd) {
                        uint8_t lsb = data[pos++];
                        uint8_t msb = data[pos++];
                    }
                    break;
                }
                default:
                    break;
            }
        }
        // Running status
        else if (lastStatus >= 0x80) {
            pos--; // Возвращаем байт
            
            uint8_t channel = lastStatus & 0x0F;
            uint8_t msgType = lastStatus & 0xF0;
            
            if (msgType == 0x90 && pos + 1 < trackEnd) {
                uint8_t pitch = data[pos++];
                uint8_t velocity = data[pos++];
                if (velocity > 0) {
                    events.push_back({currentTime, msgType, channel, pitch, velocity});
                }
            }
        }
    }
    
    return true;
}

uint32_t MidiParser::readBigEndian32(const unsigned char *data) {
    return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
}

uint16_t MidiParser::readBigEndian16(const unsigned char *data) {
    return (data[0] << 8) | data[1];
}

qint64 MidiParser::readVariableLength(const unsigned char *&data) {
    qint64 value = 0;
    unsigned char c;
    
    do {
        c = *data++;
        value = (value << 7) | (c & 0x7F);
    } while (c & 0x80);
    
    return value;
}
