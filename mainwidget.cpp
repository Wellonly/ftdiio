
#include <QFormLayout>
#include <QScrollBar>
#include <QChart>
#include <QChartView>
#include <QPushButton>
#include <QFrame>
#include <QAbstractScrollArea>
#include <QScrollArea>
#include <QSpinBox>
#include <QLabel>
#include <QDebug>
#include <QBarSet>
#include <QBarSeries>
#include <QLegend>
#include <QFormLayout>
#include <QLineSeries>
#include <QStackedBarSeries>
#include <QCategoryAxis>
#include <QPrinter>
#include <QPrintPreviewDialog>
#include <QProgressDialog>
#include <QApplication>
#include <QPrintDialog>
#include <QInputDialog>
#include <QLegendMarker>
#include <QBarCategoryAxis>
#include <QDateTime>
#include <QLatin1Literal>
#include <QMessageBox>

#include "c_cpp_macros.h"
#include "jconfig.h"
#include "zvvlib.h"
#include "datetime.h"
#include "ftdilib.h"

#include "mainwidget.h"

#ifndef APP_VERSION
#define APP_VERSION DATE_OF_BUILD_AS_YYYYMMDD()
#endif

#ifndef STEP_TIME_MAX_SEC
#define STEP_TIME_MAX_SEC 32768
#endif

#ifndef STEP_COUNT_MAX
#define STEP_COUNT_MAX 128
#endif

#ifndef REPLAY_COUNT_MAX
#define REPLAY_COUNT_MAX 128
#endif

#ifndef CHANNEL_COUNT
#define CHANNEL_COUNT 4
#endif

#ifndef INIT_STATE
#define INIT_STATE {{1,3677}} /*{0, 25, 50, 75, 100}*/
#endif

#ifndef COLOR_BAR_EMPTY
#define COLOR_BAR_EMPTY "#d5d5d5"
#endif

#ifndef COLOR_BAR_ENGAGED
#define COLOR_BAR_ENGAGED "#79cc99"
#endif

#ifndef FTDI_SERIAL_NUMBER
#define FTDI_SERIAL_NUMBER "DN2WSEEK"
#endif

#ifndef FTDI_DDIR_MASK
#define FTDI_DDIR_MASK 0xF0
#endif

#ifndef FTDI_ASYNC_SCL_MASK
#define FTDI_ASYNC_SCL_MASK 0x01
#endif

#ifndef HW_CHANNEL_REMASK
#define HW_CHANNEL_REMASK {2,1,8,4} /* remask per channel. For good hardware (when {1,2,4,8}) this doesn't need, so comment it out!!! */
#endif

#ifdef HW_CHANNEL_REMASK
static int hw_remask[] = HW_CHANNEL_REMASK;
#endif

#define isHWPowerGood() (!(getAsyncFTDIData() & FTDI_ASYNC_SCL_MASK))

QT_CHARTS_USE_NAMESPACE

static const int ChannelPercent = 100/CHANNEL_COUNT;

static QVector<QPair<int,int>> dataTable = INIT_STATE; //<ctrlBits, time_seconds>
//static QVector<int> dataTable = INIT_STATE; // <time_seconds>

static QPrinter* Printer = NULL;

static int currentGUIIndex = -1;
static int currentTimerStep = -1;
//static int currentStepTime = 0;
static int replayCount = 0;
static int loopCounter = 0;
static bool isReplayEndless = false;

static QScrollArea* scrollArea = 0;
static ScrolledChartView* chartView = 0;

//ftdi...
static FT_HANDLE ftHandle = 0;
static FT_STATUS ftStatus = 0;
static HWStatus  hwStatus = HWStatus_undefined;

