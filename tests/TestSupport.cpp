#include "TestSupport.h"

#include <cmath>
#include <iostream>

static int failureCount = 0;

void printSection(std::string title) {
    std::cout << "\n--- " << title << " ---\n";
}

void check(bool condition, std::string name) {
    if (condition) {
        std::cout << "[ ok ] " << name << "\n";
    } else {
        std::cout << "[FAIL] " << name << "\n";
        failureCount = failureCount + 1;
    }
}

void checkNear(double actual, double expected, double tolerance, std::string name) {
    double difference = actual - expected;
    if (difference < 0.0) {
        difference = -difference;
    }

    if (difference <= tolerance) {
        std::cout << "[ ok ] " << name << "\n";
    } else {
        std::cout << "[FAIL] " << name << " (erwartet " << expected << ", erhalten " << actual << ")\n";
        failureCount = failureCount + 1;
    }
}

int testSummary() {
    std::cout << "\n";
    if (failureCount == 0) {
        std::cout << "Alle Tests bestanden.\n";
        return 0;
    }
    std::cout << failureCount << " Test(s) fehlgeschlagen.\n";
    return 1;
}
