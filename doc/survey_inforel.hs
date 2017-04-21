import InfoRelations

-- slicing survey tip

-- vars
n = Var "n"
i = Var "i"
x = Var "x"
-- exprs
en = Expr "n" [n]
e1 = Expr "1" []
eilen = Expr "i <= n" [i,n]
eimod2eq0 = Expr "i mod 2 = 0" [i]
e17 = Expr "17" []
e18 = Expr "18" []
eip1 = Expr "i + 1" [i]
ex = Expr "x" [x]
-- stms
s1 = assign "read(n)" en n
s2 = assign "i = 1"   e1 i
s3 = rep "while (i <= n)" eilen [s4]
s4 = cond_else "if (i mod 2 = 0)" eimod2eq0 [s5,s6,s7]
s5 = assign "x = 17" e17 x
s6 = assign "x = 18" e18 x
s7 = assign "i = i + 1" eip1 i
s8 = assign "write(x)" ex x

fn = sseq [s1,s2,s3,s8]

fvars  = [n,i,x]
fexprs = [en,e1,eilen,eimod2eq0,e17,e18,eip1,ex]
fstmts = [s1,s2,s3,s4,s5,s6,s7,s8]
