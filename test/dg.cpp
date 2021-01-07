#include <assert.h>
#include <stdio.h>

long int fact(int x)
{
	long int r = x;
	int not_r = 1;
	int definitely_not_r = 2;
	while (--x >=2)
		r *= x;


	long still_not_r = 3;
	return r;
}

int main(void)
{
	int a, b, c = 7;

	while (scanf("%d", &a) > 0) {
		assert(a > 0);
		printf("fact: %lu\n", fact(a));
	}

	return 0;
}