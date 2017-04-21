{-# LANGUAGE DuplicateRecordFields #-}

-- data
import Data.List
import Control.Monad (liftM2)

data    Expr = Expr {text :: String, vars :: [Var]}

newtype Var  = Var String

data StmtType = Empty | Assign | Seq | Cond | Cond_else | Rep
    deriving (Eq,Show)

data Stmt = Stmt {stype :: StmtType, 
                  text  :: String, 
                  expr  :: Expr,      
                  var   :: Var,
                  children :: [Stmt]}
instance Eq Expr where
  (==) (Expr t1 _) (Expr t2 _) = t1 == t2
instance Eq Var where
  (==) (Var v1) (Var v2) = v1 == v2
instance Show Expr where
    show (Expr t _) = t
instance Show Var where
    show (Var t) = t
instance Show Stmt where
    show (Stmt st t _ _ []) = show st ++ ":" ++ t
    show (Stmt st t _ _ cs) = show st ++ ":" ++ t ++ "[" ++ (concat $ intersperse ",\n" $ map show cs) ++ "]"

-- utils
mapWith :: (Eq b) => ([b] -> [b] -> [b]) -> (a -> [b]) -> [a] -> [b]
mapWith hf f xs = foldl1 hf $ map f xs

uunion :: (Eq b) => (a -> [b]) -> [a] -> [b]
uunion     = mapWith union

iintersect :: (Eq b) => (a -> [b]) -> [a] -> [b]
iintersect = mapWith intersect

--cartesian prod
(<+>) = liftM2 (,)

--relational composition
(<.>) :: (Eq a, Eq b, Eq c) => [(a,b)] -> [(b,c)] -> [(a,c)]
(<.>) xs ys = nub $ map (\((a,b),(c,d)) -> (a,d)) $ filter (\(x,y) -> snd x == fst y) $ xs <+> ys

--transitive closure
transClos :: (Eq a) => [(a,a)] -> [(a,a)]
transClos xs =  nub $ concat $ take (length uniqs) $ iterate ((<.>) pairs) pairs
    where
     uniqs = nub $ (\(x,y) -> x ++ y) $ unzip xs
     pairs = (xs ++ zip uniqs uniqs)

idd vs = [(v,w) | v <- vs,w <- vs, v == w]

-- relations
def :: Stmt -> [Var]
def (Stmt Empty  t e v cs) = []
def (Stmt Assign t e v cs) = [v]
def (Stmt _      t e v cs) = uunion def cs

-- current stmt, all (stmts,vars), vars
pres :: ([Stmt],[Var]) -> Stmt -> [Var]
pres (ss,vs) (Stmt Empty  t e v cs) = vs
pres (ss,vs) (Stmt Assign t e v cs) = vs \\ [v]
pres (ss,vs) (Stmt Seq    t e v cs) = iintersect (pres (ss,vs)) cs
pres (ss,vs) (Stmt Cond   t e v cs) = uunion (pres (ss,vs)) cs
pres (ss,vs) (Stmt _      t e v cs) = vs

lam :: [Var] -> Stmt -> [(Var,Expr)]
lam _  s@(Stmt Empty     t e v cs) = []
lam _  s@(Stmt Assign    t e v cs) = vars e <+> [e]
lam vs s@(Stmt Seq       t e v cs) = concat $ zipWith (<.>) ([] ++ scanl1 (<.>) (map (p vs) cs)) $ map (lam vs) cs 
lam vs s@(Stmt Rep       t e v cs) = transClos (p vs (head cs)) <.> (vars e <+> [e] `union` uunion (lam vs) cs) 
lam vs s@(Stmt _         t e v cs) = (vars e <+> [e]) `union` uunion (lam vs) cs


u :: [Var] -> Stmt -> [(Expr,Var)] 
u _  s@(Stmt Empty     t e v cs) = []
u _  s@(Stmt Assign    t e v cs) = [(e,v)]
u vs s@(Stmt Seq       t e v cs) = concat $ zipWith (<.>) (map (u vs) cs) $ tail (map (p vs) cs) ++ []
u vs s@(Stmt Rep       t e v cs) = ([e] <+> (def (head cs))) `union` u vs (head cs) <.> transClos (p vs (head cs)) <.> ((vars e <+> def (head cs)) `union` idd vs)
u vs s@(Stmt _         t e v cs) = ([e] <+> uunion def cs) `union` uunion (u vs) cs

p :: [Var] -> Stmt -> [(Var,Var)]
p vs (Stmt Empty     t e v cs) = idd vs
p vs (Stmt Assign    t e v cs) = vars e <+> [v] `union` (idd vs \\ [(v,v)])
p vs (Stmt Seq       t e v cs) = mapWith (<.>) (p vs) cs
p vs (Stmt Cond      t e v cs) = (vars e <+> (uunion def cs)) `union` uunion (p vs) cs `union` idd vs
p vs (Stmt Cond_else t e v cs) = (vars e <+> (uunion def cs)) `union` uunion (p vs) cs
p vs (Stmt Rep       t e v cs) = transClos (p vs (head cs)) <.> ((vars e <+> def (head cs)) `union` idd vs)

-- slice
e :: [Expr] -> [Var] -> (Var,Stmt) -> [Expr]
e es vs (v,s) = [ex | ex <- es, (ex,v) `elem` u vs s]



-- data
--constructors
empty            = Stmt Empty "" emptyExpr emptyVar []
assign t e v     = Stmt Assign t e v []
sseq cs          = Stmt Seq "" emptyExpr emptyVar cs
cond t e cs      = Stmt Cond t e emptyVar cs
cond_else t e cs = Stmt Cond t e emptyVar cs
rep t e cs       = Stmt Rep  t e emptyVar cs
-- gcd
-- exprs
emptyExpr = Expr "" []
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
emptyVar = Var ""
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

