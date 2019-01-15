#include "myfs.h"

int main()
{
	if(create_myfs(10*1024*1024)==-1)
		printf("Error creating file system\n");
	copy_pc2myfs("Text1.txt","Text1.txt");
	copy_pc2myfs("Text2.txt","Text2.txt");
	copy_pc2myfs("Text3.txt","Text3.txt");
	copy_pc2myfs("Text4.txt","Text4.txt");
	copy_pc2myfs("Text5.txt","Text5.txt");
	copy_pc2myfs("Text6.txt","Text6.txt");
	copy_pc2myfs("Text7.txt","Text7.txt");
	copy_pc2myfs("Text8.txt","Text8.txt");
	copy_pc2myfs("Text9.txt","Text9.txt");
	copy_pc2myfs("Text10.txt","Text10.txt");
	copy_pc2myfs("Text11.txt","Text11.txt");
	copy_pc2myfs("Text12.txt","Text12.txt");
	printf("1 - delete , 2 - list , 3 - quit\n");
	int choice;
	do
	{
		printf("Enter an option : ");
		scanf("%d",&choice);
		if(choice==1)
		{
			char str[20];
			printf("Enter the name of the file to delete : ");
			scanf("%s",str);
			rm_myfs(str);
		}
		if(choice==2)
		{
			ls_myfs();
		}
		if(choice==3)
		{
			break;
		}
	}while(1);
	return 0;
}
