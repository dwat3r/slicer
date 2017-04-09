## Dependences
  - Control and Data
  - Describe CFG 
    - Start/Exit nodes
    - basic block nodes
    - control flow edges
  - Data dependence:
    - Def(i) Ref(i) for defined & referenced at a CFG node i
    - types of data deps: flow, output, anti-dependence. 
    - for slicing we need only flow, these can be distinguished as: def-def, def-use, backedge in loop, loop-carried, loop-independent 
    - write what reaching definition is
  - Control dependence:
    - Definition of post dominance
  - Write example examination of horwitz.cc with source, CFG and PDG




## Dataflow Equations

- Mark Weiser's original definition
- Slice as an executable program which has only the relevant statements
- slicing criterion (n,V) pair: n node in the CFG, V is a subset of the program's variables
- statement-minimal slices
- two algorithm steps:
 - tracing transitive data dependences (iterate when facing with loops)
 - tracing control dependences
 