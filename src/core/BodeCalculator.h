#ifndef BODECALCULATOR_H
#define BODECALCULATOR_H

#include <vector>

#include "TransferFunction.h"

// Frequenzgang und Stabilitaetsspannen. G gilt als offene Kette mit
// Einheitsrueckfuehrung; instabile offene Pole (RHP) werden nicht geprueft.
struct BodeResult {
    std::vector<double> omega;        // rad/s, logarithmisch verteilt
    std::vector<double> magnitudeDb;  // 20*log10(|G(j*omega)|)
    std::vector<double> phaseDeg;     // unwrapped, in Grad

    bool hasGainMargin = false;
    double gainMarginDb = 0.0;     // gueltig nur wenn hasGainMargin
    double gainMarginOmega = 0.0;  // Phasendurchgangsfrequenz (-180 Grad)

    bool hasPhaseMargin = false;
    double phaseMarginDeg = 0.0;     // gueltig nur wenn hasPhaseMargin
    double phaseMarginOmega = 0.0;   // Amplitudendurchgangsfrequenz (0 dB)

    enum Stability { Stable, Unstable, Indeterminate };
    Stability stability = Indeterminate;
};

class BodeCalculator {
public:
    // Bereich [omegaMin, omegaMax] in rad/s, logarithmisches Raster.
    // Vorbedingung: omegaMin > 0, omegaMax > omegaMin, pointsPerDecade >= 1.
    static BodeResult compute(const TransferFunction& tf,
                               double omegaMin,
                               double omegaMax,
                               int pointsPerDecade);

private:
    static void unwrapPhaseDeg(std::vector<double>& phaseDeg);
    static void computeMargins(BodeResult& result);
};

#endif
