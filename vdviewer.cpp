#include "vdviewer.h"
#include "csvreader.h"
#include <QMenu>
#include <QMenuBar>
#include <QFileDialog>
#include <QStatusBar>
#include <QValueAxis>
#include <QSettings>

/**
 * @brief Class constructor
 * @param parent
 */
VdViewer::VdViewer(QWidget *parent) : QMainWindow(parent)
{
    readSettings();
    setWindowTitle("vdViewer");
    this->resize(winWidth, winHeight);
    setWindowIcon(QIcon(":/images/diag1.png"));

    /* Menu File */
    QAction* openAct = new QAction(tr("&Открыть"), this);
    QAction* closeAct = new QAction(tr("&Закрыть"), this);
    QAction* quitAct = new QAction(tr("&Выход"), this);
    QMenu* menuFile = menuBar()->addMenu(tr("&Файл"));
    openAct->setEnabled(true);
    menuFile->addAction(openAct);
    menuFile->addAction(closeAct);
    menuFile->addSeparator();
    menuFile->addAction(quitAct);

    /* Menu Encoding */
    QMenu* menuEncoding = menuBar()->addMenu(tr("&Кодировка"));
    utf8Act = new QAction(tr("&UTF-8"), this);
    ansiAct = new QAction(tr("&CP1251"), this);
    utf8Act->setCheckable(true);
    utf8Act->setChecked(isUtf8Encoding);
    ansiAct->setCheckable(true);
    ansiAct->setChecked(!isUtf8Encoding);
    menuEncoding->addAction(utf8Act);
    menuEncoding->addAction(ansiAct);

    /* Menu Settings */
    showLegend = new QAction(tr("&Показывать легенду"), this);
    showLegend->setCheckable(true);
    showLegend->setChecked(true);
    showLegend->setEnabled(false);
    QAction* csvSettings = new QAction(tr("&Настройки лог-файла"), this);
    QMenu* menuSettings = menuBar()->addMenu(tr("&Настройки"));
    menuSettings->addAction(showLegend);
    menuSettings->addAction(csvSettings);


    /* Menu Help */
    QAction* manualAct = new QAction(tr("&Инструкция"), this);
    QAction* aboutAct = new QAction(tr("&О программе"), this);
    QMenu* menuHelp = menuBar()->addMenu(tr("&Справка"));
    menuHelp->addAction(manualAct);
    menuHelp->addAction(aboutAct);

    /**/
    connect(openAct, SIGNAL(triggered()), this, SLOT(openAct_triggered()));
    connect(closeAct, SIGNAL(triggered()), this, SLOT(closeAct_triggered()));
    connect(quitAct, SIGNAL(triggered()), this, SLOT(onQuitAct_triggered()));
    connect(showLegend, SIGNAL(toggled(bool)), this, SLOT(showLegendAct_toggled(bool)));
    connect(csvSettings, SIGNAL(triggered()), this, SLOT(setCsvSettingsAct_triggered()));
    connect(manualAct, SIGNAL(triggered()), this, SLOT(manualAct_triggered()));
    connect(utf8Act, SIGNAL(triggered()), this, SLOT(utf8Act_triggered()));
    connect(ansiAct, SIGNAL(triggered()), this, SLOT(ansiAct_triggered()));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(aboutAct_triggered()));

    TsStr << "TIME" << "STAMP" << "Time" << "Stamp"
          << "ВРЕМЯ" << "МЕТКА" << "Время" << "Метка";

    statusBar()->showMessage("");
}

/**
 * @brief Class destructor
 */
VdViewer::~VdViewer()
{
    writeSettings();
    closeAct_triggered();
}

/**
 * @brief VdViewer::onKeyPressed
 * @param key
 */
