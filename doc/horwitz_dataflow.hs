import Data.List
import Control.Monad.State
import Debug.Trace

newtype Var  = Var {getName :: String}
  deriving (Eq, Show)

data Stmt = Stmt {
                   text  :: String, -- source text
                   def   :: [Var],  -- defines variables
                   ref   :: [Var],  -- uses variables
                   infl  :: [Stmt], -- stmts control dependent on this
                   edges :: [Stmt]  -- CFG edges
                  }

instance Eq Stmt where
  (==) (Stmt t1 _ _ _ _) (Stmt t2 _ _ _ _) = t1 == t2

instance Show Stmt where
    show s = text s

-- c is criterion
type Criterion = (Stmt,[Var])

-- directly relevant variables and statements with 0
-- indirectly relevant variables and statements with > 0
rj :: Int -> Criterion -> Stmt -> [Stmt] -> [Stmt] -> [Var]
rj k c i ss vs = concat [r k c j ss (j:vs) | j <- edges i]

-- i -cfg-> j
-- it, crit, curstmt, visitedstmts, stmts , affectedvars
r :: Int -> Criterion -> Stmt -> [Stmt] -> [Stmt] -> [Var]
r 0 c i ss vs | i `elem` vs = []
              | otherwise   =  initi ++ [v | v <- rj 0 c i ss (i:vs), v `notElem` def i ] ++ 
                            [v | v <- ref i, not $ null (def i `intersect` rj 0 c i ss (i:vs)) ]
                            where initi | i == fst c = snd c
                                        | otherwise  = []

r k c i ss vs | i `elem` vs = []
              | otherwise   = r (k-1) c i ss (i:vs) ++ nub (concat [r 0 (b,ref b) i ss (i:vs) | b <- b (k-1) c ss vs])

--todo recursion to eliminate infinite loops
s :: Int -> Criterion -> [Stmt] -> [Stmt] -> [Stmt]
s 0 c ss vs = [i | i <- ss, not $ null (def i `intersect` rj 0 c i ss vs)]
s k c ss vs = b (k-1) c ss vs ++ [i | i <- ss, not $ null (def i `intersect` rj k c i ss (i:vs))]

b :: Int -> Criterion -> [Stmt] -> [Stmt] -> [Stmt]
b k c ss vs = [bi | bi <- s k c ss vs,not $ null $ infl bi]


-- sample data from horwitz:
vi = Var "i"
vsum = Var "sum"
n0 = Stmt {text = "int sum = 0;",   def = [vsum], ref = [],     infl = [],     edges = [n1]}
n1 = Stmt {text = "int i = sum;",   def = [vi],   ref = [vsum], infl = [],     edges = [n2]}
n2 = Stmt {text = "while (i < 11)", def = [],     ref = [vi],   infl = [n3,n4],edges = [n3]}
n3 = Stmt {text = "sum += i;",      def = [vsum], ref = [vi],   infl = [],     edges = [n4]}
n4 = Stmt {text = "i++",            def = [vi],   ref = [vi],   infl = [],     edges = [n2]}

horwitz :: [Stmt]
horwitz = [n0,n1,n2,n3,n4]

c :: Criterion
c = (n4,[vi])

{-
todo.
type Iteration = Int
type VisitAmount = Int
type SStat = (([(Stmt,VisitAmount)],Criterion,Iteration),[Stmt])
type SVal  = [Stmt]

r :: Stmt -> State SStat SVal
r i = do
  (cur@(ss,c,it),res) <- get
  case it of
    0 -> put (cur,initi ++ )
    _ -> 
    where initi | i == fst c = snd c
              | otherwise  = []

s :: State SStat SVal
s = do
  (ss,_,_) <- get

b :: State SStat SVal
b = do



computeSlice :: [Stmt] -> State SStat SVal
computeSlice = 



-}