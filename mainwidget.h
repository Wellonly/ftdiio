
#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QtCharts/QChartGlobal>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtWidgets/QWidget>
#include <QtWidgets/QGraphicsWidget>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGraphicsGridLayout>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGroupBox>
#include <QtCharts/QBarSeries>
#include <QCheckBox>
#include <QLabel>
#include <QPercentBarSeries>
#include <QPrinter>
#include <QPushButton>
#include <QSlider>
#include <QStackedBarSeries>
#include <QTextEdit>

QT_CHARTS_USE_NAMESPACE
class ScrolledChartView;

enum HWStatus {
    HWStatus_OK,
    HWStatus_POWER_ERROR,
    HWStatus_PORT_ERROR,
    HWStatus_undefined,
};

class MainWidget : public QWidget
{
    Q_OBJECT
public:
    MainWidget(QWidget *parent = 0);
    ~MainWidget();
    QChart *createAreaChart() const;
    QChart *createBarChart(int valueCount);
//    void showLegendSpinbox();
//    void hideLegendSpinbox();

//    QScrollArea *scrollArea() const;

    void createAxises(QChart *chart);

    bool event(QEvent *event) override;

    bool getTitle();

    void updateLegendLabel();
    
    void stepUIRefresh(int index);

public Q_SLOTS:
    void newJob();
    void openJob();
    void saveJob();
    void editTitle();

    void runJob(bool isChecked);

    void stopJob(QString stopMessage = QString());
    void pauseFromUI(bool isChecked);
    void replayEndless(bool isChecked);
    void toPrint();
    void toPrintPreview();
    void about();
//    void zoomIn();
//    void zoomOut();
    void stepCountChanged(int newCount);
    void replayCountChanged();

    void barClicked(int index, int barset);
//    void hover(bool status, int index);

    void printDocument(QPrinter *printer);
    void printPage(int index, QPainter *painter, QPrinter *printer);
    void stepTimeChanged();
    int get_UI_time_sec();

    void setupTimer(int step);
    void stopTimer();
//    void resumeTimer();
    void disableTimer();
    void setReplayEndless(bool isOn);


    void timerEvent(QTimerEvent* tevent) override;

    HWStatus updateFTDI(int bitmask);
    int  getAsyncFTDIData();
    void closeFTDI();


private:
    QString stepname(int step, int seconds);
    QString totaltime();
    void toLog(const QString& line);

//    QChart *m_sliderChart;
//    QChartView *m_sliderChartView;
//    QScrollArea* m_scrollArea;

    QChart *m_chart;
//    QBarSeries *m_series; // QPercentBarSeries *;
    QStackedBarSeries *m_series;

    QSlider *m_hourSetter;
    QSlider *m_minuteSetter;
    QSlider *m_secondSetter;
    QLabel  *m_hourLabel;
    QLabel  *m_minuteLabel;
    QLabel  *m_secondLabel;

    ScrolledChartView *m_chartView;
    QTextEdit *m_log;
    QGridLayout *m_mainLayout;
    QGridLayout *m_buttonLayout;
    QGridLayout *m_fontLayout;

    QGroupBox *m_modeSettings;
    QGroupBox *m_stepSettings;
    QSpinBox *m_stepCount;
    QSpinBox *m_replayCount;
    QLabel *m_replayLabel;

    QPushButton *m_runJobButton;
    QPushButton *m_pauseButton;
    QPushButton *m_replayButton;

    int timerID = 0;

    friend class ScrolledChartView;
    void resumeTimerOnPowerOk();
    void pauseTimerOnPowerBad();
    void timeToPause();
    bool timeToUnPause();
};

class ScrolledChartView : public QChartView
{
    Q_OBJECT
public:
    ScrolledChartView(QChart *chart, QWidget *parent = Q_NULLPTR, MainWidget *mainW = 0)
        : QChartView(chart, parent), mainWidget(mainW) {}

    QSize sizeHint() const override;

private:
    MainWidget *mainWidget;
};
#endif // MAINWIDGET_H