void VdViewer::onKeyPressed(int key)
{
    QPointF ptVal, ptPos;
    qreal linePosX;
    int dataPoints = prmRowCount - (prmNamesLine + prmNamesCount);

    switch (key)
    {
    case 0: // left arrow key pressed
        if (trackLinePosX > 0) trackLinePosX--;
        break;
    case 1: // right arrow key pressed
        if (trackLinePosX < dataPoints -1) trackLinePosX++;
        break;
    case 2: // home key pressed
        trackLinePosX = 0;
        break;
    case 3: // end key pressed
        trackLinePosX = dataPoints -1;
        break;
    }

    ptVal.setX(trackLinePosX);
    ptVal.setY(0);
    ptPos = chartView->chart()->mapToPosition(ptVal);
    linePosX = ptPos.x();
    lineItem->setLine(linePosX, chartView->chart()->plotArea().top(),
                      linePosX, chartView->chart()->plotArea().bottom());

    tableWidget->selectRow(trackLinePosX);

    fillPrmLabels();
    lineItem->show();
}

/**
 * @brief VdViewer::onResizeChart
 */
void VdViewer::onResizeChart()
{
    lineItem->hide();
}

/**
 * @brief VdViewer::setDefaultSettings
 */
void VdViewer::setDefaultSettings()
{
    prmNamesLine = 3;
    prmNamesCount = 3;
}

/**
 * @brief VdViewer::closeAct_triggered
 */
void VdViewer::closeAct_triggered()
{
    if (mainTabWidget != nullptr) {
        disconnect(chartView, SIGNAL(keyPressed(int)), this, SLOT(onKeyPressed(int)));
        disconnect(chartView, SIGNAL(resizeChart()), this, SLOT(onResizeChart()));
        delete mainTabWidget;
        mainTabWidget = nullptr;
    }
}

/**
 * @brief VdViewer::openAct_triggered
 */
void VdViewer::openAct_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                    tr("Open File"), pathLogs, tr("csv (*.csv *.CSV)"));

    pathLogs = QFileInfo(fileName).path();

    CsvReader *csv = new CsvReader(this, fileName, utf8Act->isChecked());
    QList<QStringList> csvData;

    csv->setSeparator(csvSep);

    /* Reading data from a CSV file into memory */
    if (csv->fileOpen()) {
        csv->csvRead(csvData);
        statusBar()->showMessage(tr("Открыт файл: ") + fileName);

        prmRowCount = csvData.size();

        /* Get number of columns */
        int totalColumnsCount = 0;
        for (auto i = 0; i < prmNamesCount; i++) {
            int len = csvData.at(prmNamesLine+i).size();
            if (totalColumnsCount < len) {
                totalColumnsCount = len;
            }
        }

        /* Find empty columns */
        QList<int> emptyColumns;
        for (auto i = 1; i < totalColumnsCount; i++) {
            bool isEmty = true;
            for (auto k = 0; k < prmNamesCount; k++) {
                isEmty = isEmty && csvData.at(prmNamesLine + k).at(i).isEmpty();
            }
            if (isEmty) emptyColumns << i;
        }

        /* Creating parameter lists */
        QStringList strlist;
        prmStrList.clear();
        tsCollumnIndex.clear();
        for (auto i = 1; i < totalColumnsCount; ++i) {
            if (emptyColumns.contains(i)) continue;
            for (auto j = prmNamesLine; j < prmRowCount; ++j) {
                if (j > (prmNamesLine + prmNamesCount) && !csvData.at(j).at(0).isEmpty()) {
                    continue;
                }
                int len = csvData.at(j).size();
                if (len < totalColumnsCount && i >= len) strlist.append("");
                else strlist.append(csvData.at(j).at(i));

                if (TsStr.contains(strlist.at(strlist.size()-1))) {
                    if (!tsCollumnIndex.contains(i)) tsCollumnIndex << i;
                }
            }
            prmStrList.append(strlist);
            strlist.clear();
        }

        /* Creating a parameter selection window */
        prmSelectionWindow = new QDialog(this);
        prmSelectionWindow->setWindowTitle(tr("Выбор параметров для графика"));
        prmSelectionWindow->resize(300, 150);
        prmSelectionWindow->setModal(true);
        prmSelectionWindow->setWindowFlags((prmSelectionWindow->windowFlags())
                                           & (~Qt::WindowContextHelpButtonHint));
        prmSelectionWindow->setAttribute(Qt::WA_DeleteOnClose);

        prmSelector.clear();
        prmNameList.clear();
        QVBoxLayout *vLayout = new QVBoxLayout;
        for (auto i = 0; i < prmStrList.size(); ++i) {
            prmSelector.append(new QCheckBox);
            prmSelector.at(i)->setCheckable(true);
            prmSelector.at(i)->setChecked(false);
            if (tsCollumnIndex.contains(i+1)) {
                prmSelector.at(i)->setVisible(false);
            }
            QString prmNameString;
            for (auto k = 0; k < prmNamesCount; ++k) {
                prmNameString.append(" " + prmStrList.at(i).at(k));
            }
            prmNameList << prmNameString;
            prmSelector.at(i)->setText(prmNameString);
            vLayout->addWidget(prmSelector.at(i));
        }

        QPushButton *showGraphButton = new QPushButton(tr(" Показать "));
        QHBoxLayout *hLayout = new QHBoxLayout;
        connect(showGraphButton, SIGNAL(clicked()), this, SLOT(onShowGraphClicked()));
        hLayout->addStretch();
        hLayout->addWidget(showGraphButton);
        hLayout->addStretch();
        vLayout->addLayout(hLayout);

        prmSelectionWindow->setLayout(vLayout);
        prmSelectionWindow->show();
    }
    delete csv;
    csv = nullptr;
}

