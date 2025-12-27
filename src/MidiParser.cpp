#include "MidiParser.h"
#include <QDebug>

// midifile
#include "MidiFile.h"

using namespace smf;

MidiParser::MidiParser() {}
MidiParser::~MidiParser() {}

bool MidiParser::parseFile(const QString &filePath) {
    notes.clear();
    durationMs = 0;
    loaded = false;

    MidiFile mf;
    if (!mf.read(filePath.toStdString())) {
        qWarning() << "midifile: cannot read" << filePath;
        return false;
    }
    if (!mf.status()) {
        qWarning() << "midifile: read status failed for" << filePath;
        return false;
    }

    // Важно: анализ времени с учетом tempo meta-messages:
    mf.doTimeAnalysis();     // выставляет .seconds у событий по tempo map [page:0]
    mf.linkNotePairs();      // связывает note-on/off, дает getDurationInSeconds() [page:0]

    // Чтобы проходить в одном списке:
    mf.joinTracks();

    // Длительность файла:
    durationMs = static_cast<qint64>(mf.getFileDurationInSeconds() * 1000.0);

    // Забираем ноты:
    for (int i = 0; i < mf[0].getEventCount(); ++i) {
        auto &ev = mf[0][i];
        if (!ev.isNoteOn())
            continue;

        double startSec = ev.seconds;
        double durSec   = ev.getDurationInSeconds(); // после linkNotePairs [page:0]
        if (durSec <= 0.0)
            continue;

        MidiNote n;
        n.pitch     = static_cast<uint8_t>(ev.getKeyNumber());  // pitch
        n.velocity  = static_cast<uint8_t>(ev.getVelocity());   // vel
        n.channel   = static_cast<uint8_t>(ev.getChannelNibble());
        n.startTime = static_cast<qint64>(startSec * 1000.0);
        n.duration  = static_cast<qint64>(durSec   * 1000.0);

        notes.push_back(n);
    }

    // Если durationMs почему-то 0, можно взять максимум из нот:
    if (durationMs <= 0) {
        qint64 maxEnd = 0;
        for (const auto &n : notes)
            maxEnd = qMax(maxEnd, n.startTime + n.duration);
        durationMs = maxEnd;
    }

    qDebug() << "midifile notes:" << notes.size()
             << "duration(ms):" << durationMs;

    loaded = !notes.isEmpty();
    return loaded;
}
