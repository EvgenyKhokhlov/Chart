#ifndef CHART_H
#define CHART_H

#include <QWidget>
#include <QChartView>
#include <QSplineSeries>
#include <QDateTimeAxis>
#include <QValueAxis>
#include <QAreaSeries>

class Chart : public QChartView
{
    Q_OBJECT
public:
    explicit Chart(QWidget *parent = nullptr);

private:
    QChart *chart;
    QScrollBar *scrollBar;
    QDateTimeAxis *axisX;
    QValueAxis *axisAnalogY;
    QValueAxis *axisDiscreteY;

    void wheelEvent(QWheelEvent *event);
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent *event);
    void setRangeAxisX();
    void setScrollBarRange();

    int mouseOffset;
    double scrollValue = 0;
    double currentRange;

    const int defaultVisibleRangeHrs = 1;
    const int scrollValueStepSecs = 60;
    const int maxScrollableRangeHrs = 24 * 30;
    const double maxVisibleRangeHrs = 24 * 0.5;
    const double minVisibleRangeHrs = 0.1;

private slots:
    void scrollValueChange(int value);
    void plusButtonClicked();
    void minusButtonClicked();

};

#endif // CHART_H
