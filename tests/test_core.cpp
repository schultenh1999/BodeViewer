// Modultests des Rechenkerns (TransferFunction, BodeCalculator).

#include <complex>
#include <stdexcept>
#include <string>
#include <vector>

#include "BodeCalculator.h"
#include "TestSupport.h"
#include "TransferFunction.h"


void testParseCoefficients() {
    printSection("TransferFunction: Koeffizienten parsen");

    std::vector<double> coefficients;

    bool ok = TransferFunction::parseCoefficients("1, 3, 2", coefficients);
    check(ok, "eine gueltige Liste wird angenommen");
    check(coefficients.size() == 3, "die Liste \"1, 3, 2\" ergibt drei Koeffizienten");
    check(coefficients[0] == 1.0 && coefficients[1] == 3.0 && coefficients[2] == 2.0,
          "die Koeffizienten stehen in der Eingabereihenfolge");

    ok = TransferFunction::parseCoefficients("  -0.5 , 7 ", coefficients);
    check(ok, "Leerzeichen, Minuszeichen und Dezimalpunkt werden angenommen");
    check(coefficients[0] == -0.5 && coefficients[1] == 7.0, "-0.5 und 7 werden richtig umgewandelt");

    check(!TransferFunction::parseCoefficients("", coefficients), "eine leere Eingabe wird abgelehnt");
    check(!TransferFunction::parseCoefficients("1, abc", coefficients), "Buchstaben werden abgelehnt");
    check(!TransferFunction::parseCoefficients("1,,2", coefficients), "ein leeres Feld wird abgelehnt");
}

void testValidate() {
    printSection("TransferFunction: Koeffizienten pruefen");

    std::vector<double> numerator;
    numerator.push_back(1.0);

    std::vector<double> denominator;
    denominator.push_back(1.0);
    denominator.push_back(1.0);

    std::vector<double> empty;

    std::vector<double> zeroDenominator;
    zeroDenominator.push_back(0.0);
    zeroDenominator.push_back(0.0);

    std::string errorMessage;
    check(TransferFunction::validate(numerator, denominator, errorMessage), "gueltige Polynome werden angenommen");
    check(!TransferFunction::validate(empty, denominator, errorMessage), "ein leerer Zaehler wird abgelehnt");
    check(!TransferFunction::validate(numerator, empty, errorMessage), "ein leerer Nenner wird abgelehnt");
    check(!TransferFunction::validate(numerator, zeroDenominator, errorMessage),
          "ein Nennerpolynom gleich Null wird abgelehnt");
    check(errorMessage != "", "im Fehlerfall wird eine Meldung gesetzt");
}

void testConstructor() {
    printSection("TransferFunction: Konstruktor");

    std::vector<double> numerator;
    numerator.push_back(1.0);

    std::vector<double> zeroDenominator;
    zeroDenominator.push_back(0.0);

    bool exceptionWasThrown = false;
    try {
        TransferFunction tf(numerator, zeroDenominator);
    } catch (std::invalid_argument& error) {
        exceptionWasThrown = true;
    }
    check(exceptionWasThrown, "der Konstruktor wirft bei ungueltigen Koeffizienten eine Ausnahme");
}

void testEvaluate() {
    printSection("TransferFunction: auswerten und formatieren");

    // G(j) = 1 / (1 + j) = 0.5 - 0.5j
    std::vector<double> numerator;
    numerator.push_back(1.0);

    std::vector<double> denominator;
    denominator.push_back(1.0);
    denominator.push_back(1.0);

    TransferFunction tf(numerator, denominator);
    std::complex<double> value = tf.evaluate(std::complex<double>(0.0, 1.0));
    checkNear(value.real(), 0.5, 0.000001, "Realteil von 1/(s+1) bei s = j");
    checkNear(value.imag(), -0.5, 0.000001, "Imaginaerteil von 1/(s+1) bei s = j");

    std::vector<double> exampleNumerator;
    exampleNumerator.push_back(1.0);
    exampleNumerator.push_back(5.0);

    std::vector<double> exampleDenominator;
    exampleDenominator.push_back(1.0);
    exampleDenominator.push_back(3.0);
    exampleDenominator.push_back(2.0);

    TransferFunction example(exampleNumerator, exampleDenominator);
    check(example.toString() == "(s + 5) / (s^2 + 3s + 2)",
          "toString liefert einen lesbaren Bruch (erhalten: " + example.toString() + ")");
}

// --- Modul core, Klasse BodeCalculator -----------------------------------

TransferFunction makeTransferFunction(double gain, std::vector<double> denominator) {
    std::vector<double> numerator;
    numerator.push_back(gain);
    return TransferFunction(numerator, denominator);
}

// (s + 1)^3
std::vector<double> makeThirdOrderDenominator() {
    std::vector<double> denominator;
    denominator.push_back(1.0);
    denominator.push_back(3.0);
    denominator.push_back(3.0);
    denominator.push_back(1.0);
    return denominator;
}

// s + 1
std::vector<double> makeFirstOrderDenominator() {
    std::vector<double> denominator;
    denominator.push_back(1.0);
    denominator.push_back(1.0);
    return denominator;
}

