#include "BodeCalculator.h"

#include <cmath>
#include <stdexcept>

const double kPi = 3.14159265358979323846;

// Erste Kreuzung von searchCurve mit target (linear interpoliert, log. Frequenz).
// Liefert Kreuzungsfrequenz und den dort interpolierten Wert von otherCurve.
bool findLinearCrossing(const std::vector<double>& omega,
                         const std::vector<double>& searchCurve,
                         const std::vector<double>& otherCurve,
                         double target,
                         double& omegaOut,
                         double& otherOut) {
    for (size_t i = 1; i < searchCurve.size(); ++i) {
        const double y0 = searchCurve[i - 1] - target;
        const double y1 = searchCurve[i] - target;

        if (y0 == 0.0) {
            omegaOut = omega[i - 1];
            otherOut = otherCurve[i - 1];
            return true;
        }

        const bool crossesDown = (y0 > 0.0 && y1 <= 0.0);
        const bool crossesUp = (y0 < 0.0 && y1 >= 0.0);
        if (!crossesDown && !crossesUp) continue;

        const double t = y0 / (y0 - y1);
        const double logOmega0 = std::log10(omega[i - 1]);
        const double logOmega1 = std::log10(omega[i]);
        omegaOut = std::pow(10.0, logOmega0 + t * (logOmega1 - logOmega0));
        otherOut = otherCurve[i - 1] + t * (otherCurve[i] - otherCurve[i - 1]);
        return true;
    }
    return false;
}

void BodeCalculator::unwrapPhaseDeg(std::vector<double>& phaseDeg) {
    for (size_t i = 1; i < phaseDeg.size(); ++i) {
        double diff = phaseDeg[i] - phaseDeg[i - 1];
        while (diff > 180.0) {
            phaseDeg[i] -= 360.0;
            diff -= 360.0;
        }
        while (diff < -180.0) {
            phaseDeg[i] += 360.0;
            diff += 360.0;
        }
    }
}

void BodeCalculator::computeMargins(BodeResult& result) {
    // Phase bei -180 Grad -> Amplitudenreserve.
    double gainMarginOmega = 0.0;
    double gainMarginOther = 0.0;
    if (findLinearCrossing(result.omega, result.phaseDeg, result.magnitudeDb, -180.0, gainMarginOmega, gainMarginOther)) {
        result.hasGainMargin = true;
        result.gainMarginOmega = gainMarginOmega;
        result.gainMarginDb = -gainMarginOther;
    }

    // Betrag bei 0 dB -> Phasenreserve.
    double phaseMarginOmega = 0.0;
    double phaseMarginOther = 0.0;
    if (findLinearCrossing(result.omega, result.magnitudeDb, result.phaseDeg, 0.0, phaseMarginOmega, phaseMarginOther)) {
        result.hasPhaseMargin = true;
        result.phaseMarginOmega = phaseMarginOmega;
        result.phaseMarginDeg = 180.0 + phaseMarginOther;
    }

    const bool gainOk = !result.hasGainMargin || result.gainMarginDb > 0.0;
    const bool phaseOk = !result.hasPhaseMargin || result.phaseMarginDeg > 0.0;

    if (!result.hasGainMargin && !result.hasPhaseMargin) {
        result.stability = BodeResult::Stable;
    } else if (gainOk && phaseOk) {
        result.stability = BodeResult::Stable;
    } else {
        result.stability = BodeResult::Unstable;
    }
}

BodeResult BodeCalculator::compute(const TransferFunction& tf,
                                    double omegaMin,
                                    double omegaMax,
                                    int pointsPerDecade) {
    if (omegaMin <= 0.0 || omegaMax <= omegaMin || pointsPerDecade < 1) {
        throw std::invalid_argument("Ungueltiger Frequenzbereich fuer die Bode-Berechnung.");
    }

    const double logMin = std::log10(omegaMin);
    const double logMax = std::log10(omegaMax);
    const double decades = logMax - logMin;
    int totalPoints = static_cast<int>(std::round(decades * pointsPerDecade)) + 1;
    if (totalPoints < 2) {
        totalPoints = 2;
    }

    BodeResult result;
    result.omega.reserve(totalPoints);
    result.magnitudeDb.reserve(totalPoints);
    result.phaseDeg.reserve(totalPoints);

    for (int i = 0; i < totalPoints; ++i) {
        const double t = static_cast<double>(i) / (totalPoints - 1);
        const double omega = std::pow(10.0, logMin + t * decades);

        const std::complex<double> g = tf.evaluate(std::complex<double>(0.0, omega));
        const double magnitude = std::abs(g);

        result.omega.push_back(omega);
        result.magnitudeDb.push_back(20.0 * std::log10(magnitude));
        result.phaseDeg.push_back(std::arg(g) * 180.0 / kPi);
    }

    unwrapPhaseDeg(result.phaseDeg);
    computeMargins(result);
    return result;
}
