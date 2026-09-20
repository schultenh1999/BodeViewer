#include "TransferFunction.h"

#include <cmath>
#include <locale>
#include <sstream>
#include <stdexcept>

bool isAllZero(const std::vector<double>& coeffs) {
    for (size_t i = 0; i < coeffs.size(); ++i) {
        if (coeffs[i] != 0.0) {
            return false;
        }
    }
    return true;
}

std::string trim(const std::string& s) {
    const char* whitespace = " \t\n\r\f\v";
    const size_t begin = s.find_first_not_of(whitespace);
    if (begin == std::string::npos) return "";
    const size_t end = s.find_last_not_of(whitespace);
    return s.substr(begin, end - begin + 1);
}

bool TransferFunction::validate(const std::vector<double>& numerator,
                                 const std::vector<double>& denominator,
                                 std::string& errorMessage) {
    if (numerator.empty()) {
        errorMessage = "Das Zaehlerpolynom benoetigt mindestens einen Koeffizienten.";
        return false;
    }
    if (denominator.empty()) {
        errorMessage = "Das Nennerpolynom benoetigt mindestens einen Koeffizienten.";
        return false;
    }
    if (isAllZero(denominator)) {
        errorMessage = "Das Nennerpolynom darf nicht identisch Null sein (Division durch 0).";
        return false;
    }
    errorMessage = "";
    return true;
}

TransferFunction::TransferFunction(const std::vector<double>& numerator, const std::vector<double>& denominator)
    : numerator_(numerator), denominator_(denominator) {
    std::string errorMessage;
    if (!validate(numerator_, denominator_, errorMessage)) {
        throw std::invalid_argument(errorMessage);
    }
}

std::complex<double> TransferFunction::evaluatePolynomial(const std::vector<double>& coeffs, std::complex<double> s) {
    std::complex<double> result(0.0, 0.0);
    for (size_t i = 0; i < coeffs.size(); ++i) {
        result = result * s + coeffs[i];
    }
    return result;
}

std::complex<double> TransferFunction::evaluate(std::complex<double> s) const {
    std::complex<double> numeratorValue = evaluatePolynomial(numerator_, s);
    std::complex<double> denominatorValue = evaluatePolynomial(denominator_, s);
    return numeratorValue / denominatorValue;
}

std::string TransferFunction::formatPolynomial(const std::vector<double>& coeffs) {
    const int degree = static_cast<int>(coeffs.size()) - 1;
    std::ostringstream out;
    bool wroteTerm = false;

    for (size_t i = 0; i < coeffs.size(); ++i) {
        const double c = coeffs[i];
        if (c == 0.0) continue;

        const int power = degree - static_cast<int>(i);
        const double magnitude = std::abs(c);

        if (wroteTerm) {
            if (c < 0) {
                out << " - ";
            } else {
                out << " + ";
            }
        } else if (c < 0) {
            out << "-";
        }

        if (power == 0 || magnitude != 1.0) {
            out << magnitude;
        }
        if (power >= 1) {
            out << "s";
            if (power > 1) out << "^" << power;
        }
        wroteTerm = true;
    }

    if (!wroteTerm) {
        return "0";
    }
    return out.str();
}

std::string TransferFunction::toString() const {
    return "(" + formatPolynomial(numerator_) + ") / (" + formatPolynomial(denominator_) + ")";
}

bool TransferFunction::parseCoefficients(const std::string& text, std::vector<double>& result) {
    const std::string trimmed = trim(text);
    if (trimmed.empty()) {
        return false;
    }

    std::vector<double> coefficients;
    std::stringstream stream(trimmed);
    std::string token;

    while (std::getline(stream, token, ',')) {
        const std::string value = trim(token);
        if (value.empty()) {
            return false;
        }

        // Classic-Locale: Punkt als Dezimaltrennzeichen.
        double parsed = 0.0;
        std::istringstream numberStream(value);
        numberStream.imbue(std::locale::classic());
        numberStream >> parsed;
        if (numberStream.fail() || !numberStream.eof()) {
            return false;
        }

        coefficients.push_back(parsed);
    }

    if (coefficients.empty()) {
        return false;
    }
    result = coefficients;
    return true;
}
