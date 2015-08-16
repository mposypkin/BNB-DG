#include <stdlib.h>
#include <limits>
#include <fstream>

#include <problems/poly/polynom.hpp>
#include <problems/poly/polynomtxt.hpp>
#include <problems/poly/polyutil.hpp>
#include <problems/optlib/polyobjective.hpp>
#include <problems/nlp/cuts/nlprecstore.hpp>
#include <problems/nlp/cuts/unconsrecstore.hpp>
#include <problems/nlp/poly/polysupp.hpp>
#include <problems/nlp/cuts/eigencutfactory.hpp>
#include <problems/nlp/cuts/unconscutfactory.hpp>
#include <problems/nlp/cuts/convcutfactory.hpp>
#include <problems/nlp/cuts/compcutfactory.hpp>
#include <problems/nlp/cuts/smartcutapplicator.hpp>
#include <util/tree/wfsdfsmanager.hpp>
#include <problems/nlp/bnc/bncsolver.hpp>
#include <problems/nlp/bnc/stdboxsplitter.hpp>
#include <problems/optlib/gradboxdesc.hpp>


#include "parseinp.hpp"
#include "savebncstate.hpp"

/**
 * Termination condition for local search
 */
class MyStopper : public GradBoxDescent<double>::Stopper {
public:

    bool stopnow(double xdiff, double fdiff, double gnorm, double fval, int n) {
        return fdiff <= 0.0000001;
    }
};

/**
 * Local search method
 */
class GradLocSearch : public LocSearch<double> {
public:

    GradLocSearch(Box<double>& box, Objective<double>* obj) {
        mLS = new GradBoxDescent<double>(box, new MyStopper);
        mLS->setObjective(obj);
    }

    double search(double* x) {
        double v;
        mLS->search(x, &v);
        return v;
    }

private:
    GradBoxDescent<double>* mLS;

};

/*
 * 
 */
int main(int argc, char** argv) {
    // Dimension
    int n = 2;
    // Box size
    int d = 3;
    // Accuracy 
    double eps = .001;
    // Cut analysis depth
    int ldepth = 1;
    // Use or not boxed cut or boxed cut together with normal cut
    int boxedcut = true;


    if (argc != 7)
        BNB_ERROR_REPORT("Usage bnbdg.exe dimension box_size polynom nsteps inputf outputf");
    n = atoi(argv[1]);
    d = atoi(argv[2]);
    long long int iters = atoi(argv[4]);

    double x[n];
    double rec = 0;
    UnconsRecStore<double> ors(std::numeric_limits<double>::max(), n);

    /* Reading problem description and state */
    NlpProblem<double> nlp;
    WFSDFSManager manager;
    manager.setOptions(WFSDFSManager::Options::DFS);
    BNCState<double> state(&manager, &ors);
    std::string str;
    ParseInp::getStringFromFile(argv[5], str);
    ParseInp::parse(str, nlp, state);

    /* Setup cut generators */
    /* Cut generator for objective*/
    PolyObjective<double>* obj = dynamic_cast<PolyObjective<double>*> (nlp.mObj);
    PolyEigenSupp objEigenSupp(obj);

    /* Setup eigen based cut factory*/
    EigenCutFactory<double> objEigenCutFact(&ors, &objEigenSupp, obj, eps);

    /* Setup specialized unconstrained cut factory*/
    UnconsCutFactory<double> unconsCutFact(&ors, &objEigenSupp, obj, &(nlp.mBox), eps);
    //UnconsCutFactory<double> unconsCutFact(&ors, &objEigenSupp, obj, NULL, eps);

    /** Setup Convexity cut factory */
    GradLocSearch gls(nlp.mBox, obj);
    ConvCutFactory<double> convCutFact(&ors, &objEigenSupp, obj, &gls);

    /* Setup composite cut factory  */
    CompCutFactory <double> fact;
    fact.push(&objEigenCutFact);
    //fact.push(&unconsCutFact);
    //fact.push(&convCutFact);

    /* Setup cut applicator */
    SmartCutApplicator<double> sca(nlp.mVariables);
    if (boxedcut == 0)
        sca.getOptions() = SmartCutApplicator<double>::Options::CUT_BALL_SIMPLE;
    if (boxedcut == 1)
        sca.getOptions() = SmartCutApplicator<double>::Options::CUT_BALL_BOXED;
    if (boxedcut == 2)
        sca.getOptions() = (SmartCutApplicator<double>::Options::CUT_BALL_SIMPLE
            | SmartCutApplicator<double>::Options::CUT_BALL_BOXED);

    /* Setup splitter */
    StdBoxSplitter<double> splt;

    /* Setup solver */
    BNCSolver<double> bnc(&fact, &sca, &splt, ldepth);


    /* Solving problem */
    bool ru;

    bnc.solve(iters, state, ru);

    /* Printing results*/
    std::cout << iters << " iterations\n";
    std::cout << "Record = " << state.mRecord->getValue() << "\n";
    VecUtils::vecPrint(n, (double*) ors.getX());

    /* Saving state */
    std::ofstream ofs;
    ofs.open(argv[6]);
    if (ofs.is_open())
        SaveBNCState::saveState(n, state, ofs);
    else
        BNB_ERROR_REPORT("Failed to open output file\n");
    ofs.close();
    return 0;
}