//QTimer
static QDateTime stepStartRunTime;
static QDateTime stepStartPauseTime;
static qint64 stepPauseTime_mS = 0;

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent)
{
    setWindowIcon(zvvlib::getIcon("Well"));
    setWindowTitle(QString("Таймер управления нагрузкой, v.%1").arg(APP_VERSION));
    // Create buttons for ui
    m_buttonLayout = new QGridLayout();
    QPushButton *newJobButton = new QPushButton("Новый режим");
    connect(newJobButton, SIGNAL(clicked()), this, SLOT(newJob()));
    m_buttonLayout->addWidget(newJobButton, 0, 0);

    QPushButton *openJobButton = new QPushButton("Открыть режим...");
    connect(openJobButton, SIGNAL(clicked()), this, SLOT(openJob()));
    m_buttonLayout->addWidget(openJobButton, 2, 0);
    QPushButton *saveJobButton = new QPushButton("Сохранить режим...");
    connect(saveJobButton, SIGNAL(clicked()), this, SLOT(saveJob()));
    m_buttonLayout->addWidget(saveJobButton, 3, 0);

    QPushButton *editTitleButton = new QPushButton("Наименование режима...");
    connect(editTitleButton, SIGNAL(clicked()), this, SLOT(editTitle()));
    m_buttonLayout->addWidget(editTitleButton, 4, 0);

    m_runJobButton = new QPushButton(tr("Запуск режима").rightJustified(32));
    m_runJobButton->setCheckable(true);
    m_runJobButton->setIcon(zvvlib::getIcon("play"));
    connect(m_runJobButton, SIGNAL(clicked(bool)), this, SLOT(runJob(bool)));
    m_buttonLayout->addWidget(m_runJobButton, 5, 0);

    QPushButton *stopJobButton = new QPushButton(tr("Отставить работу режима"));
    stopJobButton->setIcon(zvvlib::getIcon("stop"));
    connect(stopJobButton, SIGNAL(clicked()), this, SLOT(stopJob()));
    m_buttonLayout->addWidget(stopJobButton, 6, 0);

    m_pauseButton = new QPushButton(tr("Бесконечный такт").rightJustified(29));
    m_pauseButton->setCheckable(true);
    m_pauseButton->setIcon(zvvlib::getIcon("pause"));
    connect(m_pauseButton, SIGNAL(toggled(bool)), this, SLOT(pauseFromUI(bool)));
    m_buttonLayout->addWidget(m_pauseButton, 7, 0);

    m_replayButton = new QPushButton(tr("Бесконечный цикл").rightJustified(29));
    m_replayButton->setCheckable(true);
    m_replayButton->setIcon(zvvlib::getIcon("refresh"));
    connect(m_replayButton, SIGNAL(clicked(bool)), this, SLOT(replayEndless(bool)));
    m_buttonLayout->addWidget(m_replayButton, 8, 0);

    m_stepCount = new QSpinBox();
    m_stepCount->setValue(dataTable.count());
    m_stepCount->setMinimum(1);
    m_stepCount->setMaximum(STEP_COUNT_MAX);
    m_stepCount->setToolTip(QString("Диапазон: 1..%1").arg(STEP_COUNT_MAX));
    connect(m_stepCount, SIGNAL(valueChanged(int)), this, SLOT(stepCountChanged(int)));

    m_replayCount = new QSpinBox();
    m_replayCount->setValue(1);
    m_replayCount->setMinimum(1);
    m_replayCount->setMaximum(REPLAY_COUNT_MAX);
    m_replayCount->setToolTip(QString("Диапазон: 1..%1").arg(REPLAY_COUNT_MAX));
    connect(m_replayCount, SIGNAL(valueChanged(int)), this, SLOT(replayCountChanged()));

    m_replayLabel = new QLabel("Количество циклов");

    QFormLayout* flayoutMode = new QFormLayout();
    flayoutMode->addRow("Количество тактов", m_stepCount);
    flayoutMode->addRow(m_replayLabel, m_replayCount);

    m_modeSettings = new QGroupBox("Параметры режима");
    m_modeSettings->setLayout(flayoutMode);
    m_buttonLayout->addWidget(m_modeSettings);

/////////////////////////////////////////////////////////////////////////////////
    m_hourSetter   = new QSlider(Qt::Orientation::Horizontal);
    m_hourSetter->setRange(0,24);
    connect(m_hourSetter, SIGNAL(valueChanged(int)), this, SLOT(stepTimeChanged()));
    m_minuteSetter = new QSlider(Qt::Orientation::Horizontal);
    m_minuteSetter->setRange(0,59);
    connect(m_minuteSetter, SIGNAL(valueChanged(int)), this, SLOT(stepTimeChanged()));
    m_secondSetter = new QSlider(Qt::Orientation::Horizontal);
    m_secondSetter->setRange(0,59);
    connect(m_secondSetter, SIGNAL(valueChanged(int)), this, SLOT(stepTimeChanged()));
    m_hourLabel   = new QLabel("Часы");
    m_minuteLabel = new QLabel("Минуты");
    m_secondLabel = new QLabel("Секунды");

    QFormLayout* flayoutStep = new QFormLayout();
    flayoutStep->addRow(m_hourLabel,   m_hourSetter);
    flayoutStep->addRow(m_minuteLabel, m_minuteSetter);
    flayoutStep->addRow(m_secondLabel, m_secondSetter);
    flayoutStep->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    m_stepSettings = new QGroupBox("Параметры такта");
    m_stepSettings->setLayout(flayoutStep);
    m_buttonLayout->addWidget(m_stepSettings);

    QPushButton *printPreviewButton = new QPushButton("Печать...");
    connect(printPreviewButton, SIGNAL(clicked()), this, SLOT(toPrintPreview()));
    m_buttonLayout->addWidget(printPreviewButton);


    QPushButton *aboutButton = new QPushButton("О программе");
    connect(aboutButton, SIGNAL(clicked()), this, SLOT(about()));
    m_buttonLayout->addWidget(aboutButton);

//    QPushButton *zoomInButton = new QPushButton("Zoom in");
//    connect(zoomInButton, SIGNAL(clicked()), this, SLOT(zoomIn()));
//    m_buttonLayout->addWidget(zoomInButton);
//    QPushButton *zoomOutButton = new QPushButton("Zoom out");
//    connect(zoomOutButton, SIGNAL(clicked()), this, SLOT(zoomOut()));
//    m_buttonLayout->addWidget(zoomOutButton);

    // Create chart view with the chart
    scrollArea = new QScrollArea();
    scrollArea->setBackgroundRole(QPalette::Dark);

    m_chart = createBarChart(1); //new QChart(); //createAreaChart();
    chartView = m_chartView = new ScrolledChartView(m_chart, this, this);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    scrollArea->setWidget(m_chartView);

    // Create layout for grid and detached legend
    m_mainLayout = new QGridLayout();
    m_mainLayout->addLayout(m_buttonLayout, 0, 0);
    m_mainLayout->addWidget(scrollArea, 0, 1, 2, 1);

    m_log = new QTextEdit();
    m_log->setReadOnly(true);
    m_mainLayout->addWidget(m_log, 2, 1, 1, 1);

    m_mainLayout->setColumnStretch(0,0);
    m_mainLayout->setColumnStretch(1,1);
    setLayout(m_mainLayout);

    m_chartView->adjustSize();

    QList<QBarSet *> sets = m_series->barSets();
    for (int i = 0; i < sets.count(); i++) {
        QBarSet *set = sets.at(i);
        connect(set, &QBarSet::clicked, [this, i](int ind){ barClicked(ind, i);});
//        connect(set, &QBarSet::hovered, [this, i](bool status, int ind){ hover(status, ind);});
    }
}

