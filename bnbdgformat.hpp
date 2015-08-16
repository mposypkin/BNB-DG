/* 
 * File:   bnbdgformat.hpp
 * Author: mposypkin
 *
 * Format related constants definitions 
 * Created on August 11, 2015, 1:56 PM
 */

#ifndef BNBDGFORMAT_HPP
#define	BNBDGFORMAT_HPP

// Json name for a node storing problem definition
#define PROBLEM_NAME "problem"

// Json name for a node storing problem's the objective
#define OBJECTIVE_NAME "objective"

// Json name for a node storing the bounding box for a problem 
#define PROBLEM_BOX_NAME "box"

// Json name for an node storing the solver's state 
#define STATE_NAME "state"

// Json name for a node storing records
#define RECORD_NAME "record"

// Json name for record value 
#define RECORD_VALUE_NAME "v"

// Json name for record point
#define RECORD_POINT_NAME "x"

// Json name for a node storing subproblems 
#define SUBPROBLEMS_NAME "subproblems"

// Json name for a lower bound of a box
#define BOX_A_NAME "a"

// Json name for an upper bound of a box
#define BOX_B_NAME "b"
#endif	/* BNBDGFORMAT_HPP */

