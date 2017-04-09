Slicing:
========

### Tasks need to be done:
- make matcher either for every type of node which is using a variable, or a generalized one.
- handle continue and break statements in a loop, regarding data dependence
- correct the data dependence builder algorithm when dealing with nested CompoundStatements in a loop
- add more tests

### Aspects of performance:
 - what should be stored
 - how should it be stored (maybe JSON)

#### Program structure:
 - Get source file, and line no.
 - Parse AST via ASTMatchers and create the Statement graph.
 - Insert data dependence edges to it.
 - Compute slice regarding the input line no via a breadth-first search.
 - Print (prettily) the slice. (currently it's a GraphViz dot dump.)