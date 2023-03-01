#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include "pnl/pnl_vector.h"
#include "pnl/pnl_matrix.h"
#include "json_reader.hpp"


class Position {
public:
    int date;
    PnlVect *deltas;
    PnlVect *deltasStdDev;
    double price;
    double priceStdDev;
    double portfolioValue;

    Position(int date, double price, double priceStdDev, PnlVect* deltas, PnlVect* deltasStdDev, double portfolioValue);
    friend void to_json(nlohmann::json &j, const Position &positions);
    void print() const;
};
