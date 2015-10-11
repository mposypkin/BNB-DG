#include <stdlib.h>
#include <limits>
#include <fstream>

#include <util/poly/polynom.hpp>
#include <util/poly/polynomtxt.hpp>
#include <util/poly/polyutil.hpp>
#include <util/common/fileutils.hpp>
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
    int n;
    // Cut analysis depth
    int ldepth = 1;
    // Use or not boxed cut or boxed cut together with normal cut
    int boxedcut = 1;

    if (argc != 3)
        BNB_ERROR_REPORT("Usage bnbdg.exe inputf outputf");
    
 
    /* Reading problem description and state */
    NlpProblem<double> nlp;
    WFSDFSManager manager;    
    std::string str;
    FileUtils::getStringFromFile(argv[1], str);
    bnbdg::SolverData sd; 
    bnbdg::ParseInp::parseSolverData(str, sd);
    bnbdg::ParseInp::parseNLP(str, nlp);
    manager.setOptions(sd.mTreeTraverseStrategy);
    n = nlp.mBox.mDim;
    std::cout << "n = " << n << "\n";
    double x[n];
    UnconsRecStore<double> ors(std::numeric_limits<double>::max(), n);
    BNCState<double> state(&manager, &ors);
    bnbdg::ParseInp::parseState(str, nlp, state);


    /* Setup cut generators */
    /* Cut generator for objective*/
    PolyObjective<double>* obj = dynamic_cast<PolyObjective<double>*> (nlp.mObj);
    PolyEigenSupp objEigenSupp(obj);

    /* Setup eigen based cut factory*/
    EigenCutFactory<double> objEigenCutFact(&ors, &objEigenSupp, obj, sd.mEps);

    /* Setup specialized unconstrained cut factory*/
    UnconsCutFactory<double> unconsCutFact(&ors, &objEigenSupp, obj, &(nlp.mBox), sd.mEps);
    //UnconsCutFactory<double> unconsCutFact(&ors, &objEigenSupp, obj, NULL, sd.mEps);

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

    long long iters = sd.mNSteps;
    bnc.solve(iters, state, ru);

    /* Printing results*/
    std::cout << iters << " iterations\n";
    std::cout << "Record = " << state.mRecord->getValue() << "\n";
    VecUtils::vecPrint(n, (double*) ors.getX());

    /* Saving state */
    std::ofstream ofs;
    ofs.open(argv[2]);
    if (ofs.is_open())
        SaveBNCState::saveState(n, state, ofs);
    else
        BNB_ERROR_REPORT("Failed to open output file\n");
    ofs.close();
    return 0;
}