#include "MainWindow.h"

#include <QDir>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QSplitter>
#include <QVBoxLayout>
#include <QWidget>
#include <cmath>

#include <string>
#include <vector>

#include "BodePlotWidget.h"
#include "TransferFunction.h"

const double kTwoPi = 2.0 * 3.14159265358979323846;

QString formatFrequency(double omega) {
    const double value = omega / kTwoPi;
    return QString::number(value, 'g', 4) + " Hz";
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    buildUi();
    setWindowTitle("Bode-Viewer");
    resize(1200, 750);
    onCompute();
}

void MainWindow::buildUi() {
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(splitter);

    QWidget* controlPanel = new QWidget(splitter);
    QVBoxLayout* controlLayout = new QVBoxLayout(controlPanel);

    QGroupBox* tfGroup = new QGroupBox("Übertragungsfunktion G(s) = Zähler / Nenner", controlPanel);
    QFormLayout* tfLayout = new QFormLayout(tfGroup);

    numeratorEdit_ = new QLineEdit("1, 5", tfGroup);
    numeratorEdit_->setObjectName("numeratorEdit");
    numeratorEdit_->setPlaceholderText("z. B. 1, 5  (für 1s + 5)");
    denominatorEdit_ = new QLineEdit("1, 3, 2", tfGroup);
    denominatorEdit_->setObjectName("denominatorEdit");
    denominatorEdit_->setPlaceholderText("z. B. 1, 3, 2  (für s² + 3s + 2)");
    tfLayout->addRow("Zähler-Koeffizienten:", numeratorEdit_);
    tfLayout->addRow("Nenner-Koeffizienten:", denominatorEdit_);

    previewLabel_ = new QLabel(tfGroup);
    previewLabel_->setObjectName("previewLabel");
    previewLabel_->setWordWrap(true);
    previewLabel_->setStyleSheet("font-style: italic; color: #444;");
    tfLayout->addRow("Vorschau:", previewLabel_);

    errorLabel_ = new QLabel(tfGroup);
    errorLabel_->setObjectName("errorLabel");
    errorLabel_->setWordWrap(true);
    errorLabel_->setStyleSheet("color: #c0392b;");
    errorLabel_->hide();
    tfLayout->addRow(errorLabel_);

    controlLayout->addWidget(tfGroup);

    QGroupBox* rangeGroup = new QGroupBox("Frequenzbereich", controlPanel);
    QFormLayout* rangeLayout = new QFormLayout(rangeGroup);

    // Punkt als Dezimaltrennzeichen, unabhaengig vom Systemgebiet.
    const QLocale decimalPointLocale = QLocale::c();

    minExponentSpin_ = new QDoubleSpinBox(rangeGroup);
    minExponentSpin_->setObjectName("minExponentSpin");
    minExponentSpin_->setLocale(decimalPointLocale);
    minExponentSpin_->setRange(-9, 8);
    minExponentSpin_->setValue(-2);
    minExponentSpin_->setSuffix(" (10^x Hz)");

    maxExponentSpin_ = new QDoubleSpinBox(rangeGroup);
    maxExponentSpin_->setObjectName("maxExponentSpin");
    maxExponentSpin_->setLocale(decimalPointLocale);
    maxExponentSpin_->setRange(-8, 9);
    maxExponentSpin_->setValue(3);
    maxExponentSpin_->setSuffix(" (10^x Hz)");

    pointsPerDecadeSpin_ = new QSpinBox(rangeGroup);
    pointsPerDecadeSpin_->setObjectName("pointsPerDecadeSpin");
    pointsPerDecadeSpin_->setRange(5, 500);
    pointsPerDecadeSpin_->setValue(100);

    rangeLayout->addRow("Startfrequenz:", minExponentSpin_);
    rangeLayout->addRow("Endfrequenz:", maxExponentSpin_);
    rangeLayout->addRow("Punkte pro Dekade:", pointsPerDecadeSpin_);

    controlLayout->addWidget(rangeGroup);

    computeButton_ = new QPushButton("Berechnen", controlPanel);
    computeButton_->setObjectName("computeButton");
    controlLayout->addWidget(computeButton_);

    QGroupBox* resultGroup = new QGroupBox("Stabilitätsanalyse", controlPanel);
    QVBoxLayout* resultLayout = new QVBoxLayout(resultGroup);
    gainMarginLabel_ = new QLabel(resultGroup);
    gainMarginLabel_->setObjectName("gainMarginLabel");
    phaseMarginLabel_ = new QLabel(resultGroup);
    phaseMarginLabel_->setObjectName("phaseMarginLabel");
    stabilityLabel_ = new QLabel(resultGroup);
    stabilityLabel_->setObjectName("stabilityLabel");
    gainMarginLabel_->setWordWrap(true);
    phaseMarginLabel_->setWordWrap(true);
    resultLayout->addWidget(gainMarginLabel_);
    resultLayout->addWidget(phaseMarginLabel_);
    resultLayout->addWidget(stabilityLabel_);
    controlLayout->addWidget(resultGroup);

    exportButton_ = new QPushButton("Diagramm exportieren…", controlPanel);
    exportButton_->setObjectName("exportButton");
    controlLayout->addWidget(exportButton_);

    controlLayout->addStretch(1);
    controlPanel->setMaximumWidth(380);

    plot_ = new BodePlotWidget(splitter);
    plot_->setObjectName("bodePlot");

    splitter->addWidget(controlPanel);
    splitter->addWidget(plot_);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);

    connect(computeButton_, &QPushButton::clicked, this, &MainWindow::onCompute);
    connect(exportButton_, &QPushButton::clicked, this, &MainWindow::onExport);
}