/**
 * @brief VdViewer::onShowGraphClicked
 */
void VdViewer::onShowGraphClicked()
{
    prmSelectionWindow->close();

    if (mainTabWidget != nullptr) {
        closeAct_triggered();
    }

    /* Create main widget */
    showLegend->setEnabled(true);
    mainTabWidget = new QTabWidget(this);
    QWidget* graphTab = new QWidget(mainTabWidget);
    QWidget* tableTab = new QWidget(mainTabWidget);
    mainTabWidget->addTab(tableTab, tr("Таблица"));
    mainTabWidget->addTab(graphTab, tr("График"));
    mainTabWidget->setCurrentIndex(1);
    setCentralWidget(mainTabWidget);
    chartLayout = new QVBoxLayout(graphTab);
    chartView = new ChartView;
    chartLayout->addWidget(chartView);
    QVBoxLayout *tableLayout = new QVBoxLayout(tableTab);
    tableWidget = new QTableWidget;
    tableLayout->addWidget(tableWidget);

    /* Writing data to the table and graph */
    tableWidget->setRowCount(prmRowCount - (prmNamesLine + prmNamesCount));
    tableWidget->setColumnCount(prmStrList.size());
    tableWidget->setShowGrid(true);
    tableWidget->setEditTriggers(QTableWidget::NoEditTriggers);
    QStringList tableHeader;
    QHBoxLayout *labelLayout = new QHBoxLayout;
    prmLabel.clear();
    prmVisible.clear();
    for (auto i = 0; i < prmStrList.size(); ++i) {
        QString prmNameString;
        for (auto j = 0; j < prmNamesCount; ++j) {
            prmNameString.append(prmStrList.at(i).at(j));
            if (j < prmNamesCount-1) prmNameString.append('\n');
        }
        tableHeader << prmNameString;
        for (auto k = 0; k < prmStrList.at(i).size() - prmNamesCount; ++k) {
            tableWidget->setItem(k, i, new QTableWidgetItem(prmStrList.at(i).at(k + prmNamesCount)));
            tableWidget->item(k, i)->setTextAlignment(Qt::AlignCenter);
        }
        prmLabel << new QLabel;
        setupLabel(prmLabel.at(i));
        if (tsCollumnIndex.contains(i+1) || (!prmSelector.at(i)->isChecked())) {
            prmLabel.at(i)->setVisible(false);
        }
        else {
            prmVisible << i;
            prmLabel.at(i)->setToolTip(prmNameList.at(i));
        }
        labelLayout->addWidget(prmLabel.at(i));
    }
    deltaLabel = nullptr;
    if (prmVisible.size() == 2) {
        deltaLabel = new QLabel;
        setupLabel(deltaLabel);
        labelLayout->addWidget(deltaLabel);
    }
    fillPrmLabels();
    chartLayout->addLayout(labelLayout);
    tableWidget->setHorizontalHeaderLabels(tableHeader);
    tableWidget->resizeColumnsToContents();
    tableWidget->horizontalHeader()->setFocus();

    tableWidget->setStyleSheet("QTableWidget {"
                               "color: rgb(10, 120, 10);"
                               "background color: (100, 0, 100);"
                                            "}"
    );

    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setCursor(Qt::CrossCursor);

    /* Creating lists of parameter values for graphs */
    double value;
    int mrk_cnt = 0;
    bool success;
    try {
        for (auto k = 0; k < prmStrList.size(); k++) {
            if (!prmSelector.at(k)->isChecked()) continue;
            QList<qreal> *Column = new QList<qreal>;
            for (auto i = prmNamesCount; i < prmStrList.at(k).size(); ++i) {
                value = prmStrList.at(k).at(i).split(csvSep).first().toFloat(&success);
                if (success) {
                    Column->append(value);
                }
                else {
                    i++;
                    continue;
                }
            }
            /* Writing data to charts */
            QLineSeries *series = new QLineSeries(chartView);
            for (auto i = 0; i < Column->size(); ++i) {
                if (!Column->isEmpty() && (prmSelector.at(k)->isChecked())) {
                    series->append(i, Column->at(i));
                }
            }
            if (k < prmStrList.size()) {
                series->setName(prmSelector.at(k)->text());
            }
            chartView->chart()->addSeries(series);
            QPalette pal = prmLabel.at(k)->palette();
            pal.setColor(QPalette::WindowText,
                         chartView->chart()->legend()->markers().at(mrk_cnt++)->brush().color());
            prmLabel.at(k)->setPalette(pal);
            delete Column;
        }
    } catch (...) {
        ; // some shit happened
        return;
    }

    QPen penLine;
    lineItem = new QGraphicsLineItem(chartView->chart());
    penLine.setColor(QColor(0, 0, 0));
    penLine.setStyle(Qt::SolidLine);
    penLine.setWidthF(1.5);
    lineItem->setPen(penLine);

    connect(chartView, SIGNAL(keyPressed(int)), this, SLOT(onKeyPressed(int)));
    connect(chartView, SIGNAL(resizeChart()), this, SLOT(onResizeChart()));

    if (!chartView->chart()->series().isEmpty()) {
        chartView->chart()->createDefaultAxes();
        chartView->chart()->axes(Qt::Horizontal).back()->setLabelsVisible(false);
        chartView->setFocus();
    }
}

