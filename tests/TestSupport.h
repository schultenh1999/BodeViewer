#ifndef TESTSUPPORT_H
#define TESTSUPPORT_H

#include <string>

// Minimales Testgeruest fuer beide Testprogramme.

void printSection(std::string title);

void check(bool condition, std::string name);

void checkNear(double actual, double expected, double tolerance, std::string name);

// Rueckgabewert fuer main(): 0 wenn alles bestanden, sonst 1.
int testSummary();

#endif
