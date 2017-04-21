import InfoRelations

-- gcd
-- exprs
e0  = Expr "0"  []
e1  = Expr "1"  []
em  = Expr "m"  [m] 
en  = Expr "n"  [n] 
ed  = Expr "d"  [d] 
er  = Expr "r"  [r] 
ea1 = Expr "a1" [a1] 
ea2 = Expr "a2" [a2] 
eb1 = Expr "b1" [b1] 
eb2 = Expr "b2" [b2] 
ec  = Expr "c"  [c]
edne0    = Expr "d != 0" [d]
ecdivd   = Expr "c div d" [c,d]
ecmodd   = Expr "c mod d" [c,d]
ea2_qxa1  = Expr "a2 - q * a1" [a2,q,a1]
eb2_qxb1 = Expr "b2 - q * b1" [b2,q,b1]

-- vars
m = Var "m"
n = Var "n"
x = Var "x"
y = Var "y"
z = Var "z"

a1 = Var "a1"
a2 = Var "a2"
b1 = Var "b1"
b2 = Var "b2"
c  = Var "c"
d  = Var "d"
q  = Var "q"
r  = Var "r"

s1 = assign "a1 = 0" e0 a1
s2 = assign "a2 = 1" e1 a2
s3 = assign "b1 = 1" e1 b1
s4 = assign "b2 = 0" e0 b2

s5 = assign "c = m" em c
s6 = assign "d = n" en d

s7 = rep "while (d != 0)" edne0 [s8_19]
s8_19 = sseq [s8,s9,s10,s11,s12,s13,s14,s15,s16,s17,s18,s19]
s8 = assign "q = c div d" ecdivd q
s9 = assign "r = c mod d" ecmodd r
s10 = assign "a2 = a2 - q * a" ea2_qxa1 a2
s11 = assign "b2 = b2 - q * b1" eb2_qxb1 b2
s12 = assign "c = d" ed c
s13 = assign "d = r" er d
s14 = assign "r = a1" ea1 r
s15 = assign "a1 = a2" ea2 a1
s16 = assign "a2 = r" er a2
s17 = assign "r = b1" eb1 r
s18 = assign "b1 = b2" eb2 b1
s19 = assign "b2 = r" er b2

s20 = assign "x = c" ec x
s21 = assign "y = a2" ea2 y
s22 = assign "z = b2" eb2 z

gcds = sseq [s1,s2,s3,s4,s5,s6,s7,s20,s21,s22]

gcdVars  = [m,n,x,y,z,a1,a2,b1,b2,c,d,q,r]
gcdExprs = [e0,e1,em,en,ed,er,ea1,ea2,eb1,eb2,ec,edne0,ecdivd,ecmodd,ea2_qxa1,eb2_qxb1]
gcdStmts = [s1,s2,s3,s4,s5,s6,s7,s8,s9,s10,s11,s12,s13,s14,s15,s16,s17,s18,s19,s20,s21,s22]