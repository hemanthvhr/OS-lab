#include<bits/stdc++.h>
#include<sys/types.h>
#include<unistd.h>
using namespace std;


int main()
{
	pid_t pida,pidb,pidc,pidd;
	int pipd[2],pipc[2],pipb[2];
	int array[100];
	pipe(pipd);
	pipe(pipc);
	pipe(pipb);
	pidc=fork();
	if(pidc==0)
	{
		pidd=fork();
		if(pidd==0)
		{
			pidd=getpid();			//process d
			int temp=100,ret;
			while(temp--)
			{
				array[temp-1]=rand();
				//cout<<"process d "<<array[temp-1]<<endl;
			}
			sort(array,array+100);
			temp=100;
			while(temp--)
			{
				ret=write(pipd[1],(void *)(array+temp),sizeof(int));
			}
		}
		else
		{
			pidc=getpid();			//process c
			int temp=100,ret;
			while(temp--)
			{
				array[temp-1]=rand();
				//cout<<"process c "<<array[temp-1]<<endl;
			}
			sort(array,array+100);
			temp=100;
			while(temp--)
			{
				ret=write(pipc[1],(void *)(array+temp),sizeof(int));
			}
		}
	}
	else
	{
		pidb=fork();
		if(pidb==0)
		{
			pidb=getpid();			//process b
			int temp=100,ret;
			while(temp--)
			{
				array[temp-1]=rand();
				cout<<"process b "<<array[temp-1]<<endl;
			}
			sort(array,array+100);
			temp=100;
			while(temp--)
			{
				ret=write(pipb[1],(void *)(array+temp),sizeof(int));
			}
		}
		else
		{
			pida=getpid();			//process a
			int arraya[300];
			
			for(int i=0;i<300;i+3){
				read(pipd[0],(void *)&arraya[i],sizeof(int));
				cout<<"process a "<<arraya[i]<<endl;
				read(pipc[0],(void *)&arraya[i+1],sizeof(int));
				cout<<"process a "<<arraya[i+1]<<endl;
				read(pipb[0],(void *)&arraya[i+2],sizeof(int));
				cout<<"process a "<<arraya[i+2]<<endl;
				}
			sort(arraya,arraya+300);
			for(int i=0;i<300;i++)
			{
				cout<<arraya[i]<<endl;
			}
		}
	}
}

