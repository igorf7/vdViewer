#include "chartview.h"
#include  <QChartView>

ChartView::ChartView(QWidget *parent):QChartView(parent)
{
    this->setDragMode(QGraphicsView::RubberBandDrag);
    this->setMouseTracking(false);
}

ChartView::~ChartView()
{
}

/**
 * @brief Mouse press event
 * @param event
 */
void ChartView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        beginPoint=event->pos();
    }
    QChartView::mousePressEvent(event);
}

/**
 * @brief Mouse release event
 * @param event
 */
void ChartView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button()==Qt::LeftButton)
    {
        endPoint = event->pos();
        QRectF  rectF;
        rectF.setTopLeft(this->beginPoint);
        rectF.setBottomRight(this->endPoint);
        this->chart()->zoomIn(rectF);
    }
    else if (event->button() == Qt::RightButton) {
             this->chart()->zoomReset();
    }
    QChartView::mouseReleaseEvent(event);
}

/**
 * @brief Keyboard event
 * @param event
 */
void ChartView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_Up:
        break;
    case Qt::Key_Down:
        break;
    case Qt::Key_Left:
        emit keyPressed(0);
        break;
    case Qt::Key_Right:
        emit keyPressed(1);
        break;
    case Qt::Key_Home:
        emit keyPressed(2);
        break;
    case Qt::Key_End:
        emit keyPressed(3);
        break;
    case Qt::Key_PageUp:
        break;
    case Qt::Key_PageDown:
        break;
    case Qt::Key_W:
        chart()->scroll(0, -10);
        break;
    case Qt::Key_A:
        chart()->scroll(10, 0);
        break;
    case Qt::Key_S:
        chart()->scroll(0, 10);
        break;
    case Qt::Key_D:
        chart()->scroll(-10, 0);
        break;
    default:
        break;
    }
    QChartView::keyPressEvent(event);
}

/**
 * @brief Resize event
 * @param event
 */
void ChartView::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)

    emit resizeChart();

    QChartView::resizeEvent(event);
}
