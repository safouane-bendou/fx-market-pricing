#include "Currency.hpp"	

Currency::Currency(double drift_, PnlVect * volatilityVectorPnlVect_, double interestRate, double domesticInterestRate)
{
    drift = drift_;
    volatilityVector = volatilityVectorPnlVect_;
    domesticInterestRate_ = domesticInterestRate;
    interestRate_ = interestRate;
}

Currency::~Currency() {
}