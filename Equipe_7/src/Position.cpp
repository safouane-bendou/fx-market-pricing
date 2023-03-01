#include <iostream>
#include "Position.hpp"


Position::Position(int date, double price, double priceStdDev, PnlVect* deltas, PnlVect* deltasStdDev, double portfolioValue)
    : date(date), price(price), priceStdDev(priceStdDev), portfolioValue(portfolioValue), deltas(deltas), deltasStdDev(deltasStdDev) {

}

void to_json(nlohmann::json &j, const Position &position) {
    j["date"] = position.date;
    j["value"] = position.portfolioValue;
    j["price"] = position.price;
    j["priceStdDev"] = position.priceStdDev;
    j["deltas"] = position.deltas;
    j["deltasStdDev"] = position.deltasStdDev;
}

void Position::print() const {
    nlohmann::json j = *this;
    std::cout << j.dump(4);
}
