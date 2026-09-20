#ifndef TRANSFERFUNCTION_H
#define TRANSFERFUNCTION_H

#include <complex>
#include <string>
#include <vector>

// Uebertragungsfunktion G(s) = N(s) / D(s).
// Koeffizienten in absteigender Potenz: {a_n, ..., a_1, a_0}.
class TransferFunction {
public:
    // Bei ungueltigen Koeffizienten: false und Meldung in errorMessage.
    static bool validate(const std::vector<double>& numerator,
                          const std::vector<double>& denominator,
                          std::string& errorMessage);

    // Wirft std::invalid_argument bei ungueltigen Koeffizienten.
    TransferFunction(const std::vector<double>& numerator, const std::vector<double>& denominator);

    std::complex<double> evaluate(std::complex<double> s) const;

    // z.B. "(s + 5) / (s^2 + 3s + 2)"
    std::string toString() const;

    // "1, 3, 2" -> {1, 3, 2}; false bei ungueltigem Format.
    static bool parseCoefficients(const std::string& text, std::vector<double>& result);

private:
    static std::complex<double> evaluatePolynomial(const std::vector<double>& coeffs, std::complex<double> s);
    static std::string formatPolynomial(const std::vector<double>& coeffs);

    std::vector<double> numerator_;
    std::vector<double> denominator_;
};

#endif