/**
 * @brief VdViewer::onQuitAct_triggered
 */
void VdViewer::onQuitAct_triggered()
{
    close();
}

/**
 * @brief VdViewer::setCsvSettingsAct_triggered
 */
void VdViewer::setCsvSettingsAct_triggered()
{
    /* CSV file settings window */
    csvSettingsWindow = new QDialog(this);
    csvSettingsWindow->setWindowTitle(tr("Настройки лог-файла"));
    csvSettingsWindow->resize(100, 150);
    csvSettingsWindow->setModal(true);
    csvSettingsWindow->setWindowFlags(csvSettingsWindow->windowFlags()
                                      & (~Qt::WindowContextHelpButtonHint));

    csvSettingsWindow->setAttribute(Qt::WA_DeleteOnClose);

    QList<QLabel*> settingLabelList;
    settingSpinBoxList.clear();

    settingLabelList << new QLabel(tr("Индекс строки имён параметров:"));
    settingLabelList << new QLabel(tr("Количество строк имён параметров:"));

    QVBoxLayout *vLayout = new QVBoxLayout;
    int settingsPrmSize = settingLabelList.size();

    for (auto i = 0; i < settingsPrmSize; ++i) {
        settingSpinBoxList << new QSpinBox();
        vLayout->addWidget(settingLabelList.at(i));
        vLayout->addWidget(settingSpinBoxList.at(i));
        settingSpinBoxList.at(i)->setMinimum(0);
        settingSpinBoxList.at(i)->setEnabled(!defaultSettings);
    }
    settingSpinBoxList.at(0)->setToolTip(tr("Индексация с 0, пустые строки игнорируются"));

    defaultSettingsCheckBox = new QCheckBox(tr("Настройки по молчанию"));
    connect(defaultSettingsCheckBox, SIGNAL(clicked()),this, SLOT(onDefaultStateChanged()));
    defaultSettingsCheckBox->setChecked(defaultSettings);
    QPushButton *saveSettings = new QPushButton(tr(" Применить настройки "));
    connect(saveSettings, SIGNAL(clicked()), this, SLOT(onSaveSettingsClicked()));
    QStringList Separators;
    Separators << tr("Запятая") << tr("Точка с запятой") << tr("Табуляция");

    settingSpinBoxList.at(0)->setValue(prmNamesLine);
    settingSpinBoxList.at(1)->setValue(prmNamesCount);

    vLayout->addWidget(defaultSettingsCheckBox);
    vLayout->addWidget(saveSettings);
    csvSettingsWindow->setLayout(vLayout);
    csvSettingsWindow->show();
}

