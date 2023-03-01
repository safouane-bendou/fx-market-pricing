#include "QuantoExchange.hpp"

QuantoExchange::QuantoExchange(PnlVect* dates_, double strike_)
{
    dates = dates_;
    strike = strike_;
}
double QuantoExchange::payoff(const PnlMat* path)
{

    double mypayoff = pnl_mat_get(path, path->m - 1, 0) - pnl_mat_get(path, path->m - 1, 1) - strike;
    if (mypayoff > 0) {
        return mypayoff;
    } else {
        return 0;
    }
}
QuantoExchange::~QuantoExchange() {
    //pnl_vect_free(dates);
}