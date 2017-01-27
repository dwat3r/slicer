# Slicing:

###Tasks need to be done:
 - flow dependence
 - aliasing
 - control dependence

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

"." is relational join.
v is variable, e is expression, S is statement.

for backward-slicing:
the "u" function:
(v,e) is in u(S) iff 
the value computed for e potentially affects 
the value of v on exit from S

sequence:
u(seq(S1,S2)) = (u(S1) . u(S2)) U u(S2)

