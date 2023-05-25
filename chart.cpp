#include "chart.h"

#include <QPushButton>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QDebug>
#include <QRandomGenerator>
#include <QDateTime>

Chart::Chart(QWidget *parent) : QChartView(parent)
{
    setRenderHint(QPainter::Antialiasing);

    chart = new QChart();
    chart->setBackgroundBrush(QBrush(QColor(255, 255, 255, 0)));
    chart->legend()->hide();
    setChart(chart);

    axisX = new QDateTimeAxis;
    axisX->setGridLineColor(QColor(255, 255, 255, 85));
    axisX->setLineVisible(false);
    axisX->setLabelsColor(Qt::white);
    axisX->setFormat("dd.MM hh:mm");
    axisX->setTitleText(" ");
    chart->addAxis(axisX, Qt::AlignBottom);

    axisDiscreteY = new QValueAxis;
    axisDiscreteY->hide();
    axisDiscreteY->setRange(0, 1);
    axisDiscreteY->setTickCount(2);
    chart->addAxis(axisDiscreteY, Qt::AlignLeft);

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

void Chart::attachSeries(QAbstractSeries *series, int sensorType)
{
    listSeries.append(series);
    series->attachAxis(axisX);
    series->hide();

    if(listAxisY.contains(sensorType)){
        QValueAxis *axis = listAxisY.value(sensorType);
        series->attachAxis(axis);
    }
    else{
        axisAnalogY = new QValueAxis;
        axisAnalogY->hide();
        axisAnalogY->setTickCount(5);
        axisAnalogY->setLabelsColor(Qt::white);
        axisAnalogY->setGridLineColor(QColor(255, 255, 255, 85));
        axisAnalogY->setLineVisible(false);
        axisAnalogY->setLabelFormat("%i");
        axisAnalogY->setRange(0, 100); //TODO в заисимости от типа сенсора, задается соответствующий ренж

        listAxisY.insert(sensorType, axisAnalogY);

        chart->addAxis(axisAnalogY, Qt::AlignLeft);
        series->attachAxis(axisAnalogY);
    }

    chart->addSeries(series);
}

void Chart::seriesClicked(QAbstractSeries *series)
{
    QLineSeries *line = qobject_cast<QLineSeries*>(series);
    if(line != nullptr){
        if(selectedSeries != nullptr){
            if(isMultyChart){
                selectedSeries->setOpacity(0.5);
            }
            else{
                selectedSeries->hide();
            }
        }
        line->show();
        line->setOpacity(1);
        selectedSeries = line;

        if(isMultyChart)
            selectedSeriresList.append(line);

        QList<QAbstractAxis *> axes = line->attachedAxes();
        for(auto a : axes){
            QValueAxis *value = qobject_cast<QValueAxis*>(a);
            if(value != nullptr){
                if(selectedAxis != nullptr)
                    selectedAxis->hide();

                value->show();
                selectedAxis = value;
            }
        }
    }

    QAreaSeries *area = qobject_cast<QAreaSeries*>(series);
    if(area != nullptr){
        if(selectedSeries != nullptr){
            if(isMultyChart){
                selectedSeries->setOpacity(0.15);
            }
            else{
                selectedSeries->hide();
            }
        }
        area->show();
        area->setOpacity(0.3);
        selectedSeries = area;

        if(isMultyChart)
            selectedSeriresList.append(area);

        if(selectedAxis != nullptr)
            selectedAxis->hide();

        axisDiscreteY->show();
        selectedAxis = axisDiscreteY;
    }
}

void Chart::modeChanged(bool multyChartMode)
{
    isMultyChart = multyChartMode;

    if(!multyChartMode && !selectedSeriresList.isEmpty()){
        for(auto s : selectedSeriresList){
            if(s != selectedSeries)
                s->hide();
        }
    }
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
