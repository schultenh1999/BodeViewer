#ifndef BODEPLOTWIDGET_H
#define BODEPLOTWIDGET_H

#include <QPen>
#include <QWidget>

#include "BodeCalculator.h"

class QCustomPlot;
class QCPAxisRect;
class QCPGraph;

// Bode-Diagramm mit Stabilitaetsmarkierungen und Export (PNG/JPG/PDF).
class BodePlotWidget : public QWidget {
    Q_OBJECT
public:
    explicit BodePlotWidget(QWidget* parent = nullptr);

    // Frequenzachse in Hz (f = omega / 2*pi).
    void setResult(const BodeResult& result);

    bool exportPng(const QString& fileName) const;
    bool exportJpg(const QString& fileName) const;
    bool exportPdf(const QString& fileName) const;

private:
    void addHorizontalReference(QCPAxisRect* rect, double y, const QPen& pen) const;
    void addVerticalMarker(QCPAxisRect* rect, double x, const QPen& pen) const;

    QCustomPlot* plot_;
    QCPAxisRect* magnitudeRect_;
    QCPAxisRect* phaseRect_;
    QCPGraph* magnitudeGraph_;
    QCPGraph* phaseGraph_;

    static const int kExportWidth = 1920;
    static const int kExportHeight = 1080;
};

#endif
