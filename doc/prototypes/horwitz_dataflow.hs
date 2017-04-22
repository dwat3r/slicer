import Data.List
import Control.Monad.State
import Debug.Trace

newtype Var  = Var {getName :: String}

instance Eq Var where
  (==) (Var v1) (Var v2) = v1 == v2

instance Show Var where
  show v = getName v

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
    show s = text s ++ "\n"

-- c is criterion
type Criterion = (Stmt,[Var])

-- directly relevant variables and statements with 0
-- indirectly relevant variables and statements with > 0
rj :: Int -> Criterion -> Stmt -> [Stmt] -> [Stmt] -> [Var]
rj k c i ss vs = concat [r k c j ss vs | j <- edges i]

-- i -cfg-> j
-- it, crit, curstmt, visitedstmts, stmts , affectedvars
r :: Int -> Criterion -> Stmt -> [Stmt] -> [Stmt] -> [Var]
r 0 c i ss vs | length (filter (i==) vs) == 3 = []
              | otherwise   = nub $ initi ++ [v | v <- rj 0 c i ss (i:vs), v `notElem` def i ] ++ 
                            [v | v <- ref i, not $ null (def i `intersect` rj 0 c i ss (i:vs)) ]
                            where initi | i == fst c = snd c
                                        | otherwise  = []

r k c i ss vs = nub $ r (k-1) c i ss vs ++ concat [r 0 (b,ref b) i ss vs | b <- b (k-1) c ss vs]


s :: Int -> Criterion -> [Stmt] -> [Stmt] -> [Stmt]
s 0 c ss vs = nub [i | i <- ss, not $ null (def i `intersect` rj 0 c i ss vs)]
s k c ss vs = nub $ b (k-1) c ss vs ++ [i | i <- ss, not $ null (def i `intersect` rj k c i ss vs)]

b :: Int -> Criterion -> [Stmt] -> [Stmt] -> [Stmt]
b k c ss vs = nub [bi | i <- s k c ss vs, bi <- ss, i `elem` infl bi]


-- sample data from horwitz:
vi = Var "i"
vsum = Var "sum"
n0 = Stmt {text = "int sum = 0;",   def = [vsum], ref = [],        infl = [],     edges = [n1]}
n1 = Stmt {text = "int i = sum;",   def = [vi],   ref = [vsum],    infl = [],     edges = [n2]}
n2 = Stmt {text = "while (i < 11)", def = [],     ref = [vi],      infl = [n3,n4],edges = [n3]}
n3 = Stmt {text = "sum += i;",      def = [vsum], ref = [vsum,vi], infl = [],     edges = [n4]}
n4 = Stmt {text = "i++",            def = [vi],   ref = [vi],      infl = [],     edges = [n2]}

horwitz :: [Stmt]
horwitz = [n0,n1,n2,n3,n4]

c :: Criterion
c = (n4,[vi])

w1 = Stmt {text = "y = x;", def = [Var "y"], ref = [Var "x"], infl = [], edges = [w2]} 
w2 = Stmt {text = "a = b;", def = [Var "a"], ref = [Var "b"], infl = [], edges = [w3]} 
w3 = Stmt {text = "z = y;", def = [Var "z"], ref = [Var "y"], infl = [], edges = []} 

weiser :: [Stmt]
weiser = [w1,w2,w3]
 
w21  = Stmt {text = "A = const", def = [Var "A"], ref = [Var "const"],     infl = [], edges = [w22]}
w22  = Stmt {text = "WHILE P(k)",def = [],        ref = [Var "K"],         infl = [w23,w24,w25,w26,w27,w28], edges = [w23,w29]}
w23  = Stmt {text = "IF Q(C)",   def = [],        ref = [Var "C"],         infl = [w24,w25,w26,w27], edges = [w24,w26]}
w24  = Stmt {text = "B = A",     def = [Var "B"], ref = [Var "A"],         infl = [], edges = [w25]}
w25  = Stmt {text = "X = 1",     def = [Var "X"], ref = [],                infl = [], edges = [w28]}
w26  = Stmt {text = "C = B",     def = [Var "C"], ref = [Var "B"],         infl = [], edges = [w27]}
w27  = Stmt {text = "Y = 2",     def = [Var "Y"], ref = [],                infl = [], edges = [w28]}
w28  = Stmt {text = "K = K + 1", def = [Var "K"], ref = [Var "K"],         infl = [], edges = [w22]}
w29  = Stmt {text = "Z = X + Y", def = [Var "Z"], ref = [Var "X",Var "Y"], infl = [], edges = [w210]}
w210 = Stmt {text = "WRITE(Z)",  def = [],        ref = [Var "Z"],         infl = [], edges = []}

weiser2 :: [Stmt]
weiser2 = [w21,w22,w23,w24,w25,w26,w27,w28,w29,w210]

weiserCrit :: Criterion
weiserCrit = (w210,[Var "Z"])

weiser2s k = s k weiserCrit weiser2 []
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