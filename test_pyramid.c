#include <stdio.h>
#include <stdlib.h>

int main()
{
	int a= 5;
	int n = a;
	int b = 1;
	int cnt;
	while(b <= n)
	{
		cnt = 1;
		int c = b+1;
		while(c <= n){
			printf(" ");
			c++;
		}
		while(cnt <= 2*b-1)
		{
			printf("#");
			cnt++;
		}
		printf("\n");
		b++;
	}
	return 0;
}
