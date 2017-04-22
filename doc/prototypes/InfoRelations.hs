{-# LANGUAGE DuplicateRecordFields #-}
module InfoRelations where

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
emptyVar = Var ""
emptyExpr = Expr "" []

empty            = Stmt Empty "" emptyExpr emptyVar []
assign t e v     = Stmt Assign t e v []
sseq cs          = Stmt Seq "" emptyExpr emptyVar cs
cond t e cs      = Stmt Cond t e emptyVar cs
cond_else t e cs = Stmt Cond t e emptyVar cs
rep t e cs       = Stmt Rep  t e emptyVar cs
