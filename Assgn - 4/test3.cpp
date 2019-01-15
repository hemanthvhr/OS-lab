#include "myfs.h"
#include <vector>
#include <algorithm>

int main()
{
	char c;
	restore_myfs("mydump-16.backup");
	ls_myfs();
	int fd = open_myfs("mytest.txt", 'r');
	vector<int> content;
	char buffer[10];
	for(int i=0; i<100; i++)
	{
		bzero(buffer,10);
		for(int j=0; j<10; j++)
		{
			 read_myfs(fd, 1, &c);
			 if(c=='\n')
			 {
			 	buffer[j]='\0';
			 	break;
			 }
			 buffer[j] = c;
		}
		content.push_back(atoi(buffer));
	}
	close_myfs(fd);
	printf("printing contents of mytest.txt\n");
	showfile_myfs("mytest.txt");
	sort(content.begin(),content.end());
	fd = open_myfs("sorted.txt", 'w');
	for(int i=0; i<100; i++)
	{
		bzero(buffer,10);
		sprintf(buffer,"%d\n",content[i]);
		write_myfs(fd, strlen(buffer), buffer);
	}
	close_myfs(fd);
	printf("printing contents of sorted.txt\n");
	showfile_myfs("sorted.txt");
}
