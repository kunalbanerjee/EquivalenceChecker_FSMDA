# EquivalenceChecker_FSMDA
### Kunal Banerjee
#### Feb 11, 2016

This tool is a product of our works reported below:
* [A Translation Validation Framework for Symbolic Value Propagation based Equivalence Checking of FSMDAs, SCAM 2015](http://ieeexplore.ieee.org/document/7335421/?arnumber=7335421)
* [Extending the FSMD Framework for Validating Code Motions of Array-Handling Programs, TCAD 2014](http://ieeexplore.ieee.org/document/6951853/?arnumber=6951853)
* [Verification of Code Motion Techniques Using Value Propagation, TCAD 2014](http://ieeexplore.ieee.org/document/6856295/?arnumber=6856295)
* [A Value Propagation Based Equivalence Checking Method for Verification of Code Motion Techniques, ISED 2012](http://ieeexplore.ieee.org/document/6526555/?tp=&arnumber=6526555)

This folder contains the following files:

* ABOUT.txt:       Copyright notice.

* FsmdaEqvChecker: This folder contains the tool for symbolic value propagation based equivalence checking of FSMDAs, 
                    see the file README.txt within this folder for further instructions.

* fsmda2dot:       This folder contains the tool to convert an FSMDA file into dot (graph description language) format,
                    see the file README.txt within this folder for further instructions.

* Benchmarks:      This folder contains some sample FSMDAs; note that each sub-folder within this folder has 6 files:
  * benchmark_org.txt  <-- Original FSMDA
  * benchmark_org.dot  <-- Original FSMDA in dot format
  * benchmark_org.pdf  <-- The graph corresponding to original FSMDA in pdf format
  * benchmark_schd.txt <-- Transformed (scheduled) FSMDA
  * benchmark_schd.dot <-- Transformed FSMDA in dot format
  * benchmark_schd.pdf <-- The graph corresponding to transformed FSMDA in pdf format

* ConstructFSMDA.pdf: This file describes how FSMDAs can be derived from behavioural descriptions and their textual representation.

* README.md:       This file.

If you use this tool, then please cite the following work: <br />
@inproceedings{BanerjeeMS15, <br />
  author    = {Kunal Banerjee and Chittaranjan Mandal and Dipankar Sarkar}, <br />
  title     = {A Translation Validation Framework for Symbolic Value Propagation based Equivalence Checking of FSMDAs}, <br />
  booktitle = {15th {IEEE} International Working Conference on Source Code Analysis and Manipulation, {SCAM} 2015, Bremen, Germany, September 27-28, 2015}, <br />
  pages     = {247--252}, <br />
  year      = {2015}, <br />
  url       = {<http://dx.doi.org/10.1109/SCAM.2015.7335421>}, <br />
  doi       = {10.1109/SCAM.2015.7335421}, <br />
}


For more information regarding any of the above mentioned objects, please contact:
kunal.banerjee.cse@gmail.com
