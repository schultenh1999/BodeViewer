#include "BodePlotWidget.h"

#include <QVBoxLayout>
#include <cmath>

#include "qcustomplot.h"

const double kTwoPi = 2.0 * 3.14159265358979323846;

QVector<double> toQVector(const std::vector<double>& values) {
    QVector<double> result;
    result.reserve(static_cast<int>(values.size()));
    for (double v : values) result.append(v);
    return result;
}

void configureLogAxis(QCPAxis* axis) {
    axis->setScaleType(QCPAxis::stLogarithmic);
    QSharedPointer<QCPAxisTickerLog> ticker(new QCPAxisTickerLog);
    axis->setTicker(ticker);
    axis->grid()->setSubGridVisible(true);
}

BodePlotWidget::BodePlotWidget(QWidget* parent) : QWidget(parent) {
    plot_ = new QCustomPlot(this);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(plot_);
    setLayout(layout);

    plot_->plotLayout()->clear();
    plot_->plotLayout()->insertRow(0);
    plot_->plotLayout()->addElement(0, 0, new QCPTextElement(plot_, "Bode-Diagramm", 13));

    magnitudeRect_ = new QCPAxisRect(plot_);
    phaseRect_ = new QCPAxisRect(plot_);
    plot_->plotLayout()->addElement(1, 0, magnitudeRect_);
    plot_->plotLayout()->addElement(2, 0, phaseRect_);
    plot_->plotLayout()->setRowStretchFactor(1, 3);
    plot_->plotLayout()->setRowStretchFactor(2, 2);

    configureLogAxis(magnitudeRect_->axis(QCPAxis::atBottom));
    configureLogAxis(phaseRect_->axis(QCPAxis::atBottom));
    magnitudeRect_->axis(QCPAxis::atBottom)->setTickLabels(false);

    magnitudeGraph_ = plot_->addGraph(magnitudeRect_->axis(QCPAxis::atBottom),
                                       magnitudeRect_->axis(QCPAxis::atLeft));
    magnitudeGraph_->setPen(QPen(QColor(30, 90, 200), 2));

    phaseGraph_ = plot_->addGraph(phaseRect_->axis(QCPAxis::atBottom), phaseRect_->axis(QCPAxis::atLeft));
    phaseGraph_->setPen(QPen(QColor(200, 60, 40), 2));

    magnitudeRect_->axis(QCPAxis::atLeft)->setLabel("Amplitude (dB)");
    phaseRect_->axis(QCPAxis::atLeft)->setLabel("Phase (°)");
}

void BodePlotWidget::addHorizontalReference(QCPAxisRect* rect, double y, const QPen& pen) const {
    QCPItemStraightLine* line = new QCPItemStraightLine(plot_);
    line->point1->setAxes(rect->axis(QCPAxis::atBottom), rect->axis(QCPAxis::atLeft));
    line->point2->setAxes(rect->axis(QCPAxis::atBottom), rect->axis(QCPAxis::atLeft));
    const QCPRange range = rect->axis(QCPAxis::atBottom)->range();
    line->point1->setCoords(range.lower, y);
    line->point2->setCoords(range.upper, y);
    line->setClipAxisRect(rect);
    line->setPen(pen);
}

void BodePlotWidget::addVerticalMarker(QCPAxisRect* rect, double x, const QPen& pen) const {
    QCPItemStraightLine* line = new QCPItemStraightLine(plot_);
    line->point1->setAxes(rect->axis(QCPAxis::atBottom), rect->axis(QCPAxis::atLeft));
    line->point2->setAxes(rect->axis(QCPAxis::atBottom), rect->axis(QCPAxis::atLeft));
    line->point1->setCoords(x, 0.0);
    line->point2->setCoords(x, 1.0);
    line->setClipAxisRect(rect);
    line->setPen(pen);
}

void BodePlotWidget::setResult(const BodeResult& result) {
    if (result.omega.empty()) return;

    QVector<double> freq = toQVector(result.omega);
    for (double& f : freq) f /= kTwoPi;
    const QVector<double> magnitude = toQVector(result.magnitudeDb);
    const QVector<double> phase = toQVector(result.phaseDeg);

    magnitudeGraph_->setData(freq, magnitude, true);
    phaseGraph_->setData(freq, phase, true);

    plot_->clearItems();

    const QCPRange freqRange(freq.first(), freq.last());
    magnitudeRect_->axis(QCPAxis::atBottom)->setRange(freqRange);
    phaseRect_->axis(QCPAxis::atBottom)->setRange(freqRange);
    phaseRect_->axis(QCPAxis::atBottom)->setLabel("f (Hz)");

    magnitudeRect_->axis(QCPAxis::atLeft)->rescale();
    magnitudeRect_->axis(QCPAxis::atLeft)->scaleRange(1.15);
    phaseRect_->axis(QCPAxis::atLeft)->rescale();
    phaseRect_->axis(QCPAxis::atLeft)->scaleRange(1.15);

    const QPen referencePen(QColor(150, 150, 150), 1, Qt::DashLine);
    addHorizontalReference(magnitudeRect_, 0.0, referencePen);
    addHorizontalReference(phaseRect_, -180.0, referencePen);

    if (result.hasPhaseMargin) {
        const double x = result.phaseMarginOmega / kTwoPi;
        const QPen pen(QColor(40, 160, 70), 1, Qt::DashLine);
        addVerticalMarker(magnitudeRect_, x, pen);
        addVerticalMarker(phaseRect_, x, pen);
    }
    if (result.hasGainMargin) {
        const double x = result.gainMarginOmega / kTwoPi;
        const QPen pen(QColor(210, 90, 20), 1, Qt::DashLine);
        addVerticalMarker(magnitudeRect_, x, pen);
        addVerticalMarker(phaseRect_, x, pen);
    }

    plot_->replot();
}

bool BodePlotWidget::exportPng(const QString& fileName) const {
    return plot_->savePng(fileName, kExportWidth, kExportHeight);
}

bool BodePlotWidget::exportJpg(const QString& fileName) const {
    return plot_->saveJpg(fileName, kExportWidth, kExportHeight);
}

bool BodePlotWidget::exportPdf(const QString& fileName) const {
    return plot_->savePdf(fileName, kExportWidth, kExportHeight);
}
