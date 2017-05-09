int f(int x,int y){
  x = 1;
  {
    int x = 2;
    y = x; 
  }
  y = x;
  {
    int x = 3;
    {
      int x = 4;
    }
    x = y;
  }
  return x;
}