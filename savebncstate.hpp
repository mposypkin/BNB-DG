/* 
 * File:   savestate.hpp
 * Author: mposypkin
 * 
 * Serialize the state of Branch-and-Cut solver
 *
 * Created on August 14, 2015, 5:12 PM
 */

#ifndef SAVEBNCSTATE_HPP
#define	SAVEBNCSTATE_HPP

#include <iostream>
#include <problems/nlp/bnc/bncstate.hpp>
#include <problems/nlp/bnc/bncsub.hpp>

#include "bnbdgformat.hpp"

class SaveBNCState {
public:

    struct SaveNode {

        SaveNode(int n, int sz, std::ostream& os) : mN(n), mSz(sz), mOS(os) {
        }

        void operator()(BNBNode* node) {
            BNCSub<double>* sub = (BNCSub<double>*)node->mData;
            mOS << "{";
            mOS << "\"" << BOX_A_NAME << "\" : [";
            for (int i = 0; i < mN; i++)
                mOS << sub->mBox.mA[i] << ((i == mN - 1) ? "" : ", ");
            mOS << "], ";
            mOS << "\"" << BOX_B_NAME << "\" : [";
            for (int i = 0; i < mN; i++)
                mOS << sub->mBox.mB[i] << ((i == mN - 1) ? "" : ", ");
            mOS << "]";
            mOS << "}";
            if (--mSz > 0)
                mOS << ", ";
        };
        int mN;
        long int mSz;
        std::ostream& mOS;
    };

    static void saveState(int n, const BNCState<double>& state, std::ostream& os) {
        os << "{\n";
        os << "\"" << RECORD_NAME << "\"" << " : ";
        os << "{";
        os << "\"" << RECORD_VALUE_NAME << "\"" << " : " << state.mRecord->getValue() << ",";
        os << "\"" << RECORD_POINT_NAME << "\"" << " : ";
        os << "[";
        for (int i = 0; i < n; i++)
            os << state.mRecord->getX()[i] << ((i == n - 1) ? "" : ", ");
        os << "]";
        os << "}";
        int sz = state.mTreeManager->size();
        if (sz > 0) {
            os << ",\n";
            os << "\"" << SUBPROBLEMS_NAME << "\"" << " : ";
            os << "[";
            SaveNode sn(n, sz, os);
            state.mTreeManager->traverse(sn);
            os << "]";
        }
        os << "\n}\n";
    }

private:

};

#endif	/* SAVESTATE_HPP */

