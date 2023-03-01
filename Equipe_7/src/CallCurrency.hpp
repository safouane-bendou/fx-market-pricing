#pragma once
#include "Option.hpp"

class CallCurrency : public Option
{
  public:
    double strike;
    double foreignRate;
    CallCurrency(PnlVect* dates_, double strike, double foreignRate);
    double payoff(const PnlMat* path);
    ~CallCurrency();
};