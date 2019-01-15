#include "myfs.h"

int main()
{
	int n=0;
	char dest[30];
	int fd1;
	int fd2;
	int size;
	char temp[1000];
	srand(time(NULL));
	if(create_myfs(10*1024*1024)==-1)
		printf("Error creating file system\n");
	fd1 = open_myfs("mytest.txt", 'w');
	char buf[30];
	for(int i=0; i<100; i++)
	{
		bzero(buf,30);
		int temp = rand()%90+10;
		sprintf(buf,"%d\n",temp);
		write_myfs(fd1, strlen(buf), buf);
	}
	close_myfs(fd1);
	printf("Enter an integer:");
	scanf("%d", &n);
	for(int i=0;i<n;i++)
	{
		sprintf(dest,"mytest-%d.txt",i+1);
		fd1 = open_myfs("mytest.txt", 'r');
		fd2 = open_myfs(dest, 'w');
		while((size=read_myfs(fd1,1000,temp))>0)
		{
			write_myfs(fd2,size,temp);
		}
		close_myfs(fd2);
		close_myfs(fd1);
	}
	if(dump_myfs("mydump-16.backup")==-1)
		printf("Error in Dumping\n");
	printf("Mydump-16.backup has been successfully generated\n");
}
