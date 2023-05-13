#include <QApplication>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QWheelEvent>

class CustomRectItem : public QGraphicsRectItem {
public:
    CustomRectItem(qreal x, qreal y, qreal w, qreal h, QGraphicsItem* parent = nullptr)
        : QGraphicsRectItem(x, y, w, h, parent) {}

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override {
        if (event->button() == Qt::LeftButton) {
            this->scene()->removeItem(
                this->scene()->property("highlight").value<QGraphicsRectItem*>()
            );
            delete this->scene()->property("highlight").value<QGraphicsRectItem*>();

            QGraphicsRectItem* highlight =
                this->scene()->addRect(this->rect(), QPen(Qt::blue, 1), Qt::NoBrush);
            highlight->setPos(this->pos());
            highlight->setZValue(1);  // Ensure highlight is always on top
            this->scene()->setProperty("highlight", QVariant::fromValue(highlight));
        }
        QGraphicsRectItem::mousePressEvent(event);
    }
};

class GameView : public QGraphicsView {
    qreal   _totalScaleFactor = 1;
    qreal   _lastScaleFactor  = 1;
    bool    _mousePressed     = false;
    QPointF _lastMousePos;

public:
    GameView() {
        setRenderHint(QPainter::Antialiasing);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        setAttribute(Qt::WA_AcceptTouchEvents);
    }

protected:
    void wheelEvent(QWheelEvent* event) override {
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        double scaleFactor = 1.15;
        if (event->angleDelta().y() > 0) {
            // Zoom in
            scale(scaleFactor, scaleFactor);
        } else {
            // Zooming out
            scale(1 / scaleFactor, 1 / scaleFactor);
        }
    }

    void mousePressEvent(QMouseEvent* event) override {
        if (event->button() == Qt::RightButton) {
            _mousePressed = true;
            _lastMousePos = event->globalPosition();
        } else {
            QGraphicsView::mousePressEvent(event);
        }
    }

    void mouseMoveEvent(QMouseEvent* event) override {
        if (_mousePressed) {
            QPointF delta    = event->globalPosition() - _lastMousePos;
            _lastMousePos    = event->globalPosition();
            QScrollBar* hBar = horizontalScrollBar();
            QScrollBar* vBar = verticalScrollBar();
            hBar->setValue(hBar->value() - delta.x());
            vBar->setValue(vBar->value() - delta.y());
        }
    }

    void mouseReleaseEvent(QMouseEvent* event) override {
        if (event->button() == Qt::RightButton) {
            _mousePressed = false;
        }
    }

    bool viewportEvent(QEvent* event) override {
        switch (event->type()) {
            case QEvent::TouchBegin:
            case QEvent::TouchUpdate:
            case QEvent::TouchEnd: {
                QTouchEvent* touchEvent = static_cast<QTouchEvent*>(event);

                QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->points();
                if (touchPoints.count() == 2) {
                    // Determine scale factor
                    const QTouchEvent::TouchPoint& touchPoint0 = touchPoints.first();
                    const QTouchEvent::TouchPoint& touchPoint1 = touchPoints.last();

                    qreal currentScaleFactor =
                        QLineF(touchPoint0.position(), touchPoint1.position()).length() /
                        QLineF(touchPoint0.pressPosition(), touchPoint1.pressPosition()).length();
                    if (touchEvent->touchPointStates() & Qt::TouchPointReleased) {
                        // If one of the fingers is released, remember the last scale factor so that
                        // adding another finger later will continue zooming by adding new scale
                        // factor to the existing remembered value.
                        _lastScaleFactor = _totalScaleFactor;
                        _totalScaleFactor *= currentScaleFactor;
                    } else {
                        setTransformationAnchor(QGraphicsView::NoAnchor);
                        setTransform(QTransform::fromScale(
                            _totalScaleFactor * currentScaleFactor,
                            _totalScaleFactor * currentScaleFactor
                        ));
                    }
                } else if (touchPoints.count() == 1) {
                    // Pan
                    if (touchEvent->touchPointStates() & Qt::TouchPointMoved) {
                        QScrollBar* hBar = horizontalScrollBar();
                        QScrollBar* vBar = verticalScrollBar();
                        QPointF     delta =
                            touchPoints.first().position() - touchPoints.first().pressPosition();

                        qreal scaleFactor = transform().m11();

                        // Change this value to adjust the panning speed
                        qreal speedAdjustment = 0.1;

                        hBar->setValue(
                            hBar->value() + speedAdjustment * (delta.x() > 0 ? -1 : 1) *
                                                qAbs(delta.x()) / scaleFactor
                        );
                        vBar->setValue(
                            vBar->value() + speedAdjustment * (delta.y() > 0 ? -1 : 1) *
                                                qAbs(delta.y()) / scaleFactor
                        );
                    }
                }
                return true;
            }
            default:
                return QGraphicsView::viewportEvent(event);
        }
    }
};