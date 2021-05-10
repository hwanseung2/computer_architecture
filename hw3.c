#include <stdio.h>

int a,b;
int flag = 1;
int z;
int k;
int one;

void isDivisor()
{
	int x = z;
	int i = 2;
	while(i < x)
	{
		one = 1;
		while(i*one <= x)
		{
			if(i*one == x)
			{
				flag =0;
				one++;
			}
			else
			{
				one++;
				//printf("one : %d\n", one);
			}
			//flag = 0;
			//break;
		}
		i++;
	}
}

void isPrime()
{
	if (flag == 1){
		printf("prime : %d \n", k);
	}
}

int main()
{
	scanf("%d", &a);
	scanf("%d", &b);
	for (k = a; k <=b;k++)
	{	
		//printf("k point : %d\n", k);
		flag = 1;
		z = k;
		isDivisor();
		isPrime();
		//printf("number k : %d flag : %d\n", k, flag);

	}

	return 0;

}
