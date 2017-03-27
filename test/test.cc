int f(int x){
  int y = 0;
  int z;
  //if-else
  if(x>1)
    y = 1;
  else
    y = 2;
  if(x=2){
    y = 1;
    z = 2;
  }else
  {
    y = 2;
    z = 3;
  }
  //while
  while(x>2)
    y = 3;
  //control
  do
  {
    y = 4;
    break;
  }while(x>3);
  do
  {
    y = 5;
    continue;
  }while(x>4);
  //goto
  goto label;
  label:
  // assignment
  y = x;
  return y;
}