MainWidget::~MainWidget()
{
    updateFTDI(0);
    if (Printer) {
        delete Printer;
    }
    if (ftHandle) {
        FT_Close(ftHandle);
    }
}

QChart *MainWidget::createBarChart(int valueCount)
{
    Q_UNUSED(valueCount);
    QChart *chart = new QChart();
    chart->setTitle("Циклограмма");
    m_series = new QStackedBarSeries(chart);
    m_series->setBarWidth(1); //no space between bars!
    const int dataTableSize = dataTable.count();
    for (int barnum = 0; barnum < CHANNEL_COUNT; barnum++) { //2bars(empty,engaged) per channel...
        QBarSet *setem = new QBarSet(QString("em%1").arg(barnum));
        setem->setColor(QColor(COLOR_BAR_EMPTY));   //Qt::GlobalColor::lightGray
        QBarSet *seten = new QBarSet(QString("en%1").arg(barnum));
        seten->setColor(QColor(COLOR_BAR_ENGAGED)); //lightGreen; darkCyan
        for (int v(0); v < dataTableSize; v++) {
            QPair<int,int> value(dataTable.at(v)); //int value;
            if (value.first & (1<<barnum)) {
                *setem << 0;
                *seten << ChannelPercent;
            }
            else {
                *setem << ChannelPercent;
                *seten << 0;
            }
        }
        m_series->append(setem);
        m_series->append(seten);
    }
    chart->addSeries(m_series);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    auto mlist = chart->legend()->markers();
    for (int i(0); i < CHANNEL_COUNT*2; i++) { //for all bars...
        mlist.at(i)->setVisible(false);
    }
    mlist.at(1)->setVisible(true);
    createAxises(chart);
    barClicked(0,0);
    return chart;
}

