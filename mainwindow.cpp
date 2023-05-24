#include "chart.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setStyleSheet("background-color: black;");
    Chart *chart = new Chart;
    setCentralWidget(chart);
}

MainWindow::~MainWindow()
{
    delete ui;
}

