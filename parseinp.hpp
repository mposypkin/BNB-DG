/* 
 * File:   parseinp.hpp
 * Author: mposypkin
 *
 * Utilities for parsing the json input for BNB-DG
 * 
 * Created on August 11, 2015, 12:21 PM
 */

#ifndef PARSEINP_HPP
#define	PARSEINP_HPP

#include <string>
#include <fstream>
#include <sstream>
#include <limits>
#include <libjson/libjson.h>
#include <util/common/bnberrcheck.hpp>
#include <problems/nlp/bnc/bncstate.hpp>
#include <util/tree/wfsdfsmanager.hpp>
#include <util/poly/polynom.hpp>
#include <util/poly/polynomtxt.hpp>
#include <util/poly/polyutil.hpp>
#include <problems/optlib/polyobjective.hpp>


#include "bnbdgformat.hpp"
#include "soldata.hpp"

namespace bnbdg {

    class ParseInp {
    public:

        /**
         * Reading a string from a file
         * @param fname file name 
         * @param json resulting string
         */
        static void getStringFromFile(const char* fname, std::string& json) {
            std::ifstream is(fname);
            if (is.is_open()) {
                while (!is.eof()) {
                    std::string tmp;
                    is >> tmp;
                    json += tmp;
                }
                is.close();
            } else {
                BNB_ERROR_REPORT("Unable to open the input data file\n");
            }
        }

        static void parseSolverData(const std::string& input, SolverData& data) {
            JSONNode nd = libjson::parse(input);
            bool solverset = false;
            for (auto i = nd.begin(); i != nd.end(); i++) {
                if (i->name() == SOLVER_NAME) {
                    processSolver(*i, data);
                    solverset = true;
                    break;
                }
            }
            if(!solverset)
                BNB_ERROR_REPORT("Solver data missing\n");
        }

        /**
         * Parse problem definition from a string
         * @param input text state and problem description
         * @param prob NLP problem definition
         */
        static void parseNLP(const std::string& input, NlpProblem<double>& prob) {
            JSONNode nd = libjson::parse(input);
            bool problemset = false;
            for (auto i = nd.begin(); i != nd.end(); i++) {
                if (i->name() == PROBLEM_NAME) {
                    processProblem(*i, prob);
                    problemset = true;
                    break;
                }
            }
            if(!problemset)
                BNB_ERROR_REPORT("Problem data missing\n");
        }

        /**
         * Parse solver state from a string
         * @param input text state and problem description
         * @param n number of parameters
         * @param state solver state
         */
        static void parseState(const std::string& input, const NlpProblem<double>& prob, BNCState<double>& state) {
            JSONNode nd = libjson::parse(input);
            bool stateset = false;
            for (auto i = nd.begin(); i != nd.end(); i++) {
                if (i->name() == STATE_NAME) {
                    processState(*i, state, prob);
                    stateset = true;
                    break;
                }
            }
            if (!stateset) {
                makeDefaultSubs(nd, state, prob);
            }
        }
    private:

        static void processSolver(const JSONNode & nd, SolverData& data) {
            bool nstepsset = false;
            bool epsset = false;
            for (auto i = nd.begin(); i != nd.end(); i++) {
                if (i->name() == NSTEPS_NAME) {
                    long long int ns = i->as_int();
                    if(ns == -1) 
                        data.mNSteps = std::numeric_limits<long long int>::max();
                    else 
                        data.mNSteps = ns;
                    nstepsset = true;
                } else if (i->name() == EPS_NAME) {
                    data.mEps = i->as_float();
                    epsset = true;
                } else {
                    BNB_ERROR_REPORT("Illegal name on parsing input file");
                }
            }
            BNB_ASSERT(nstepsset && epsset);
        }

        static void processProblem(const JSONNode & nd, NlpProblem<double>& prob) {
            bool objset = false;
            bool boxset = false;
            for (auto i = nd.begin(); i != nd.end(); i++) {
                if (i->name() == OBJECTIVE_NAME) {
                    std::string objs = i->as_string();
                    processObjective(objs, prob);
                    objset = true;
                } else if (i->name() == PROBLEM_BOX_NAME) {
                    BNB_ASSERT(objset);
                    processProblemBox(*i, prob);
                    boxset = true;
                } else {
                    BNB_ERROR_REPORT("Illegal name on parsing input file");
                }
            }
            BNB_ASSERT(objset && boxset);
        }