void MainWidget::createAxises(QChart *chart)
{
//    chart->createDefaultAxes();
    QCategoryAxis *axisY = new QCategoryAxis();
    for (int i = 1; i <= CHANNEL_COUNT; i++) {
        axisY->append(tr("%1 канал").arg(i), i * ChannelPercent);
    }
    axisY->setRange(0, CHANNEL_COUNT * ChannelPercent);
    axisY->setLabelsPosition(QCategoryAxis::AxisLabelsPosition::AxisLabelsPositionCenter);
    chart->setAxisY(axisY, m_series);

    QBarCategoryAxis *axisX = new QBarCategoryAxis;
    QStringList categories;
    for (int i = 0; i < dataTable.count(); i++) {
        categories << stepname(i+1, dataTable.at(i).second);
    }
    axisX->append(categories);
    chart->setAxisX(axisX, m_series);
    m_series->setBarWidth(1); //no space between bars!
    updateLegendLabel();
}

bool MainWidget::event(QEvent *event)
{
//    qDebug() << event;
    return QWidget::event(event);
}

void MainWidget::updateLegendLabel()
{
    QList<QBarSet *> sets = m_series->barSets();
    QString sstep = QString("Количество тактов: %1").arg(m_stepCount->value());
    if (m_replayButton->isChecked()) {
        sets.at(1)->setLabel(sstep.append("; ").append(m_replayLabel->text()));
    }
    else {
        sets.at(1)->setLabel(sstep.append(QString("; %1: %2; Общее время: %3").arg(m_replayLabel->text()).arg(m_replayCount->value()).arg(totaltime())));
    }
}

void MainWidget::newJob()
{
    if (getTitle()) {
        stopJob();
        m_stepCount->setValue(1);
        m_replayCount->setValue(1);
        m_replayButton->setChecked(false);
        m_pauseButton->setChecked(false);
        update();
    }
}

void MainWidget::openJob()
{
    QString label("Открытие режима");
    QString fname = QFileDialog::getOpenFileName(this, label, QString(), "Config (*.json);; all (*.*)");
    if (!fname.isEmpty()) {
        stopJob();
        m_pauseButton->setChecked(false);

        JConfig* config = new JConfig(this, fname);
        QVariantMap configmap;
        if (!config->readToMap(configmap)) {
            m_stepCount->setValue(1);
            m_replayCount->setValue(1);
            m_replayButton->setChecked(false);
            qDebug() << config->exceptionString;
        }
        m_chart->setTitle(configmap.value("Name", QString()).toString());
        QString replstr = configmap.value("Cycles", QString()).toString();
        int tickcount = configmap.value("TickCount", 0).toInt();
        bool isok;
        int cycles = replstr.toInt(&isok);
        m_replayCount->setValue(iif(isok,cycles,1));
        m_replayButton->setChecked(!isok);

        QVariantMap datamap = configmap.value("Ticks", QVariantMap()).toMap();
        dataTable.resize(tickcount);
        if (tickcount && tickcount == datamap.size()) {
            int tidx = 0;
            bool isall = true;
            foreach (auto stick, datamap) {
                QPair<int,int> tick;
                QStringList ltick = stick.value<QString>().split(':');
                if (ltick.length() == 2) {
                    tick.first = ltick[0].toInt(&isok);
                    isall = isall && isok;
                    tick.second = ltick[1].toInt(&isok);
                    isall = isall && isok;
                    if (!isok) {
                        tick.first = 0;
                        tick.second = 1;
                    }
                }
                else {
                    isall = false;
                    tick.first = 0;
                    tick.second = 1;
                }
                dataTable[tidx++] = tick;
                qDebug() << "tick:" << tidx << tick;
            }
            toLog(tr("Режим таймера открыт из файла: %1").arg(fname));
            if (!isall) {
                toLog(tr("Внимание: не все такты считаны!").arg(fname));
            }
        }
        else {
            dataTable[0].first = 0;
            dataTable[0].second = 1;
            toLog(tr("Данные из файла не загружены: %1").arg(fname));
        }
        m_stepCount->setValue(tickcount); // stepCountChanged(tickcount);
        update();
        config->deleteLater();
    }
}

