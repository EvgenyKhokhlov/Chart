#include "chart.h"

#include <QPushButton>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QDebug>
#include <QRandomGenerator>
#include <QDateTime>

Chart::Chart(QWidget *parent) : QChartView(parent)
{
    QLineSeries *seriesLine = new QLineSeries();

    setRenderHint(QPainter::Antialiasing);

    QPen linePen;
    linePen.setWidth(2);
    seriesLine->setPen(linePen);

    QLineSeries *upper = new QLineSeries;
    QLineSeries *lower = new QLineSeries;
    QAreaSeries *seriesArea = new QAreaSeries(upper, lower);

    QPen areaPen;
    areaPen.setWidth(0);
    seriesArea->setPen(areaPen);
    seriesArea->setOpacity(0.3);

    int previous = 0;
    for(int i = 0; i < 60 * 12; i++){
        seriesLine->append(QDateTime::currentDateTime().addSecs(-i * 60).toMSecsSinceEpoch(),
                             QRandomGenerator::global()->bounded(0, 101));

        int current = QRandomGenerator::global()->bounded(0, 2);

        if(current == previous){
            upper->append(QDateTime::currentDateTime().addSecs(-i * 60).toMSecsSinceEpoch(), current);
        }
        else if(current == 1){
            upper->append(QDateTime::currentDateTime().addSecs(-i * 60).toMSecsSinceEpoch(), 0);
            upper->append(QDateTime::currentDateTime().addSecs(-i * 60).toMSecsSinceEpoch(), 1);
        }
        else{
            upper->append(QDateTime::currentDateTime().addSecs(-i * 60).toMSecsSinceEpoch(), 1);
            upper->append(QDateTime::currentDateTime().addSecs(-i * 60).toMSecsSinceEpoch(), 0);
        }

        lower->append(QDateTime::currentDateTime().addSecs(-i * 60).toMSecsSinceEpoch(), 0);
        previous = current;
    }

    seriesArea->setColor(Qt::white);
    seriesLine->setColor(Qt::white);

    chart = new QChart();
    chart->setBackgroundBrush(QBrush(QColor(255, 255, 255, 0)));
    chart->legend()->hide();
    chart->addSeries(seriesLine);
    chart->addSeries(seriesArea);
    setChart(chart);

    axisX = new QDateTimeAxis;
    axisX->setGridLineColor(QColor(255, 255, 255, 85));
    axisX->setLineVisible(false);
    axisX->setLabelsColor(Qt::white);
    axisX->setFormat("dd.MM hh:mm");
    axisX->setTitleText(" ");
    axisX->setRange(QDateTime::currentDateTime().addSecs(-60 * 60 * 2), QDateTime::currentDateTime());
    chart->addAxis(axisX, Qt::AlignBottom);
    seriesArea->attachAxis(axisX);
    seriesLine->attachAxis(axisX);

    axisAnalogY = new QValueAxis;
    axisAnalogY->setTickAnchor(10);
    axisAnalogY->setLabelsColor(Qt::white);
    axisAnalogY->setGridLineColor(QColor(255, 255, 255, 85));
    axisAnalogY->setLineVisible(false);
    axisAnalogY->setLabelFormat("%i");
    axisAnalogY->setRange(0, 100);
    chart->addAxis(axisAnalogY, Qt::AlignLeft);
    seriesLine->attachAxis(axisAnalogY);

    axisDiscreteY = new QValueAxis;
    axisDiscreteY->hide();
    axisDiscreteY->setRange(0, 1);
    chart->addAxis(axisDiscreteY, Qt::AlignLeft);
    seriesArea->attachAxis(axisDiscreteY);

    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(20);

    layout->addSpacing(20);
    layout->addLayout(buttonLayout);
    layout->addStretch();

    QPushButton *minusButton = new QPushButton("-");
    connect(minusButton, &QPushButton::clicked, this, &Chart::minusButtonClicked);
    minusButton->setFocusPolicy(Qt::NoFocus);
    minusButton->setFixedSize(72, 72);
    minusButton->setStyleSheet("QPushButton{"
                              "background-color: rgb(28, 28, 30);"
                              "border-radius: 10px;"
                              "image: url(:/new/prefix1/icons/select_but_active.png);"
                              "}");
    buttonLayout->addStretch();
    buttonLayout->addWidget(minusButton);

    QPushButton *plusButton = new QPushButton("+");
    connect(plusButton, &QPushButton::clicked, this, &Chart::plusButtonClicked);
    plusButton->setFocusPolicy(Qt::NoFocus);
    plusButton->setFixedSize(72, 72);
    plusButton->setStyleSheet("QPushButton{"
                              "background-color: rgb(28, 28, 30);"
                              "border-radius: 10px;"
                              "image: url(:/new/prefix1/icons/select_but_active.png);"
                              "}");
    buttonLayout->addWidget(plusButton);
    buttonLayout->addStretch();

    scrollBar = new QScrollBar(Qt::Horizontal);
    scrollBar->setInvertedAppearance(true);
    scrollBar->setPageStep(0);
    connect(scrollBar, &QScrollBar::valueChanged, this, &Chart::scrollValueChange);
    scrollBar->setStyleSheet("QScrollBar:horizontal {"
                             "background-color: rgba(255, 255, 255, 0);"
                             "height: 10px;"
                             "}"
                             "QScrollBar::sub-page:horizontal { background-color: rgba(255, 255, 255, 0);}"
                             "QScrollBar::add-page:horizontal { background-color: rgba(255, 255, 255, 0);}"
                             "QScrollBar::add-line:horizontal { height: 0px;}"
                             "QScrollBar::sub-line:horizontal { height: 0px;}"
                             "QScrollBar::handle:horizontal {"
                             "min-width: 50px;"
                             "background: #3E3E42;"
                             "border-radius: 5px;"
                             "}");
    layout->addWidget(scrollBar);

    currentRange = defaultVisibleRangeHrs;
    setScrollBarRange();
    setRangeAxisX();
}

