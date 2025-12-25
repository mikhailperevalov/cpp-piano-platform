#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QSlider>
#include <QLabel>
#include <QSpinBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    
    midiPlayer = new MidiPlayer(this);
    
    setupUI();
    connectSignals();
    
    setWindowTitle("Piano Platform v1.0");
    setGeometry(100, 100, 1200, 800);
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUI() {
    // Создаем центральный виджет
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    // === ЗАГОЛОВОК ===
    QLabel *lblTitle = new QLabel("🎹 Piano Platform", this);
    lblTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50;");
    mainLayout->addWidget(lblTitle);
    
    // === ИНФОРМАЦИЯ О ФАЙЛЕ ===
    QHBoxLayout *fileLayout = new QHBoxLayout();
    QLabel *lblFileLabel = new QLabel("Текущий файл:", this);
    lblFileName = new QLabel("Не загружен", this);
    lblFileName->setStyleSheet("color: #7f8c8d;");
    fileLayout->addWidget(lblFileLabel);
    fileLayout->addWidget(lblFileName);
    fileLayout->addStretch();
    mainLayout->addLayout(fileLayout);
    
    // === КНОПКИ УПРАВЛЕНИЯ ===
    QHBoxLayout *controlsLayout = new QHBoxLayout();
    
    btnOpenFile = new QPushButton("📂 Открыть MIDI", this);
    btnPlay = new QPushButton("▶ Воспроизведение", this);
    btnPause = new QPushButton("⏸ Пауза", this);
    btnStop = new QPushButton("⏹ Стоп", this);
    
    btnPlay->setEnabled(false);
    btnPause->setEnabled(false);
    btnStop->setEnabled(false);
    
    controlsLayout->addWidget(btnOpenFile);
    controlsLayout->addWidget(btnPlay);
    controlsLayout->addWidget(btnPause);
    controlsLayout->addWidget(btnStop);
    controlsLayout->addStretch();
    
    mainLayout->addLayout(controlsLayout);
    
    // === ПРОГРЕСС БАР ===
    QHBoxLayout *progressLayout = new QHBoxLayout();
    
    lblCurrentTime = new QLabel("00:00", this);
    lblCurrentTime->setMaximumWidth(50);
    
    sliderPosition = new QSlider(Qt::Horizontal, this);
    sliderPosition->setRange(0, 0);
    sliderPosition->setEnabled(false);
    
    lblDuration = new QLabel("00:00", this);
    lblDuration->setMaximumWidth(50);
    
    progressLayout->addWidget(lblCurrentTime);
    progressLayout->addWidget(sliderPosition);
    progressLayout->addWidget(lblDuration);
    
    mainLayout->addLayout(progressLayout);
    
    // === ТЕМПО ===
    QHBoxLayout *tempoLayout = new QHBoxLayout();
    
    QLabel *lblTempoLabel = new QLabel("Темпо (BPM):", this);
    sliderTempo = new QSlider(Qt::Horizontal, this);
    sliderTempo->setRange(50, 200);
    sliderTempo->setValue(120);
    sliderTempo->setMaximumWidth(200);
    lblTempo = new QLabel("120", this);
    lblTempo->setMaximumWidth(50);
    
    tempoLayout->addWidget(lblTempoLabel);
    tempoLayout->addWidget(sliderTempo);
    tempoLayout->addWidget(lblTempo);
    tempoLayout->addStretch();
    
    mainLayout->addLayout(tempoLayout);
    
    // === ИНСТРУМЕНТЫ ===
    QHBoxLayout *instrumentLayout = new QHBoxLayout();
    
    QLabel *lblInstrumentLabel = new QLabel("Инструмент:", this);
    cbInstruments = new QComboBox(this);
    cbInstruments->addItems({
        "Grand Piano",
        "Bright Piano",
        "Electric Piano",
        "Harpsichord",
        "Celesta"
    });
    cbInstruments->setMaximumWidth(200);
    
    instrumentLayout->addWidget(lblInstrumentLabel);
    instrumentLayout->addWidget(cbInstruments);
    instrumentLayout->addStretch();
    
    mainLayout->addLayout(instrumentLayout);
    
    // === ОБЛАСТЬ ДЛЯ ВИЗУАЛИЗАЦИИ КЛАВИШ ===
    pianoWidget = new PianoKeyboardWidget(this);
    mainLayout->addWidget(pianoWidget);

    
    // === СТАТУС БАР ===
    QLabel *statusLabel = new QLabel("Готово", this);
    statusLabel->setStyleSheet("color: #27ae60; padding: 5px;");
    mainLayout->addWidget(statusLabel);
    
    mainLayout->addStretch();
}