void MainWidget::saveJob()
{
    QString label("Сохранение режима");
    QString fname = QFileDialog::getSaveFileName(this, label, QString(), "Config (*.json);; all (*.*)");
    if (!fname.isEmpty()) {
        JConfig* config = new JConfig(this, fname);
        QVariantMap configmap;
        configmap.insert("Name", m_chart->title());
        configmap.insert("Cycles", iif(isReplayEndless,"endless",QString::number(m_replayCount->value())));
        configmap.insert("TickCount", dataTable.size());
        QVariantMap datamap;
        QString tmpl_name("T%1");
        QString tmpl_data("%1:%2");
        int tick = 1;
        foreach (auto data, dataTable) {
            datamap.insert(tmpl_name.arg(tick++,3,10,QChar('0')), tmpl_data.arg(data.first).arg(data.second));
        }
        configmap.insert("Ticks", datamap);
        if (config->save(configmap)) {
            toLog(tr("Режим таймера успешно сохранён в файл: %1").arg(fname));
        }
        config->deleteLater();
    }
}

bool MainWidget::getTitle()
{
    bool ok;
    QString label("Наименование режима");
    QString text = QInputDialog::getText(this,
                                         label,
                                         QString(label).append(":").leftJustified(66,' '),
                                         QLineEdit::Normal,
                                         m_chart->title(),
                                         &ok);
    if (ok && !text.isEmpty())
      m_chart->setTitle(text);
    return ok;
}

void MainWidget::editTitle()
{
    getTitle();
}

void MainWidget::runJob(bool isChecked)
{
    if (isChecked) {
        loopCounter = 0;
        m_log->clear();
        toLog(tr("Запуск режима: %1").arg(m_chart->title()));
        if (timerID) killTimer(timerID);
        timerID = startTimer(1000);
        if (timerID) {
            if (isHWPowerGood()) {
                hwStatus = HWStatus_OK;
                setupTimer(0);
            }
            else {
                pauseTimerOnPowerBad();
            }
        } else {
            toLog("Something wrong...");
            updateFTDI(0);
        }
    }
    else {
        stopJob("Режим остановлен");
    }
}

void MainWidget::stopJob(QString stopMessage)
{
    stopTimer();
    if (stopMessage.isEmpty()) {
        if (m_runJobButton->isChecked()) {
            toLog("Режим остановлен");
        }
    }
    else {
        toLog(stopMessage);
    }
    m_runJobButton->setChecked(false);
    m_pauseButton->setChecked(false);
}

void MainWidget::timeToPause()
{
    if (stepStartPauseTime.isNull()) stepStartPauseTime = QDateTime::currentDateTimeUtc();
}

bool MainWidget::timeToUnPause()
{
    if (!m_pauseButton->isChecked() && hwStatus != HWStatus_POWER_ERROR) {
        stepPauseTime_mS += stepStartPauseTime.msecsTo(QDateTime::currentDateTimeUtc());
        stepStartPauseTime = QDateTime();
        return true;
    }
    return false;
}

void MainWidget::pauseFromUI(bool isChecked)
{
//    if (m_runJobButton->isChecked()) {
        if (isChecked) {
            timeToPause();
            if (m_runJobButton->isChecked()) toLog("Бесконечный такт включён!");
        }
        else { //            resumeTimer();
            timeToUnPause();
            if (m_runJobButton->isChecked()) toLog("Бесконечный такт выключён!");
        }
//    }
//    else {
//        m_pauseButton->setChecked(false);
//    }
}

void MainWidget::replayEndless(bool isChecked)
{
    if (isChecked) {
        m_replayLabel->setText("Бесконечный цикл!");
        m_replayCount->hide();
    }
    else {
        m_replayLabel->setText("Количество циклов");
        m_replayCount->show();
    }
    setReplayEndless(isChecked);
    updateLegendLabel();
}

