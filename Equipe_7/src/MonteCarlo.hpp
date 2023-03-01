#pragma once
#include <iostream>
#include <vector>
#include "Option.hpp"
#include "BlackScholesModel.hpp"
#include "pnl/pnl_random.h"
#include "Position.hpp"


class MonteCarlo
{
  public:
    BlackScholesModel* mod_; /*! pointeur vers le modèle */
    Option* opt_;            /*! pointeur sur l'option */
    PnlRng* rng_;            /*! pointeur sur le générateur */
    double fdStep_;          /*! pas de différence finie */
    long nbSamples_;         /*! nombre de tirages Monte Carlo */



    /**
     Constructeur à cinq arguments d'un objet Monte carlo
    */

    MonteCarlo(BlackScholesModel* mod, Option* opt, PnlRng* rng, double fdStep, long nbSamples);

    /**
     * Calcule le prix de l'option à la date 0
     *
     * @param[out] prix valeur de l'estimateur Monte Carlo
     * @param[out] ic écart type de l'estimateur
     */
    void price(double& prix, double& std_dev);

    /**
     * Calcule le prix de l'option à la date t
     *
     * @param[in]  past contient la trajectoire du sous-jacent
     * jusqu'à l'instant t
     * @param[in] t date à laquelle le calcul est fait
     * @param[out] prix contient le prix
     * @param[out] std_dev contient l'écart type de l'estimateur
     */
    void price(const PnlMat* past, double t, double& prix, double& std_dev);

    /**
     * Calcule le delta de l'option à la date t
     *
     * @param[in] past contient la trajectoire du sous-jacent
     * jusqu'à l'instant t
     * @param[in] t date à laquelle le calcul est fait
     * @param[out] delta contient le vecteur de delta
     * @param[out] std_dev contient l'écart type de l'estimateur
     */
    void delta(const PnlMat* past, double t, PnlVect* delta, PnlVect* std_dev);

    /**
     * Calcule le delta de l'option à la date 0
     *
     * @param[in] t date à laquelle le calcul est fait
     * @param[out] delta contient le vecteur de delta
     * @param[out] std_dev contient l'écart type de l'estimateur
     */
    void delta(PnlVect* delta, PnlVect* std_dev);

    /**
     Vérifie les instants de la mise à jour de la composition deltas par rapport au nbTimeSteps
    */
    void makeReguralizedPast(PnlMat * past, PnlVect * shares, int i, double H);


    
    /**
      Génére le portefeuilile de couverture et calcule son erreur de couverture
     */
    vector<Position> portfolioValues(PnlMat * marketData, double &error);
    
    // Destructeur de la classe Monte Carlo
    ~MonteCarlo();






};
