#pragma once
#include <iostream>
#include <vector>
#include <cmath>
#include "pnl/pnl_random.h"
#include "pnl/pnl_vector.h"
#include "pnl/pnl_matrix.h"

#include "Currency.hpp"
#include "Asset.hpp"



using namespace std;

/// \brief Modèle de Black Scholes
class BlackScholesModel
{
  public:
    vector<Asset *> assets_;
    vector<Currency *> currencies_;
    int size;
    PnlVect * sigma_;
    PnlVect* spot_; 
    PnlVect* trend_;
    PnlVect* gaussianVector;
    PnlVect* currentShares;
    PnlVect* nextShares;


    /**
     Constructeur à quatre paramètres définissant le modèle BlackScholes 
     */

    BlackScholesModel(vector<Asset *> assets, vector<Currency *> currencies, PnlVect* spot);


    

    /**
     * Génère une trajectoire du modèle et la stocke dans path
     *
     * @param[out] path contient une trajectoire du modèle.
     * C'est une matrice de taille (nbTimeSteps+1) x d
     * @param[in] dates contient les dates de constatation
     */
    void asset(PnlMat* path, PnlVect * dates, PnlRng* rng);

    /**
     * Calcule une trajectoire du modèle connaissant le
     * passé jusqu' à la date t
     *
     * @param[out] path  contient une trajectoire du sous-jacent
     * donnée jusqu'à l'instant t par la matrice past
     * @param[in] t date jusqu'à laquelle on connait la trajectoire.
     * t n'est pas forcément une date de discrétisation
     * @param[in] dates contient les dates de constatation
     * @param[in] past trajectoire réalisée jusqu'a la date t
     */
    void asset(PnlMat* path, double t, PnlVect * dates, PnlRng* rng, const PnlMat* past);

    /**
     * Shift d'une trajectoire du sous-jacent
     *
     * @param[in]  path contient en input la trajectoire
     * du sous-jacent
     * @param[out] shift_path contient la trajectoire path
     * dont la composante d a été shiftée par (1+h)
     * à partir de la date t.
     * @param[in] t date à partir de laquelle on shift
     * @param[in] h pas de différences finies
     * @param[in] d indice du sous-jacent à shifter
     * @param[in] timestep pas de constatation du sous-jacent
     */
    void shiftAsset(PnlMat* shift_path, const PnlMat* path, int d, double h, double t, PnlVect * dates);


    //Destructeur de la classe
    ~BlackScholesModel();
};