void MainWidget::toPrint()
{
    if (!Printer) Printer = new QPrinter(QPrinter::HighResolution);
    QPrintDialog dialog(Printer, this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    int from = Printer->fromPage();
    int to = Printer->toPage();
    if (from <= 0 && to <= 0)
        Printer->setFromTo(1, 1 /*pageMap.keys().count()*/);

    printDocument(Printer);
}

void MainWidget::toPrintPreview()
{
    if (!Printer) Printer = new QPrinter(QPrinter::HighResolution);
    QPrintPreviewDialog preview(Printer, this);
    connect(&preview, SIGNAL(paintRequested(QPrinter*)),
            this, SLOT(printDocument(QPrinter*)));
    preview.exec();
}

void MainWidget::about()
{
    QString message = QString("Таймер управления нагрузкой, v.%1").arg(APP_VERSION);
    message.append(tr("<p> Автор: Валентин Зотов. %1г. </p>").arg(YEAR_OF_BUILD));
    message.append("<p> E-mail: wellman@mail.ru </p>"
                   "<p> <b> Powered by Qt </b> </p>");
    QMessageBox::information(this, "О программе", message);
}

//void MainWidget::zoomIn()
//{
//    if (zoomValue < 9) {
//        zoomValue += 1;
//        m_chart->zoomIn();
//    }
//}

//void MainWidget::zoomOut()
//{
//    if (zoomValue > 1) {
//        zoomValue -= 1;
//        m_chart->zoomOut();
//    }
//}

void MainWidget::stepCountChanged(int newCount)
{
    QList<QBarSet*> sets = m_series->barSets();
    const int diff = newCount - sets.at(0)->count();
    if (diff < 0) { //to remove...
        dataTable.resize(newCount);
        for (int ch = 0; ch<CHANNEL_COUNT; ch++) {
            QBarSet *setem = sets.at(ch*2);
            QBarSet *seten = sets.at(ch*2+1);
            setem->remove(setem->count() + diff, ::abs(diff));
            seten->remove(seten->count() + diff, ::abs(diff));
        }
    }
    else if (dataTable.size() == newCount) { //to refresh all...
        const int setscou = sets.at(0)->count();
        for (int i = 0; i < newCount; i++) {
            const int mask = dataTable.at(i).first;
            if (i < setscou) { //replace bar set...
                for (int ch = 0; ch<CHANNEL_COUNT; ch++) {
                    QBarSet *setem = sets.at(ch*2);
                    QBarSet *seten = sets.at(ch*2+1);
                    if (mask & (1<<ch))
                    { //to set...
                        setem->replace(i, 0);
                        seten->replace(i, ChannelPercent);
                    }
                    else
                    { //to clear...
                        setem->replace(i, ChannelPercent);
                        seten->replace(i, 0);
                    }
                }
            }
            else { //append to bar set...
                for (int ch = 0; ch<CHANNEL_COUNT; ch++) {
                    QBarSet *setem = sets.at(ch*2);
                    QBarSet *seten = sets.at(ch*2+1);
                    if (mask & (1<<ch)) {
                        setem->append(0);
                        seten->append(ChannelPercent);
                    }
                    else {
                        setem->append(ChannelPercent);
                        seten->append(0);
                    }
                }
            }
        }
    }
    else { //to append ...
        for (int i = 0; i < diff; i++) {
            dataTable.append(QPair<int,int>(1, get_UI_time_sec())); // on default engage first channel!
            for (int c = 0; c<CHANNEL_COUNT; c++) {
                QBarSet *setem = sets.at(c*2);
                QBarSet *seten = sets.at(c*2+1);
                setem->append(iif(c, ChannelPercent, 0));
                seten->append(iif(c, 0, ChannelPercent));
            }
        }
    }
    m_chartView->sizeHint();
    createAxises(m_chart);

    //auto update & scroll...
    scrollArea->ensureVisible(0, 0);
    scrollArea->ensureVisible(m_chartView->width(), 0);

    currentGUIIndex = newCount-1;
    stepUIRefresh(currentGUIIndex);
}

void MainWidget::replayCountChanged()
{
    replayCount = m_replayCount->value()-1;
    updateLegendLabel();
}

void MainWidget::stepUIRefresh(int index)
{
    m_stepSettings->setTitle(tr("Параметры такта № %1").arg(index+1));
    int seconds = dataTable.at(index).second;
    int hours = seconds/3600;
    int minutes = (seconds%3600)/60;
    int secs = seconds%60;
    m_hourLabel->setText(tr("Часы:%1").arg(hours,2, 10, QChar('0')));
    m_minuteLabel->setText(tr("Минуты:%1").arg(minutes,2, 10, QChar('0')));
    m_secondLabel->setText(tr("Секунды:%1").arg(secs,2, 10, QChar('0')));

    m_hourSetter->blockSignals(true);
    m_hourSetter->setValue(hours);
    m_hourSetter->blockSignals(false);
    m_minuteSetter->blockSignals(true);
    m_minuteSetter->setValue(minutes);
    m_minuteSetter->blockSignals(false);
    m_secondSetter->blockSignals(true);
    m_secondSetter->setValue(secs);
    m_secondSetter->blockSignals(false);
}

void MainWidget::barClicked(int index, int barset) {
    if (currentGUIIndex == index) {
        int ch = barset >> 1;
        QList<QBarSet *> sets = m_series->barSets();
        QBarSet *setem = sets.at(ch*2);
        QBarSet *seten = sets.at(ch*2+1);
        int cfg = dataTable.at(index).first;
        if (cfg & (1<<ch))
        { //to clear...
            cfg &= ~(1<<ch);
            setem->replace(index, ChannelPercent);
            seten->replace(index, 0);
        }
        else
        { //to set...
            cfg |= (1<<ch);
            setem->replace(index, 0);
            seten->replace(index, ChannelPercent);
        }
        dataTable.replace(index, QPair<int,int>(cfg, dataTable.at(index).second));
//        updateFTDI(dataTable[index].first);
        qDebug() << "Bar[" << barset << ":" << index + 1 << "] clicked; new value:" << dataTable.at(index);
    }
    else {
        currentGUIIndex = index;
        stepUIRefresh(index);
        qDebug() << "Bar new currentIndex[" << barset << ":" << index + 1 << "]";
    }
}
//void MainWidget::hover(bool status, int index)
//{
//    qDebug() << "Bar hovered[" << status << ":" << index + 1 << "]";
//}

void MainWidget::printDocument(QPrinter *printer)
{
    printer->setFromTo(1, 1/*pageMap.count()*/);

    QProgressDialog progress(tr("Подготовка к печати..."), tr("&Cancel"),
                             0, 1/*pageMap.count()*/, this);
    progress.setWindowModality(Qt::ApplicationModal);
    progress.setWindowTitle(tr("Печать"));
    progress.setMinimum(printer->fromPage() - 1);
    progress.setMaximum(printer->toPage());

    QPainter painter;
    painter.begin(printer);
    bool firstPage = true;

    for (int page = printer->fromPage(); page <= printer->toPage(); ++page) {

        if (!firstPage)
            printer->newPage();

        qApp->processEvents();
        if (progress.wasCanceled())
            break;

        printPage(page - 1, &painter, printer);
        progress.setValue(page);
        firstPage = false;
    }

    painter.end();
}

void MainWidget::printPage(int index, QPainter *painter, QPrinter *printer)
{
    (void)index;
    (void)printer;
    m_chartView->render(painter);
}

void MainWidget::stepTimeChanged()
{
    dataTable.replace(currentGUIIndex, QPair<int,int>(dataTable.at(currentGUIIndex).first, get_UI_time_sec()));
    stepUIRefresh(currentGUIIndex);
    createAxises(m_chart);
}

int MainWidget::get_UI_time_sec()
{
    int secs = m_hourSetter->value()*3600;
    secs += m_minuteSetter->value()*60;
    secs += m_secondSetter->value();
    return secs;
}

void MainWidget::setupTimer(int step)
{
    if (dataTable.length() <= step) {
        if (isReplayEndless || loopCounter < replayCount) {
            toLog(tr("Переход на цикл № %1").arg(++loopCounter+1));
            step = 0;
        }
    }
    currentTimerStep = step;
    if (dataTable.length() > step) {
        stepStartPauseTime = QDateTime();
        stepPauseTime_mS = 0;
        stepStartRunTime = QDateTime::currentDateTimeUtc();
        HWStatus hw = updateFTDI(dataTable.at(step).first);
        if (hw == HWStatus_OK) {
            toLog(tr("Выполняется такт № %1").arg(step+1));
        }
        else if (hw == HWStatus_PORT_ERROR) {
            stopJob(QStringLiteral("ВНИМАНИЕ: таймер остановлен из-за отказа электроники!"));
        }
        else {
            stopJob(QStringLiteral("Прилетел НЛО и отключил таймер на такте № %1").arg(step+1));
        }
    }
    else { //
        stopJob(QStringLiteral("Таймер выполнил поставленную задачу!"));
    }
}

void MainWidget::stopTimer()
{
    updateFTDI(0);
    disableTimer();
    currentTimerStep = -1;
}

//void MainWidget::resumeTimer()
//{
//    if (currentTimerStep >= 0) setupTimer(currentTimerStep);
//}

void MainWidget::disableTimer()
{
    if (timerID) {
        killTimer(timerID);
        timerID = 0;
    }
}

void MainWidget::setReplayEndless(bool isOn)
{
    isReplayEndless = isOn;
    if (m_runJobButton->isChecked()) {
        toLog(tr("Бесконечный цикл %1!").arg(iif(isOn,"включён","выключен")));
    }
}

void MainWidget::resumeTimerOnPowerOk()
{
    hwStatus = HWStatus_OK;
    if (timeToUnPause()) {
        toLog("ВНИМАНИЕ: таймер продолжил отсчёт времени после восстановления питания!");
    }
}

void MainWidget::pauseTimerOnPowerBad()
{
    timeToPause();
    hwStatus = HWStatus_POWER_ERROR;
    if (!m_pauseButton->isChecked()) {
        toLog(QStringLiteral("ВНИМАНИЕ: таймер приостановил отсчёт времени из-за отсутствия питания!"));
    }
}

void MainWidget::timerEvent(QTimerEvent *tevent)
{
    (void)tevent;
    qint64 elapsed_mS = stepStartRunTime.msecsTo(QDateTime::currentDateTimeUtc());
    if (isHWPowerGood()) {
        if (hwStatus == HWStatus_POWER_ERROR) {
            resumeTimerOnPowerOk();
        }
        if (stepStartPauseTime.isNull()) {
            if (elapsed_mS - stepPauseTime_mS + 500 >= dataTable.at(currentTimerStep).second * 1000) {
                setupTimer(currentTimerStep+1);
            }
        }
    }
    else {
        if (hwStatus != HWStatus_POWER_ERROR) {
            pauseTimerOnPowerBad();
        }
    }
}

HWStatus MainWidget::updateFTDI(int bitmask)
{
    ftHandle = iif(ftHandle, ftHandle, ftdilib::open(FTDI_SERIAL_NUMBER, &ftStatus));
    if(!ftHandle || ftStatus != FT_OK) { // Error Checking
        qDebug() << tr("FTDI open error with status code: %1").arg(ftStatus);
        closeFTDI();
        return HWStatus_PORT_ERROR;
    }

//    if (!isHWPowerGood()) {
//        return HWStatus_POWER_ERROR;
//    }

#ifdef HW_CHANNEL_REMASK
    int remask = 0;
    const int arrsize = (int)ARRAYSIZE(hw_remask);
    for (int i = 0; i < arrsize; i++) {
        if (bitmask & (1<<i)) {
            remask |= hw_remask[i];
        }
    }
    bitmask = remask;
#endif

    UCHAR mask = FTDI_DDIR_MASK | (bitmask & 0x0F);
    ftStatus = FT_SetBitMode(ftHandle, mask, FT_BITMODE_CBUS_BITBANG);
    if(ftStatus != FT_OK) { // Error Checking
        toLog(tr("FTDI SetBitMode error with status code: %1").arg(ftStatus));
        closeFTDI();
        return HWStatus_PORT_ERROR;
    }
    return HWStatus_OK;
}

int MainWidget::getAsyncFTDIData()
{
    ftStatus = FT_SetBitMode(ftHandle, 0, FT_BITMODE_ASYNC_BITBANG);
    if(ftStatus != FT_OK) { // Error Checking
        toLog(tr("FTDI SetBitMode(1) error with status code: %1").arg(ftStatus));
        closeFTDI();
        return 0;
    }

    UCHAR maskValue = 0;
    ftStatus = FT_GetBitMode(ftHandle, &maskValue); //gets Asynchronous BBM
    if (ftStatus != FT_OK) { // Error Checking
        qDebug() << tr("FTDI GetBitMode(1) error with status code: %1").arg(ftStatus);
        return 0;
    }
    return (int)maskValue;
}

void MainWidget::closeFTDI()
{
    if (ftHandle) {
        FT_Close(ftHandle);
        ftHandle = 0;
    }
}

QString MainWidget::stepname(int step, int seconds)
{
    int hours = seconds/3600;
    int minutes = (seconds%3600)/60;
    return QString("%1 %2:%3:%4").arg(step).arg(hours,2, 10, QChar('0')).arg(minutes,2, 10, QChar('0')).arg(seconds%60,2, 10, QChar('0'));
}

QString MainWidget::totaltime()
{
    int seconds = 0;
    for (int i = 0; i<dataTable.count(); i++) {
        seconds+=dataTable.at(i).second;
    }
    return stepname(0, seconds * m_replayCount->value()).mid(2);
}

void MainWidget::toLog(const QString &line)
{
    m_log->append(QDateTime::currentDateTime().toString(DateTime_format_full).append(' ').append(line));
}

QSize ScrolledChartView::sizeHint() const
{
//    qDebug() << scrollArea->geometry().height();
    if (scrollArea && chartView) {
        int wcalced = dataTable.count()*80+80;
        if (wcalced < scrollArea->geometry().width()) wcalced = scrollArea->geometry().width();
        chartView->setGeometry(scrollArea->geometry().x(),scrollArea->geometry().y(), wcalced, scrollArea->geometry().height());
        return scrollArea->geometry().size();
    }
    return QSize(20, 20);
}