void testFrequencyGrid() {
    printSection("BodeCalculator: Frequenzraster");

    TransferFunction tf = makeTransferFunction(1.0, makeFirstOrderDenominator());
    BodeResult result = BodeCalculator::compute(tf, 1.0, 100.0, 10);

    check(result.omega.size() == 21, "2 Dekaden mit 10 Punkten pro Dekade ergeben 21 Stuetzstellen");
    check(result.magnitudeDb.size() == 21, "zu jeder Frequenz gibt es einen Amplitudenwert");
    check(result.phaseDeg.size() == 21, "zu jeder Frequenz gibt es einen Phasenwert");
    checkNear(result.omega[0], 1.0, 0.000001, "das Raster beginnt bei der Startfrequenz");
    checkNear(result.omega[20], 100.0, 0.000001, "das Raster endet bei der Endfrequenz");
}

void testInvalidRange() {
    printSection("BodeCalculator: ungueltiger Frequenzbereich");

    TransferFunction tf = makeTransferFunction(1.0, makeFirstOrderDenominator());

    bool exceptionWasThrown = false;
    try {
        BodeCalculator::compute(tf, 0.0, 100.0, 10);
    } catch (std::invalid_argument& error) {
        exceptionWasThrown = true;
    }
    check(exceptionWasThrown, "eine Startfrequenz von 0 wird abgelehnt");

    exceptionWasThrown = false;
    try {
        BodeCalculator::compute(tf, 100.0, 1.0, 10);
    } catch (std::invalid_argument& error) {
        exceptionWasThrown = true;
    }
    check(exceptionWasThrown, "eine Endfrequenz unterhalb der Startfrequenz wird abgelehnt");

    exceptionWasThrown = false;
    try {
        BodeCalculator::compute(tf, 1.0, 100.0, 0);
    } catch (std::invalid_argument& error) {
        exceptionWasThrown = true;
    }
    check(exceptionWasThrown, "0 Punkte pro Dekade werden abgelehnt");
}

// --- Zusammenspiel der beiden Klassen des Moduls core --------------------

void testFrequencyResponse() {
    printSection("Zusammenspiel: Frequenzgang eines bekannten Systems");

    // PT1: -3.01 dB / -45 Grad bei der Eckfrequenz, -40 dB zwei Dekaden darueber.
    TransferFunction tf = makeTransferFunction(1.0, makeFirstOrderDenominator());
    BodeResult result = BodeCalculator::compute(tf, 1.0, 100.0, 10);

    checkNear(result.magnitudeDb[0], -3.01, 0.01, "Amplitude von 1/(s+1) bei der Eckfrequenz");
    checkNear(result.phaseDeg[0], -45.0, 0.01, "Phase von 1/(s+1) bei der Eckfrequenz");
    checkNear(result.magnitudeDb[20], -40.0, 0.1, "Amplitude zwei Dekaden oberhalb der Eckfrequenz");
}

void testStableSystem() {
    printSection("Zusammenspiel: stabiles System");

    // -180 Grad bei omega = sqrt(3), dort |G| = 1/8 -> 20*log10(8) = 18.06 dB.
    TransferFunction tf = makeTransferFunction(1.0, makeThirdOrderDenominator());
    BodeResult result = BodeCalculator::compute(tf, 0.01, 100.0, 200);

    check(result.hasGainMargin, "eine Amplitudenreserve wird gefunden");
    checkNear(result.gainMarginOmega, 1.732, 0.05, "die Phase geht bei omega = sqrt(3) durch -180 Grad");
    checkNear(result.gainMarginDb, 18.06, 0.2, "die Amplitudenreserve betraegt rund 18 dB");
    check(result.stability == BodeResult::Stable, "das System wird als stabil bewertet");

    check(result.phaseDeg[result.phaseDeg.size() - 1] < -180.0,
          "die Phase laeuft nach dem Unwrapping unter -180 Grad hinaus");
}

void testUnstableSystem() {
    printSection("Zusammenspiel: instabiles System");

    TransferFunction tf = makeTransferFunction(1000.0, makeThirdOrderDenominator());
    BodeResult result = BodeCalculator::compute(tf, 0.01, 100.0, 200);

    check(result.hasGainMargin, "eine Amplitudenreserve wird gefunden");
    check(result.gainMarginDb < 0.0, "die Amplitudenreserve ist negativ");
    check(result.stability == BodeResult::Unstable, "das System wird als instabil bewertet");
}

void testSystemWithoutMargin() {
    printSection("Zusammenspiel: System ohne Amplitudenreserve");

    TransferFunction tf = makeTransferFunction(1.0, makeFirstOrderDenominator());
    BodeResult result = BodeCalculator::compute(tf, 0.01, 100.0, 50);

    check(!result.hasGainMargin, "das PT1-Glied hat keine endliche Amplitudenreserve");
    check(result.stability == BodeResult::Stable, "das PT1-Glied wird als stabil bewertet");
}

int main() {
    testParseCoefficients();
    testValidate();
    testConstructor();
    testEvaluate();
    testFrequencyGrid();
    testInvalidRange();
    testFrequencyResponse();
    testStableSystem();
    testUnstableSystem();
    testSystemWithoutMargin();
    return testSummary();
}
