#ifndef CHART_H
#define CHART_H

#include <QWidget>
#include <QChartView>
#include <QLineSeries>
#include <QDateTimeAxis>
#include <QValueAxis>
#include <QAreaSeries>

class Chart : public QChartView
{
    Q_OBJECT
public:
    explicit Chart(QWidget *parent = nullptr);

    void attachSeries(QAbstractSeries *series, int sensorType);

private:
    QChart *chart;
    QScrollBar *scrollBar;
    QDateTimeAxis *axisX;
    QMultiMap<int, QValueAxis*> listAxisY;
    QVector<QAbstractSeries*> listSeries;

    QValueAxis *axisAnalogY;
    QValueAxis *axisDiscreteY;

    QAbstractSeries *selectedSeries = nullptr;
    QValueAxis *selectedAxis = nullptr;
    QVector<QAbstractSeries*> selectedSeriresList;

    void wheelEvent(QWheelEvent *event);
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent *event);
    void setRangeAxisX();
    void setScrollBarRange();

    int mouseOffset;
    double scrollValue = 0;
    double currentRange;
    bool isMultyChart = false;

    const int defaultVisibleRangeHrs = 1;
    const int scrollValueStepSecs = 60;
    const int maxScrollableRangeHrs = 24 * 30;
    const double maxVisibleRangeHrs = 24 * 0.5;
    const double minVisibleRangeHrs = 0.1;

private slots:
    void scrollValueChange(int value);
    void plusButtonClicked();
    void minusButtonClicked();

public slots:
    void seriesClicked(QAbstractSeries *series);
    void modeChanged(bool multyChartMode);

};

#endif // CHART_H
