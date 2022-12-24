#ifndef VDVIEWER_H
#define VDVIEWER_H

#include "chartview.h"

class VdViewer : public QMainWindow
{
    Q_OBJECT

public:
    VdViewer(QWidget *parent = nullptr);
    ~VdViewer();

private slots:
    void openAct_triggered();
    void closeAct_triggered();
    void onQuitAct_triggered();
    void showLegendAct_toggled(bool checked);
    void setCsvSettingsAct_triggered();
    void manualAct_triggered();
    void aboutAct_triggered();
    void utf8Act_triggered();
    void ansiAct_triggered();
    void onShowGraphClicked();
    void onSaveSettingsClicked();
    void onDefaultStateChanged();
    void onKeyPressed(int key);
    void onResizeChart();

private:
    QAction* showLegend;
    QAction* utf8Act;
    QAction* ansiAct;
    QTabWidget* mainTabWidget = nullptr;
    QDialog *prmSelectionWindow = nullptr;
    QDialog *csvSettingsWindow = nullptr;
    QList<QCheckBox*> prmSelector;
    QList<QLabel*> prmLabel;
    QList<QSpinBox*> settingSpinBoxList;
    ChartView *chartView = nullptr;
    QTableWidget *tableWidget = nullptr;
    QLabel *deltaLabel;
    QCheckBox *defaultSettingsCheckBox;
    QVBoxLayout *chartLayout;
    QGraphicsLineItem *lineItem;
    QList<int> prmVisible;
    QList<int> tsCollumnIndex; // timestamp column indexes
    QList<QStringList> prmStrList;
    QStringList prmNameList;
    QStringList TsStr;

    QString pathLogs;
    bool defaultSettings;
    int winWidth, winHeight;
    int prmRowCount = 0;
    int prmNamesLine;
    int prmNamesCount;
    int trackLinePosX = 0;
    bool isUtf8Encoding = true;
    QChar csvSep = ',';

    void setDefaultSettings();
    void setupLabel(QLabel *label);
    void fillPrmLabels();
    void readSettings();
    void writeSettings();
};
#endif // VDVIEWER_H