/**
 * @brief VdViewer::onSaveSettingsClicked
 */
void VdViewer::onSaveSettingsClicked()
{
    csvSettingsWindow->close();

    prmNamesLine = settingSpinBoxList.at(0)->value();
    prmNamesCount = settingSpinBoxList.at(1)->value();
}

/**
 * @brief VdViewer::onDefaultStateChanged
 */
void VdViewer::onDefaultStateChanged()
{
    defaultSettings = defaultSettingsCheckBox->isChecked();

    if (!defaultSettings) {
        for (auto i = 0; i < settingSpinBoxList.size(); ++i) {
            settingSpinBoxList.at(i)->setEnabled(true);
        }
    }
    else {
        setDefaultSettings();
        for (auto i = 0; i < settingSpinBoxList.size(); ++i) {
            settingSpinBoxList.at(i)->setEnabled(false);
        }
        settingSpinBoxList.at(0)->setValue(prmNamesLine);
        settingSpinBoxList.at(1)->setValue(prmNamesCount);
    }
}

/**
 * @brief VdViewer::setupLabel
 * @param label
 */
void VdViewer::setupLabel(QLabel *label)
{
    QFont font;
    font.setBold(true);

    label->setFrameShape(QFrame::Panel);
    label->setAlignment(Qt::AlignCenter);
    label->setLineWidth(2);
    label->setMinimumWidth(32);
    label->setFont(font);
}

/**
 * @brief VdViewer::fillPrmLabels
 */
void VdViewer::fillPrmLabels()
{
    for (auto i = 0; i < prmStrList.size(); ++i) {
        prmLabel.at(i)->setText(prmStrList.at(i).at(trackLinePosX + prmNamesCount));
    }
    if (prmVisible.size() == 2) {
        double a = prmStrList.at(prmVisible.at(0)).at(trackLinePosX + prmNamesCount).toDouble(),
               b = prmStrList.at(prmVisible.at(1)).at(trackLinePosX + prmNamesCount).toDouble();
        if (deltaLabel != nullptr) {
            deltaLabel->setText(tr("Разность: ") + QString::number(a - b));
        }
    }
}

/**
 * @brief VdViewer::showLegendAct_toggled
 * @param checked
 */
void VdViewer::showLegendAct_toggled(bool checked)
{
    if (chartView != nullptr) {
        chartView->chart()->legend()->setVisible(checked);
    }
}

/**
 * @brief VdViewer::manualAct_triggered
 */