        static void processProblemBox(const JSONNode & nd, NlpProblem<double>& prob) {
            int n = prob.mObj->getDim();
            Box<double> box(n);
            readBox(nd, box);
            prob.mBox = box;
        }

        static void readBox(const JSONNode & nd, Box<double> & box) {
            int n = box.mDim;
            for (auto i = nd.begin(); i != nd.end(); i++) {
                if (i->name() == BOX_A_NAME) {
                    int k = readVector(*i, (double*) (box.mA));
                    BNB_ASSERT(k == n);
                } else if (i->name() == BOX_B_NAME) {
                    int k = readVector(*i, (double*) (box.mB));
                    BNB_ASSERT(k == n);
                } else {
                    BNB_ERROR_REPORT("Unknown name while processing subproblem");
                }
            }
        }

        static void processObjective(const std::string& objs, NlpProblem<double>& prob) {
            std::map < std::string, int> idents;
            Polynom<double> *poly = new Polynom<double>();
            PolyUtil::fromString(objs.c_str(), idents, *poly);
            PolynomTxt fmt;
            std::string ss = PolyUtil::toString(fmt, *poly);
            std::cout << "Polynom: " << ss << "\n";
            PolyObjective<double>* pobj = new PolyObjective<double>(poly);
            prob.mObj = pobj;
        }

        static void processState(const JSONNode & nd, BNCState<double>& state, const NlpProblem<double>& prob) {
            bool recordset = false;
            bool subset = false;
            int n = prob.mBox.mDim;
            for (auto i = nd.begin(); i != nd.end(); i++) {
                if (i->name() == RECORD_NAME) {
                    processRecord(*i, n, state);
                    recordset = true;
                } else if (i->name() == SUBPROBLEMS_NAME) {
                    processSubproblems(*i, n, state);
                    subset = true;
                } else {
                    BNB_ERROR_REPORT("Illegal name on processing state");
                }
            }
            if (!recordset) {
                // Do nothing
            }
            if (!subset) {
                makeDefaultSubs(nd, state, prob);
            }
        }

        static void processRecord(const JSONNode & nd, int n, BNCState<double>& state) {
            double v;
            double x[n];
            for (auto i = nd.begin(); i != nd.end(); i++) {
                if (i->name() == RECORD_VALUE_NAME) {
                    v = i->as_float();
                } else if (i->name() == RECORD_POINT_NAME) {
                    int k = readVector(*i, x);
                    BNB_ASSERT(k == n);
                } else {
                    BNB_ERROR_REPORT("Unknown name while processing record\n");
                }
            }
            state.mRecord->update(v, x);
        }

        static void processSubproblems(const JSONNode & nd, int n, BNCState<double>& state) {
            for (auto i = nd.begin(); i != nd.end(); i++) {
                processSub(*i, n, state);
            }
        }

        static void processSub(const JSONNode & nd, int n, BNCState<double>& state) {
            BNBNode* node = new BNBNode;
            BNCSub<double>* sub = new BNCSub<double>();
            Box<double> box(n);
            readBox(nd, box);
            sub->mBox = box;
            node->mData = sub;
            state.mTreeManager->reg(node);
            BNBTree* tree = new BNBTree(node);
            state.mForest.push_back(tree);
        }

        static void makeDefaultSubs(const JSONNode & nd, BNCState<double>& state, const NlpProblem<double>& prob) {
            BNBNode* node = new BNBNode;
            BNCSub<double>* sub = new BNCSub<double>();
            sub->mBox = prob.mBox;
            node->mData = sub;
            state.mTreeManager->reg(node);
            BNBTree* tree = new BNBTree(node);
            state.mForest.push_back(tree);
        }

        static int readVector(const JSONNode& nd, double * x) {
            int k = 0;
            for (auto i = nd.begin(); i != nd.end(); i++) {
                double u = i->as_float();
                x[k++] = u;
            }
            return k;
        }
    };

}
#endif	/* PARSEINP_HPP */

