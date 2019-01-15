#include "myfs.h"

void printinodevalues(char* name)
{
	int inode = searchinodewithfilename(name);
	printf("%d %d %d %d\n",inode,inodelist[inode]->file_type,inodelist[inode]->file_size,inodelist[inode]->parent_inode_no);
}

void printdbbitmap()
{
	for(int i=0;i<75;i++)
		printf("%d ",testbit(super_block->db_bitmap,i));
}

void printinodebitmap()
{
	for(int i=0;i<75;i++)
		printf("%d ",testbit(super_block->inode_bitmap,i));
}

void printfd(int fd)
{
	printf("%d %d %c %d\n",filetable[fd].is_occupied,filetable[fd].inode_no,filetable[fd].mode,filetable[fd].offset);
}

int main()
{
	if(create_myfs(10000000)==-1)
		printf("Error creating file system\n");
	mkdir_myfs("Directory1");	
	chdir_myfs("Directory1");
	mkdir_myfs("Directory2");
	mkdir_myfs("Directory3");
	copy_pc2myfs("Text1.txt","Text1.txt");
	rmdir_myfs("Directory2");
	rmdir_myfs("Directory3");
	
	int fd = open_myfs("Text1.txt",'r');
	char buff[1000];
	buff[999]=0;
	read_myfs(fd,999,buff);
	close_myfs(fd);
	ls_myfs();
	fd = open_myfs("Text2.txt",'w');
	write_myfs(fd,999,buff);
	close_myfs(fd);
	showfile_myfs("Text2.txt");
	copy_myfs2pc("Text2.txt","text1.txt");
	status_myfs();
}
