#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "BodeCalculator.h"

class QLineEdit;
class QLabel;
class QDoubleSpinBox;
class QSpinBox;
class QPushButton;
class BodePlotWidget;

// Hauptfenster: Eingabe, Berechnung (core), Diagramm und Stabilitaetskennwerte.
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onCompute();
    void onExport();

private:
    void buildUi();
    void showError(const QString& message);
    void updateResultLabels(const BodeResult& result);

    QLineEdit* numeratorEdit_ = nullptr;
    QLineEdit* denominatorEdit_ = nullptr;
    QLabel* previewLabel_ = nullptr;
    QLabel* errorLabel_ = nullptr;

    QDoubleSpinBox* minExponentSpin_ = nullptr;
    QDoubleSpinBox* maxExponentSpin_ = nullptr;
    QSpinBox* pointsPerDecadeSpin_ = nullptr;

    QPushButton* computeButton_ = nullptr;
    QPushButton* exportButton_ = nullptr;

    QLabel* gainMarginLabel_ = nullptr;
    QLabel* phaseMarginLabel_ = nullptr;
    QLabel* stabilityLabel_ = nullptr;

    BodePlotWidget* plot_ = nullptr;
};

#endif
