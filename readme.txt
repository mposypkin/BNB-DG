
Program bnbdg.exe solves NLP (Non-Linear Programming) problems with Branch-and-Bound 
(possibly strengthened with cuts) method. It reads settings from the command line 
and a problem description and a solver's state from a file. The program performs a given number of iterations then
stops and saves a state.

A state consists of the best solution found so far ('record') and a collection of 
non-solved sub-problems. 

The data format for NLP problems only follows general JSON notation

{
# Solver's options
  "solver" : {
# tree traversing strategy (width first search or depth first search)
     "treetraverse" : "wfs" | "dfs"
# number of steps to perform ("inf" for infinite number)
     "nsteps" : number_of_steps,
# precision
     "eps" : eps
   },
# Problem definition 
  "problem" : {
               "objective" : objective,
               "box" : {"a" : [...], "b": [...]}
              },
 
# Record value and vector
  "record" : {"v": value, "x": value},

# A collection of subproblems where a, b are lower and upper bounds for boxes
  "subproblems": [{"a":[...], "b":[...]}],
}

Example:

{
    "solver": {
        "treetraverse" : "wfs",
        "nsteps": 100,
        "eps": 0.001
    },
    "problem": {
        "objective": "x^2 - y^2",
        "box": {
            "a": [-3, -3],
            "b": [3, 3]
        }
    },
    "state": {
        "record": {
            "v": 0.0,
            "x": [0, 0]
        },
        "subproblems": [{
                "a": [-3, -3],
                "b": [0, 0]
            }, {
                "a": [-3, 0],
                "b": [0, 3]
            }, {
                "a": [0, 0],
                "b": [3, 3]
            }, {
                "a": [0, -3],
                "b": [3, 0]
            }]
    }
}

Notes: 
1. Box is always defined by two opposite vertices a and b: box = {x : a <= x <= b}.
2. Only polynomial objectives are supported at the moment.
3. Constraints are to be added in future.
4. If record or subproblems filed are missing they assumed to take default values. Default 
   value for record is the maximal allowed number. Default value for subproblems is the list 
   consisting of the source problem.
5. Functional constraints are to be added in future.
6. Missing state means that the state is generated as an initial problem (top tree node).

The program can be run as follows for example:

./bnbdg.exe /tmp/st3.json /tmp/st4.json

where the initial state should be loaded from /tmp/st3.json file and stored in /tmp/st4.json file. 

Script makeinp.pl merges problem and state definition files and obtains new input file.
For example the following line

./makeinp.pl ./probdef1.json /tmp/st1.json > /tmp/pd1.json

generates new input file from ./probdef1.json problem definition and /tmp/st1.json state definition files.

To build the project run 

   make dep
   make

to clean the distribution run
   
   make clean