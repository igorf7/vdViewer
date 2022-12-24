#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include <QtCharts>
#include <QLegendMarker>

QT_CHARTS_USE_NAMESPACE
 
class ChartView : public QChartView
{
    Q_OBJECT
 
private:
    QPoint beginPoint;  // начальная точка прямоугольной области
    QPoint endPoint;    // конечная точка прямоугольной области
 
protected:
    void mousePressEvent(QMouseEvent *event);   // ЛКМ/ПКМ нажатие
    void mouseReleaseEvent(QMouseEvent *event); // ЛКМ/ПКМ отпускание
    void keyPressEvent(QKeyEvent *event);       // событие клавиатуры
    void resizeEvent(QResizeEvent *event);      // событие изм. размера
 
public:
    explicit ChartView(QWidget *parent = 0);
    ~ChartView();
 
signals:
    void keyPressed(int key);
    void resizeChart();
};
 
#endif // CHARTVIEW_H