void MainWindow::connectSignals() {
    connect(btnOpenFile, &QPushButton::clicked, this, &MainWindow::onOpenMidiFile);
    connect(btnPlay, &QPushButton::clicked, this, &MainWindow::onPlay);
    connect(btnPause, &QPushButton::clicked, this, &MainWindow::onPause);
    connect(btnStop, &QPushButton::clicked, this, &MainWindow::onStop);
    
    connect(sliderTempo, &QSlider::valueChanged, this, &MainWindow::onTempoChanged);
    connect(sliderPosition, &QSlider::sliderMoved, this, &MainWindow::onSliderMoved);
    
    // Сигналы от плеера
    connect(midiPlayer, &MidiPlayer::positionChanged, this, &MainWindow::onPositionChanged);
    connect(midiPlayer, &MidiPlayer::durationChanged, this, &MainWindow::onDurationChanged);

    connect(midiPlayer, &MidiPlayer::noteOn,
            pianoWidget, &PianoKeyboardWidget::pressKey);
    connect(midiPlayer, &MidiPlayer::noteOff,
            pianoWidget, &PianoKeyboardWidget::releaseKey);
}

void MainWindow::onOpenMidiFile() {
    QString fileName = QFileDialog::getOpenFileName(this,
        "Открыть MIDI файл", "",
        "MIDI Files (*.mid *.midi);;All Files (*)");
    
    if (!fileName.isEmpty()) {
        if (midiPlayer->loadFile(fileName)) {
            lblFileName->setText(QFileInfo(fileName).fileName());
            btnPlay->setEnabled(true);
            sliderPosition->setEnabled(true);
        }
    }
}

void MainWindow::onPlay() {

    midiPlayer->play();
    btnPlay->setEnabled(false);
    btnPause->setEnabled(true);

    // ТЕСТ: подсветить среднее до (MIDI 60) на 1 секунду
    if (pianoWidget) {
        pianoWidget->pressKey(60);
        QTimer::singleShot(1000, this, [this]() {
            pianoWidget->releaseKey(60);
        });
    }
}

void MainWindow::onPause() {
    midiPlayer->pause();
    btnPlay->setEnabled(true);
    btnPause->setEnabled(false);
}

void MainWindow::onStop() {
    midiPlayer->stop();
    btnPlay->setEnabled(true);
    btnPause->setEnabled(false);
    btnStop->setEnabled(false);
}

void MainWindow::onSliderMoved(int position) {
    midiPlayer->setPosition(position);
}

void MainWindow::onPositionChanged(qint64 position) {
    int seconds = position / 1000;
    int minutes = seconds / 60;
    seconds = seconds % 60;
    
    sliderPosition->blockSignals(true);
    sliderPosition->setValue(position);
    sliderPosition->blockSignals(false);
    
    lblCurrentTime->setText(QString("%1:%2")
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0')));
}

void MainWindow::onDurationChanged(qint64 duration) {
    int seconds = duration / 1000;
    int minutes = seconds / 60;
    seconds = seconds % 60;
    
    sliderPosition->setRange(0, duration);
    lblDuration->setText(QString("%1:%2")
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0')));
}

void MainWindow::onTempoChanged(int value) {
    lblTempo->setText(QString::number(value));
    midiPlayer->setTempo(value);
}
