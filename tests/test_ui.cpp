// Tests der Oberflaeche: BodePlotWidget, Zusammenspiel mit dem Rechenkern
// und simulierte Benutzereingaben (Qt Test, Plattform "offscreen").

#include <QApplication>
#include <QDoubleSpinBox>
#include <QFile>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTemporaryDir>
#include <QTest>

#include <vector>

#include "BodeCalculator.h"
#include "BodePlotWidget.h"
#include "MainWindow.h"
#include "TestSupport.h"
#include "TransferFunction.h"

QByteArray readFile(QString path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return QByteArray();
    }
    return file.readAll();
}

// --- Modul ui: das BodePlotWidget alleine --------------------------------

void testPlotWidgetExport() {
    printSection("BodePlotWidget: Ergebnis anzeigen und exportieren");

    // G(s) = 1/(s+1)^3
    std::vector<double> numerator;
    numerator.push_back(1.0);

    std::vector<double> denominator;
    denominator.push_back(1.0);
    denominator.push_back(3.0);
    denominator.push_back(3.0);
    denominator.push_back(1.0);

    TransferFunction tf(numerator, denominator);
    BodeResult result = BodeCalculator::compute(tf, 0.1, 100.0, 50);

    BodePlotWidget widget;
    widget.resize(800, 600);
    widget.setResult(result);

    QTemporaryDir directory;
    check(directory.isValid(), "ein temporaeres Verzeichnis steht bereit");
    if (!directory.isValid()) {
        return;
    }

    QString pngPath = directory.filePath("diagramm.png");
    QString pdfPath = directory.filePath("diagramm.pdf");

    check(widget.exportPng(pngPath), "der PNG-Export meldet Erfolg");
    check(readFile(pngPath).size() > 0, "die PNG-Datei wurde geschrieben und ist nicht leer");

    check(widget.exportPdf(pdfPath), "der PDF-Export meldet Erfolg");
    check(readFile(pdfPath).size() > 0, "die PDF-Datei wurde geschrieben und ist nicht leer");
}

// --- Gesamtstruktur: Zustand nach dem Start ------------------------------

void testStartupState(MainWindow& window) {
    printSection("MainWindow: Zustand nach dem Start");

    QLabel* preview = window.findChild<QLabel*>("previewLabel");
    QLabel* stability = window.findChild<QLabel*>("stabilityLabel");
    QLabel* error = window.findChild<QLabel*>("errorLabel");

    check(preview != nullptr && stability != nullptr && error != nullptr, "die Anzeigefelder wurden gefunden");
    if (preview == nullptr || stability == nullptr || error == nullptr) {
        return;
    }

    check(preview->text() == "(s + 5) / (s^2 + 3s + 2)", "die Vorschau zeigt das voreingestellte Beispiel");
    check(stability->text().startsWith("Bewertung:"), "die Stabilitaetsbewertung ist ausgefuellt");
    check(!error->isVisible(), "es wird keine Fehlermeldung angezeigt");
}

// --- Benutzerinteraktion -------------------------------------------------

void testUserComputesStableSystem(MainWindow& window) {
    printSection("Benutzerinteraktion: stabiles System berechnen");

    QLineEdit* numerator = window.findChild<QLineEdit*>("numeratorEdit");
    QLineEdit* denominator = window.findChild<QLineEdit*>("denominatorEdit");
    QPushButton* computeButton = window.findChild<QPushButton*>("computeButton");
    QLabel* preview = window.findChild<QLabel*>("previewLabel");
    QLabel* gainMargin = window.findChild<QLabel*>("gainMarginLabel");
    QLabel* stability = window.findChild<QLabel*>("stabilityLabel");

    check(numerator != nullptr && denominator != nullptr && computeButton != nullptr,
          "die Eingabefelder und der Rechnen-Knopf wurden gefunden");
    if (numerator == nullptr || denominator == nullptr || computeButton == nullptr) {
        return;
    }

    // G(s) = 1/(s+1)^3: erwartet 18.06 dB Amplitudenreserve, stabil.
    numerator->clear();
    QTest::keyClicks(numerator, "1");
    denominator->clear();
    QTest::keyClicks(denominator, "1, 3, 3, 1");
    QTest::mouseClick(computeButton, Qt::LeftButton);

    check(preview->text() == "(1) / (s^3 + 3s^2 + 3s + 1)",
          "die Vorschau uebernimmt die eingetippten Koeffizienten");
    check(gainMargin->text().contains("18.0"), "die berechnete Amplitudenreserve steht im Anzeigefeld");
    check(stability->text() == "Bewertung: stabil", "das System wird als stabil angezeigt");
}

void testUserComputesUnstableSystem(MainWindow& window) {
    printSection("Benutzerinteraktion: instabiles System berechnen");

    QLineEdit* numerator = window.findChild<QLineEdit*>("numeratorEdit");
    QPushButton* computeButton = window.findChild<QPushButton*>("computeButton");
    QLabel* stability = window.findChild<QLabel*>("stabilityLabel");
    if (numerator == nullptr || computeButton == nullptr || stability == nullptr) {
        check(false, "die Bedienelemente wurden gefunden");
        return;
    }

    numerator->clear();
    QTest::keyClicks(numerator, "1000");
    QTest::mouseClick(computeButton, Qt::LeftButton);

    check(stability->text() == "Bewertung: instabil", "das System wird als instabil angezeigt");
}

