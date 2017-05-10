int main(){
  int x = 0;
  int y = 1;
  while(x >= 0){
    if (x > 0){
      y = x; //2. use here 
    }else{
      x = 3;
    }
    x = 2; //1. define here
  }
}