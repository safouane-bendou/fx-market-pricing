#include "CallCurrency.hpp"

CallCurrency::CallCurrency(PnlVect* dates_, double strike_, double foreignRate_)
{
    dates = dates_;
    strike = strike_;
    foreignRate = foreignRate_;
}

double  CallCurrency::payoff(const PnlMat* path) {
    double payoff = pnl_mat_get(path, path->m - 1, 0) * exp(foreignRate*pnl_vect_get(dates, dates->size -1)) - strike;
    if (payoff > 0) {
        return payoff;
    } else {
        return 0;
    }   

}
CallCurrency::~CallCurrency() {
    //pnl_vect_free(dates);
}