void testUserEntersLetters(MainWindow& window) {
    printSection("Benutzerinteraktion: Buchstaben statt Zahlen");

    QLineEdit* numerator = window.findChild<QLineEdit*>("numeratorEdit");
    QPushButton* computeButton = window.findChild<QPushButton*>("computeButton");
    QLabel* preview = window.findChild<QLabel*>("previewLabel");
    QLabel* error = window.findChild<QLabel*>("errorLabel");
    if (numerator == nullptr || computeButton == nullptr || preview == nullptr || error == nullptr) {
        check(false, "die Bedienelemente wurden gefunden");
        return;
    }

    QString previewBefore = preview->text();

    numerator->clear();
    QTest::keyClicks(numerator, "abc");
    QTest::mouseClick(computeButton, Qt::LeftButton);

    check(error->isVisible(), "es wird eine Fehlermeldung angezeigt");
    check(error->text() != "", "die Fehlermeldung hat einen Text");
    check(preview->text() == previewBefore, "die Vorschau bleibt nach dem Fehler unveraendert stehen");
}

void testUserEntersZeroDenominator(MainWindow& window) {
    printSection("Benutzerinteraktion: Nennerpolynom gleich Null");

    QLineEdit* numerator = window.findChild<QLineEdit*>("numeratorEdit");
    QLineEdit* denominator = window.findChild<QLineEdit*>("denominatorEdit");
    QPushButton* computeButton = window.findChild<QPushButton*>("computeButton");
    QLabel* error = window.findChild<QLabel*>("errorLabel");
    if (numerator == nullptr || denominator == nullptr || computeButton == nullptr || error == nullptr) {
        check(false, "die Bedienelemente wurden gefunden");
        return;
    }

    numerator->clear();
    QTest::keyClicks(numerator, "1");
    denominator->clear();
    QTest::keyClicks(denominator, "0, 0");
    QTest::mouseClick(computeButton, Qt::LeftButton);

    check(error->isVisible(), "es wird eine Fehlermeldung angezeigt");
    check(error->text().contains("Nennerpolynom"), "die Meldung aus der Pruefung im Rechenkern wird angezeigt");
}

void testUserEntersWrongFrequencyRange(MainWindow& window) {
    printSection("Benutzerinteraktion: falscher Frequenzbereich");

    QLineEdit* denominator = window.findChild<QLineEdit*>("denominatorEdit");
    QPushButton* computeButton = window.findChild<QPushButton*>("computeButton");
    QLabel* error = window.findChild<QLabel*>("errorLabel");
    QDoubleSpinBox* minExponent = window.findChild<QDoubleSpinBox*>("minExponentSpin");
    QDoubleSpinBox* maxExponent = window.findChild<QDoubleSpinBox*>("maxExponentSpin");
    if (denominator == nullptr || computeButton == nullptr || error == nullptr) {
        check(false, "die Bedienelemente wurden gefunden");
        return;
    }
    if (minExponent == nullptr || maxExponent == nullptr) {
        check(false, "die Eingabefelder fuer den Frequenzbereich wurden gefunden");
        return;
    }

    denominator->clear();
    QTest::keyClicks(denominator, "1, 1");

    minExponent->setValue(3.0);
    maxExponent->setValue(1.0);
    QTest::mouseClick(computeButton, Qt::LeftButton);
    check(error->isVisible(), "eine Endfrequenz unterhalb der Startfrequenz wird gemeldet");
    check(error->text().contains("Endfrequenz"), "die Meldung nennt die Endfrequenz");

    minExponent->setValue(-2.0);
    maxExponent->setValue(3.0);
    QTest::mouseClick(computeButton, Qt::LeftButton);
    check(!error->isVisible(), "nach der Korrektur verschwindet die Fehlermeldung");
}

// --- Zusammenspiel: das Ergebnis erreicht das Diagramm -------------------

void testResultReachesPlot(MainWindow& window) {
    printSection("Zusammenspiel: das Ergebnis erreicht das Diagramm");

    QLineEdit* denominator = window.findChild<QLineEdit*>("denominatorEdit");
    QPushButton* computeButton = window.findChild<QPushButton*>("computeButton");
    BodePlotWidget* plot = window.findChild<BodePlotWidget*>("bodePlot");
    if (denominator == nullptr || computeButton == nullptr || plot == nullptr) {
        check(false, "die Bedienelemente und das Diagramm wurden gefunden");
        return;
    }

    QTemporaryDir directory;
    check(directory.isValid(), "ein temporaeres Verzeichnis steht bereit");
    if (!directory.isValid()) {
        return;
    }

    QString firstPath = directory.filePath("erstes.png");
    QString secondPath = directory.filePath("zweites.png");

    denominator->clear();
    QTest::keyClicks(denominator, "1, 1");
    QTest::mouseClick(computeButton, Qt::LeftButton);
    check(plot->exportPng(firstPath), "das Diagramm des ersten Systems wird exportiert");

    denominator->clear();
    QTest::keyClicks(denominator, "1, 3, 3, 1");
    QTest::mouseClick(computeButton, Qt::LeftButton);
    check(plot->exportPng(secondPath), "das Diagramm des zweiten Systems wird exportiert");

    QByteArray firstImage = readFile(firstPath);
    QByteArray secondImage = readFile(secondPath);
    check(firstImage.size() > 0 && secondImage.size() > 0, "beide Bilddateien sind nicht leer");

    check(firstImage != secondImage, "das Diagramm aendert sich mit der berechneten Uebertragungsfunktion");
}

int main(int argc, char* argv[]) {
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", "offscreen");
    }
    QApplication app(argc, argv);

    testPlotWidgetExport();

    MainWindow window;
    window.show();

    testStartupState(window);
    testUserComputesStableSystem(window);
    testUserComputesUnstableSystem(window);
    testUserEntersLetters(window);
    testUserEntersZeroDenominator(window);
    testUserEntersWrongFrequencyRange(window);
    testResultReachesPlot(window);

    return testSummary();
}