void MainWindow::showError(const QString& message) {
    errorLabel_->setText(message);
    errorLabel_->show();
}

void MainWindow::onCompute() {
    errorLabel_->hide();

    std::vector<double> numerator;
    std::vector<double> denominator;
    const bool numeratorOk = TransferFunction::parseCoefficients(numeratorEdit_->text().toStdString(), numerator);
    const bool denominatorOk = TransferFunction::parseCoefficients(denominatorEdit_->text().toStdString(), denominator);

    if (!numeratorOk || !denominatorOk) {
        showError("Bitte gültige, kommagetrennte Koeffizienten eingeben (z. B. \"1, 3, 2\").");
        return;
    }

    std::string validationError;
    if (!TransferFunction::validate(numerator, denominator, validationError)) {
        showError(QString::fromStdString(validationError));
        return;
    }

    const TransferFunction tf(numerator, denominator);
    previewLabel_->setText(QString::fromStdString(tf.toString()));

    const double omegaMin = std::pow(10.0, minExponentSpin_->value()) * kTwoPi;
    const double omegaMax = std::pow(10.0, maxExponentSpin_->value()) * kTwoPi;
    if (omegaMax <= omegaMin) {
        showError("Die Endfrequenz muss größer als die Startfrequenz sein.");
        return;
    }

    BodeResult result;
    try {
        result = BodeCalculator::compute(tf, omegaMin, omegaMax, pointsPerDecadeSpin_->value());
    } catch (const std::exception& e) {
        showError(QString::fromStdString(e.what()));
        return;
    }

    plot_->setResult(result);
    updateResultLabels(result);
    exportButton_->setEnabled(true);
}

void MainWindow::updateResultLabels(const BodeResult& result) {
    if (result.hasGainMargin) {
        gainMarginLabel_->setText(QString("Amplitudenreserve: %1 dB bei %2")
                                       .arg(QString::number(result.gainMarginDb, 'f', 2))
                                       .arg(formatFrequency(result.gainMarginOmega)));
    } else {
        gainMarginLabel_->setText("Amplitudenreserve: unendlich (keine -180°-Durchquerung)");
    }

    if (result.hasPhaseMargin) {
        phaseMarginLabel_->setText(QString("Phasenreserve: %1° bei %2")
                                        .arg(QString::number(result.phaseMarginDeg, 'f', 2))
                                        .arg(formatFrequency(result.phaseMarginOmega)));
    } else {
        phaseMarginLabel_->setText("Phasenreserve: unendlich (keine 0dB-Durchquerung)");
    }

    switch (result.stability) {
        case BodeResult::Stable:
            stabilityLabel_->setText("Bewertung: stabil");
            stabilityLabel_->setStyleSheet("color: #1a7f37; font-weight: bold;");
            break;
        case BodeResult::Unstable:
            stabilityLabel_->setText("Bewertung: instabil");
            stabilityLabel_->setStyleSheet("color: #c0392b; font-weight: bold;");
            break;
        case BodeResult::Indeterminate:
            stabilityLabel_->setText("Bewertung: unbestimmt");
            stabilityLabel_->setStyleSheet("color: #888888; font-weight: bold;");
            break;
    }
}

void MainWindow::onExport() {
    const QString fileName = QFileDialog::getSaveFileName(
        this, "Bode-Diagramm exportieren", QDir::homePath() + "/bode.png",
        "PNG-Bild (*.png);;JPEG-Bild (*.jpg *.jpeg);;PDF-Dokument (*.pdf)");
    if (fileName.isEmpty()) return;

    const QString suffix = QFileInfo(fileName).suffix().toLower();
    bool success = false;
    if (suffix == "jpg" || suffix == "jpeg") {
        success = plot_->exportJpg(fileName);
    } else if (suffix == "pdf") {
        success = plot_->exportPdf(fileName);
    } else {
        const QString target = suffix == "png" ? fileName : fileName + ".png";
        success = plot_->exportPng(target);
    }

    if (!success) {
        QMessageBox::warning(this, "Export fehlgeschlagen", "Die Datei konnte nicht gespeichert werden.");
    }
}
