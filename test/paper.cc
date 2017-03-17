#include <iostream>

int main(){
	int n;
	std::cin >> n;
	int sum = 0;
	int prod = 1;
	for (int i = 1; i <= n; i++){
		sum += i;
		prod *= i;
	}
	std::cout << sum << std::endl;
	std::cout << prod << std::endl;
}