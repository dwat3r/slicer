Slicing:
========

###Tasks need to be done:
 - flow dependence
 - aliasing
 - control dependence
 - transitive closure
 - what if compoundStmt has only 1 element?

###Aspects of performance:
 - what should be stored
 - how should it be stored
 - when should parsing happen
    - (can we do multiple slices with one parse)

##First approach:

###Information-flow relations:

hash set:
variable -> expressions (with lines) which may affect it's value

slicing-survey-tip.pdf page 10.

 - "." is relational join.
 - "x" is descartes product.
 - "*" is transitive closure.
 - v is variable, e is expression, S is statement.

####for backward-slicing:
#####the "u" function:

 - (v,e) is in u(S) iff 
 - the value computed for e potentially affects 
 - the value of v on exit from S

sequence:
u_seq(S1,S2) = (u(S1) . p(S2)) U u(S2)

assignment:
u_assign(v,e) = {(e,v)}

if then else:
u_if(e,S1,S2) = {e} x (DEFS(S1) U DEFS(S2))) U u(S1) U u(S2)

while:
u_while(e,S) = ({e} x DEFS(S)) U u(S) . p*(S) . ((VARS(e) x DEFS(S)) U Id)

#####the "p" function:

 - (v,v') is in p(S) iff
 - the value of v on entry to S may affect
 - the value of v' on exit from S

sequence:
p_seq(S1,S2) = p(S1) . p(S2)

assignment:
p_assign(v,e) = (VARS(e) x {v}) U (ID - (v,v))

if then else:
p_if(e,S1,S2) = (VARS(e) x (DEFS(S1) U DEFS(S2))) U p(S1) U p(S2)

while:
p_while(e,S) = p*(S) . ((VARS(e) x DEFS(S)) U ID)

Computing a slice requires a dummy assignment v' := v

####Program structure:
 - Get source file, and line no.
 - Parse AST and gather info to the Statement graph.
 - Compute slice regarding the input line no.
 - Print (prettily) the slice.