void VdViewer::manualAct_triggered()
{
    QTextBrowser* helpBrowser = new QTextBrowser;

    QFont hbFont = helpBrowser->font();
    hbFont.setPointSizeF(10.0);
    helpBrowser->setFont(hbFont);
    helpBrowser->setWindowTitle(tr("Работа с программой"));
    helpBrowser->resize(760, 540);
    helpBrowser->setAttribute(Qt::WA_DeleteOnClose);
    helpBrowser->setSearchPaths(QStringList() << ":/");
    helpBrowser->setSource(QString("text/help.htm"));

    helpBrowser->show();
}

/**
 * @brief VdViewer::utf8Act_triggered
 */
void VdViewer::utf8Act_triggered()
{
    utf8Act->setChecked(true);
    ansiAct->setChecked(false);
}

/**
 * @brief VdViewer::ansiAct_triggered
 */
void VdViewer::ansiAct_triggered()
{
    utf8Act->setChecked(false);
    ansiAct->setChecked(true);
}

/**
 * @brief VdViewer::aboutAct_triggered
 */
void VdViewer::aboutAct_triggered()
{
    QDialog* aboutWindow = new QDialog(this);
    aboutWindow->setWindowTitle("О программе");
    aboutWindow->resize(180, 110);
    aboutWindow->setModal(true);
    aboutWindow->setWindowFlags((aboutWindow->windowFlags())
                                & (~Qt::WindowContextHelpButtonHint));

    QLabel* logoLabel = new QLabel;
    QPixmap logoPixmap(":/images/diag1.png");
    logoLabel->setPixmap(logoPixmap);
    aboutWindow->setAttribute(Qt::WA_DeleteOnClose);

    QLabel* textLabel = new QLabel;
    QVBoxLayout* aboutLayot = new QVBoxLayout;
    textLabel->setText(tr("<p>&copy; Программа распространяется безвозмездно, то есть даром.</p>"
                          "<h3>Версия 1.0.1</h3>"));

    QLabel* refLabel = new QLabel;
    refLabel->setText(tr("<a href=mailto:igorf777@gmail.com>Книга жалоб и предложений</a>"));
    refLabel->setOpenExternalLinks(true);

    aboutLayot->addWidget(logoLabel, 0, Qt::AlignCenter);
    aboutLayot->addWidget(textLabel, 0, Qt::AlignCenter);
    aboutLayot->addWidget(refLabel, 0, Qt::AlignCenter);
    aboutWindow->setLayout(aboutLayot);

    aboutWindow->show();
}

/**
 * @brief VdViewer::readSettings
 */
void VdViewer::readSettings()
{
    QSettings settings("vdviewer.ini", QSettings::IniFormat);

    settings.beginGroup("Settings");
    winWidth = settings.value("winWidth", 930).toInt();
    winHeight = settings.value("winHeight", 600).toInt();
    prmNamesLine = settings.value("prmNamesLine", 3).toInt();
    prmNamesCount = settings.value("prmNamesCount", 3).toInt();
    defaultSettings = settings.value("defaultSettings", true).toBool();
    isUtf8Encoding = settings.value("utf8Encoding", true).toBool();
    pathLogs = settings.value("pathLogs", "").toString();
    settings.endGroup();
}

/**
 * @brief VdViewer::writeSettings
 */
void VdViewer::writeSettings()
{
    QSettings settings("vdviewer.ini", QSettings::IniFormat);

    winWidth = this->width();
    winHeight = this->height();

    settings.beginGroup("Settings");
    settings.setValue("prmNamesLine", prmNamesLine);
    settings.setValue("winWidth", winWidth);
    settings.setValue("winHeight", winHeight);
    settings.setValue("prmNamesCount", prmNamesCount);
    settings.setValue("defaultSettings", defaultSettings);
    settings.setValue("utf8Encoding", utf8Act->isChecked());
    settings.setValue("pathLogs", pathLogs);
    settings.endGroup();
}
