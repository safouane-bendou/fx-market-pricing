#pragma once
#include "Option.hpp"

class QuantoExchange : public Option
{
  public:
    double strike;
    
    QuantoExchange(PnlVect* dates_, double strike_);
    double payoff(const PnlMat* path);
    ~QuantoExchange();
};