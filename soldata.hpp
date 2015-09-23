/* 
 * File:   soldata.hpp
 * Author: medved
 * Data definition for a solver
 * 
 * Created on September 23, 2015, 9:02 AM
 */

#ifndef SOLDATA_HPP
#define	SOLDATA_HPP

namespace bnbdg {
    
    /**
     * Structure defining solver's data
     */
    struct SolverData {
        /**
         * Number of steps to perform
         */
        long long int mNSteps;
        
        /**
         * Precision
         */
        double mEps;
    };
    
}


#endif	/* SOLDATA_HPP */

