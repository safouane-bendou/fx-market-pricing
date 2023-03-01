#include "Asset.hpp"	

Asset::Asset(double drift_, PnlVect * volatilityVectorPnlVect_, double domesticInterestRate)
{
    drift = drift_;
    volatilityVector = volatilityVectorPnlVect_;
    domesticInterestRate_ = domesticInterestRate;
}

Asset::~Asset() {
}