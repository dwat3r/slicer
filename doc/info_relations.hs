{-# LANGUAGE DuplicateRecordFields #-}

import Data.List
import Control.Monad (liftM2)

data    Expr = Expr {text :: String, vars :: [Var]}
    deriving (Eq)
newtype Var  = Var String
    deriving (Eq)

data StmtType = Empty | Assign | Seq | Cond | Cond_else | Rep
    deriving (Eq,Show)

data Stmt = Stmt {stype :: StmtType, 
                  text  :: String, 
                  expr  :: Expr,      
                  var   :: Var,
                  children :: [Stmt]}

instance Show Expr where
    show (Expr t _) = t
instance Show Var where
    show (Var t) = t
instance Show Stmt where
    show (Stmt st t _ _ _ ) = show st ++ ":" ++ t
 
def :: Stmt -> [Var]
def (Stmt Empty  t e v cs) = []
def (Stmt Assign t e v cs) = [v]
def (Stmt _      t e v cs) = mapWith union def cs

-- current stmt, all (stmts,vars), vars
pres :: ([Stmt],[Var]) -> Stmt -> [Var]
pres (ss,vs) (Stmt Empty  t e v cs) = vs
pres (ss,vs) (Stmt Assign t e v cs) = vs \\ [v]
pres (ss,vs) (Stmt Seq    t e v cs) = mapWith intersect (pres (ss,vs)) cs
pres (ss,vs) (Stmt Cond   t e v cs) = mapWith union (pres (ss,vs)) cs
pres (ss,vs) (Stmt _      t e v cs) = vs

lam :: Stmt -> [(Var,Expr)]
lam s@(Stmt Empty     t e v cs) = []
lam s@(Stmt Assign    t e v cs) = vars e <+> [e]
lam s@(Stmt Seq       t e v cs) = concat $ zipWith (<.>) (scanl (<.>) [] $ map p cs) $ map lam cs 
lam s@(Stmt Cond      t e v cs) = (vars e <+> [e]) `union` mapWith union lam cs
lam s@(Stmt Cond_else t e v cs) = (vars e <+> [e]) `union` mapWith union lam cs
lam s@(Stmt Rep       t e v cs) = transClos (concatMap p cs) <.> (vars e <+> [e] `union` concatMap lam cs) 


u :: Stmt -> [(Expr,Var)] 
u s@(Stmt Empty     t e v cs) = []
u s@(Stmt Assign    t e v cs) = [(e,v)]
u s@(Stmt Seq       t e v cs) = concat $ zipWith relComp (map u cs) $ tail (map p cs) ++ []
u s@(Stmt Cond      t e v cs) = ([e] <+> map def cs) ++ mapWith union u cs
u s@(Stmt Cond_else t e v cs) = undefined
u s@(Stmt Rep       t e v cs) = undefined

p :: Stmt -> [(Var,Var)]
p s = undefined


mapWith :: (Eq b) => ([b] -> [b] -> [b]) -> (a -> [b]) -> [a] -> [b]
mapWith hf f xs = foldl1 hf $ map f xs

--cartesian prod
(<+>) = liftM2 (,)

--relational composition
(<.>) :: (Eq b) => [(a,b)] -> [(b,c)] -> [(a,c)]
(<.>) xs ys = map (\((a,b),(c,d)) -> (a,d)) $ filter (\(x,y) -> snd x == fst y) $ zip xs ys

--transitive closure
transClos :: (Eq a) => [(a,a)] -> [(a,a)]
transClos xs =  concat $ take (length uniqs) $ iterate (relComp pairs) pairs
    where
     uniqs = nub $ (\(x,y) -> x ++ y) $ unzip xs
     pairs = (xs ++ zip uniqs uniqs)