void Chart::wheelEvent(QWheelEvent *event)
{
    if(event->angleDelta().ry() > 0){
        if(scrollValue >= scrollBar->maximum()) return;

        if(currentRange > 1)
            scrollValue += currentRange;
        else
            scrollValue++;
    }else{
        if(scrollValue < 0) return;

        if(currentRange > 1)
            scrollValue -= currentRange;
        else
            scrollValue--;

        if(scrollValue < 0)
            scrollValue = 0;
    }
    scrollBar->setValue(scrollValue);
    setRangeAxisX();
}

void Chart::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() != Qt::LeftButton) return;

    if(mouseOffset - event->pos().x() < 0){
        if(scrollValue >= scrollBar->maximum()) return;

        if(currentRange > 1)
            scrollValue += currentRange;
        else
            scrollValue++;
    }else{
        if(scrollValue < 0) return;

        if(currentRange > 1)
            scrollValue -= currentRange;
        else
            scrollValue--;
    }

    scrollBar->setValue(scrollValue);
    setRangeAxisX();
}

void Chart::mousePressEvent(QMouseEvent *event)
{
    mouseOffset = event->pos().x();
}

void Chart::scrollValueChange(int value)
{
    scrollValue = value;

    setRangeAxisX();
}

void Chart::setScrollBarRange()
{
    scrollBar->setRange(0, (maxScrollableRangeHrs - currentRange) * 60);
}

void Chart::setRangeAxisX()
{
    axisX->setRange(QDateTime::currentDateTime().addSecs(-60 * 60 * currentRange - scrollValue * scrollValueStepSecs),
                    QDateTime::currentDateTime().addSecs(-scrollValue * scrollValueStepSecs));
}

void Chart::plusButtonClicked()
{
    if(currentRange <= minVisibleRangeHrs) return;

    currentRange /= 2;

    setRangeAxisX();
    setScrollBarRange();
}

void Chart::minusButtonClicked()
{
    if(currentRange >= maxVisibleRangeHrs) return;

    currentRange *= 2;

    setRangeAxisX();
    setScrollBarRange();
}
