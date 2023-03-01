#include "ForeignAsian.hpp"

ForeignAsian::ForeignAsian(PnlVect* dates_)
{
    dates = dates_;
}
double ForeignAsian::payoff(const PnlMat* path)
{
    PnlVect* spots = pnl_vect_create(path->m);
    pnl_mat_get_col(spots, path, 1);
    double payoff = (1 / (double)path->m) * pnl_vect_sum(spots) - pnl_mat_get(path, path->m -1, 0);
    if (payoff > 0) {
        return payoff;
    } else {
        return 0;
    }
}
ForeignAsian::~ForeignAsian() {
    //pnl_vect_free(dates);
}