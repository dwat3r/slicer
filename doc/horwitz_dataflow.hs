import Data.List

newtype Var  = Var {getName :: String}
  deriving (Eq, Show)

data Stmt = Stmt {
                   text  :: String, -- source text
                   def   :: [Var],  -- defines variables
                   ref   :: [Var],  -- uses variables
                   infl  :: [Stmt], -- stmts control dependent on this
                   edges :: [Stmt]  -- CFG edges
                  }
   deriving (Eq)

instance Show Stmt where
    show s = text s

-- c is criterion
type Criterion = (Stmt,[Var])

-- directly relevant variables and statements with 0
-- indirectly relevant variables and statements with > 0
rj :: Int -> Criterion -> Stmt -> [Stmt] -> [Var]
rj k c i ss = concat [r k c j ss | j <- edges i]

-- i -cfg-> j
r :: Int -> Criterion -> Stmt -> [Stmt] -> [Var]
r 0 c i ss = initi ++ [v | v <- rj 0 c i ss, v `notElem` def i ] ++ 
          [v | v <- ref i, not $ null (def i `intersect` rj 0 c i ss) ]
          where initi | i == fst c = snd c
                      | otherwise  = []

r k c i ss = r (k-1) c i ss ++ nub (concat [r 0 (b,ref b) i ss | b <- b (k-1) c ss])

--todo recursion to eliminate infinite loops
s :: Int -> Criterion -> [Stmt] -> [Stmt]
s 0 c ss = [i | i <- ss, not $ null (def i `intersect` rj 0 c i ss)]
s k c ss = b (k-1) c ss ++ [i | i <- ss, not $ null (def i `intersect` rj k c i ss)]

b :: Int -> Criterion -> [Stmt] -> [Stmt]
b k c ss = [bi | bi <- s k c ss,not $ null $ infl bi]

-- sample data from horwitz:
vi = Var "i"
vsum = Var "sum"
n0 = Stmt {text = "int sum = 0;",   def = [vsum], ref = [],     infl = [],     edges = [n1]}
n1 = Stmt {text = "int i = sum;",   def = [vi],   ref = [vsum], infl = [],     edges = [n2]}
n2 = Stmt {text = "while (i < 11)", def = [],     ref = [vi],   infl = [n3,n4],edges = [n3]}
n3 = Stmt {text = "sum += i;",      def = [vsum], ref = [vi],   infl = [],     edges = [n4]}
n4 = Stmt {text = "i++",            def = [vi],   ref = [vi],   infl = [],     edges = []}

horwitz :: [Stmt]
horwitz = [n0,n1,n2,n3,n4]

c :: Criterion
c = (n4,[vi])