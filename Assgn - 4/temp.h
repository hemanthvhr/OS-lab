#ifndef MY_HEADER_H
#define MY_HEADER_H

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>

#define myfs_inodes 96
#define block_size 256
#define fdmax 100
#define setbit(A,k)     ( A[(k/32)] |= (1 << (k%32)) )
#define clearbit(A,k)   ( A[(k/32)] &= ~(1 << (k%32)) )
#define testbit(A,k)    (( A[(k/32)] & (1 << (k%32)) ) >> k%32)

using namespace std;

struct super_block_struct
{
	unsigned int total_size;
	unsigned int max_no_of_inodes;
	unsigned int inodes_used=0;
	unsigned int max_no_dblocks;
	unsigned int dblocks_used=0;
	unsigned int* inode_bitmap;
	unsigned int* db_bitmap;
};

struct inode_struct
{
	bool file_type;
	unsigned int file_size=0;
	time_t lastmodified;
	time_t lastaccessed;
	unsigned int parent_inode_no;
	bool access_perm[9];
	char* data_pntrs[8]={NULL};
	int* indirect_pntr=NULL;
	int* dbl_ind_pntr=NULL;
};

struct file_table_struct
{
	bool is_occupied=false;
	int inode_no;
	char mode;
	unsigned int offset = 0;
};


char *pointer;
super_block_struct *super_block;
inode_struct* inodelist[myfs_inodes];
char* data;
int current_directory;
file_table_struct filetable[100];

extern int rmdir_myfs(char* dirname);


extern int create_myfs(int myfs_size)
{
	if(!(pointer =(char *)malloc(myfs_size*sizeof(char))))
		return -1;
	int no_of_blocks = myfs_size/block_size;

	//super block initialization
	super_block = (super_block_struct *)pointer;
	super_block->total_size=myfs_size;
	super_block->max_no_of_inodes=myfs_inodes;
	super_block->max_no_dblocks=(myfs_size-(sizeof(inode_struct)*myfs_inodes)-(myfs_size/(256*8)+sizeof(super_block_struct)+myfs_inodes/8+2))/256;
	super_block->inode_bitmap = (unsigned int *)(pointer+sizeof(super_block_struct));
	memset(super_block->inode_bitmap,0,myfs_inodes/8+1);
	super_block->db_bitmap = (unsigned int *)(pointer+sizeof(super_block_struct)+myfs_inodes/8+1);
	memset(super_block->db_bitmap,0,myfs_size/(256*8)+1);
	
	//inode list initialization
	for(int i=0;i<myfs_inodes;i++)
	{
	  inodelist[i] = (inode_struct *)(pointer+sizeof(super_block_struct)+myfs_inodes/8+myfs_size/(256*8)+2+sizeof(inode_struct)*i);
	  memset(inodelist[i]->access_perm,true,9*sizeof(bool));
	}
	
	//data initialization
	data = (char *)inodelist[myfs_inodes-1]+sizeof(inode_struct);

	//initialization of root directory
	super_block->inodes_used = 1;
	setbit(super_block->inode_bitmap,0);
	current_directory = 0;
	inodelist[0]->parent_inode_no=-1;
	inodelist[0]->file_type = true;
	inodelist[0]->file_size = 0;
	inodelist[0]->lastmodified = time(NULL);
	inodelist[0]->lastaccessed = time(NULL);
	for(int i=0;i<8;i++)
		inodelist[0]->data_pntrs[i]=NULL;
	inodelist[0]->indirect_pntr=NULL;
	inodelist[0]->dbl_ind_pntr=NULL;
	return 0;
}

extern int getandsetfreeinode(unsigned int A[])
{
	for(int i=0;i<myfs_inodes;i++)
	{
		if(testbit(A,i)==0)
		{
			setbit(A,i);
			super_block->inodes_used++;
			memset(inodelist[i],0,sizeof(inode_struct));
			for(int j=0;j<9;j++)
				inodelist[i]->access_perm[j]=true;
			return i;
		}
	}
	return -1;
}

extern int getandsetfreeblock(unsigned int A[])
{
	for(int i=0;i<super_block->max_no_dblocks;i++)
	{
		if(testbit(A,i)==0)
		{
			setbit(A,i);
			super_block->dblocks_used++;
			memset(data+i*256,0,256);
			return i;
		}
	}
	return -1;
}

int update_myfs(int filesize,char* dest,FILE* fp)
{
	int currfileinode = getandsetfreeinode(super_block->inode_bitmap);
	if(currfileinode == -1)
		return -1;
	inodelist[current_directory]->lastaccessed = time(NULL);
	inodelist[current_directory]->lastmodified = time(NULL);
	int j=0,k=0;
	while(inodelist[current_directory]->data_pntrs[j]!=NULL)
	{
		char z[30];
		memcpy(z,inodelist[current_directory]->data_pntrs[j]+32*k,30);
		if(strcmp(z,"")==0)
			break;
		else
		{
			k++;
			if(k==8)
			{
				k=0;
				j++;
			}
		}
		if(j==8)
			break;
	}
	if(j!=8)
	{
		if(inodelist[current_directory]->data_pntrs[j]==NULL)
		{
			int freedblock = getandsetfreeblock(super_block->db_bitmap);
			if(freedblock==-1)
				return -1;
			inodelist[current_directory]->data_pntrs[j] = (char *)(data+freedblock*256);
			memset(inodelist[current_directory]->data_pntrs[j],0,256);
			memcpy(inodelist[current_directory]->data_pntrs[j],dest,strlen(dest)+1);
			//printf("%d\n",inodelist[current_directory]->data_pntrs[j]);
			short int *num = (short int *)(inodelist[current_directory]->data_pntrs[j]+30);
			*num = currfileinode;
		}
		else
		{
			//printf("%d\n",inodelist[current_directory]->data_pntrs[j]+k*32);
			//printf("%s\n",dest);
			memcpy(inodelist[current_directory]->data_pntrs[j]+k*32,dest,strlen(dest)+1);
			short int *num = (short int *)(inodelist[current_directory]->data_pntrs[j]+k*32+30);
			*num = currfileinode;
		}
	}
	else
	{
		//implement indirect and double indirect pointers
	}
	
	inodelist[current_directory]->file_size += 32;
	
	inodelist[currfileinode]->file_type=false;
	inodelist[currfileinode]->file_size=filesize;
	inodelist[currfileinode]->lastmodified = time(NULL);
	inodelist[currfileinode]->lastaccessed = time(NULL);
	inodelist[currfileinode]->parent_inode_no = current_directory;
	int rem_size = filesize;
	int i=0;
	while(rem_size > 0)
	{
		int freedblock = getandsetfreeblock(super_block->db_bitmap);
		if(freedblock==-1)
			return -1;
		inodelist[currfileinode]->data_pntrs[i] = (char *)(data+freedblock*256);
		fread(inodelist[currfileinode]->data_pntrs[i],256,1,fp);
		//bzero(buff,256);
		//memcpy(buff,data+freedblock*256,256);
		//printf("%s",buff);
		i++;
		rem_size-=256;
		if(i==8)
			break;
	}
	if(rem_size > 0)
	{
		int indirectpointer = getandsetfreeblock(super_block->db_bitmap);
		if(indirectpointer==-1)
			return -1;
		inodelist[currfileinode]->indirect_pntr = (int *)(data+indirectpointer*256);
	}
	i=0;
	while(rem_size > 0)
	{
		int freedblock = getandsetfreeblock(super_block->db_bitmap);
		if(freedblock==-1)
			return -1;
		unsigned int *x = (unsigned int *)(inodelist[currfileinode]->indirect_pntr + i);
		*x = freedblock;
		fread(data+freedblock*256,256,1,fp);
		//bzero(buff,256);
		//memcpy(buff,data+freedblock*256,256);
		//printf("%s",buff);
		i++;
		rem_size-=256;
		if(i==64)
			break;
	}
	if(rem_size > 0)
	{
		int doubleindptr = getandsetfreeblock(super_block->db_bitmap);
		if(doubleindptr==-1)
			return -1;
		inodelist[currfileinode]->dbl_ind_pntr = (int *)(data+doubleindptr*256);
	}
	j=0;
	while(rem_size > 0)
	{
		if(rem_size > 0)
		{
			int indirectpointer = getandsetfreeblock(super_block->db_bitmap);
			if(indirectpointer==-1)
				return -1;
			inodelist[currfileinode]->indirect_pntr = (int *)(data+indirectpointer*256);
			unsigned int *x = (unsigned int *)(inodelist[currfileinode]->dbl_ind_pntr + j);
			*x = indirectpointer;
		}
		int i=0;
		while(rem_size > 0)
		{
			int freedblock = getandsetfreeblock(super_block->db_bitmap);
			if(freedblock==-1)
				return -1;
			unsigned int *x = (unsigned int *)(inodelist[currfileinode]->indirect_pntr + i);
			*x = freedblock;
		
			fread(data+freedblock*256,256,1,fp);
			i++;
			rem_size-=256;
			if(i==64)
				break;
		}
		j++;
		if(j==64)
			break;
	}
	super_block->inodes_used++;
	super_block->dblocks_used += filesize/256;
	return 0;
}

extern int searchinodewithfilename(char* filename)
{
	int temp=0;
	int i=0,j=0;
	while(temp < inodelist[current_directory]->file_size)
	{
		char tempfile[30];
		memcpy(tempfile,inodelist[current_directory]->data_pntrs[i]+32*j,30);
		tempfile[strlen(tempfile)]='\0';
		int inode=*(inodelist[current_directory]->data_pntrs[i]+32*j+30);
		if(!strcmp(filename,tempfile))
		{
			inodelist[current_directory]->lastaccessed = time(NULL);
			return inode;
		}
		else
		{
			if(strcmp(tempfile,""))
				temp+=32;
			j++;
			if(j==8)
			{
				j=0;
				i++;
			}
		}
	}
	// implement indirect pointer and double indirect pointer
	return -1;	
}


extern int copy_pc2myfs(char* source,char* dest)
{
	int temp = searchinodewithfilename(dest);
	if(temp!=-1)
	{
		printf("file with name %s already exists\n",dest);
		return -1;
	}
	FILE *fp;
	if(fp  = fopen(source,"rb"))
	{
		int filesize;
		fseek(fp, 0L, SEEK_END);
		filesize = ftell(fp);
		fseek(fp, 0L, SEEK_SET);
		if(update_myfs(filesize,dest,fp)==-1)
		{
			fclose(fp);
			return -1;
		}
	}
	else
	{
		printf("No file with %s name exists\n",source);
		return -1;
	}
	fclose(fp);
	return 0;
}

extern int ls_myfs()
{
	int temp = 0;
	int i=0,j=0;
	while(temp < inodelist[current_directory]->file_size)
	{
		char filename[30];
		memcpy(filename,inodelist[current_directory]->data_pntrs[i]+32*j,30);
		printf("Hello\n");
		j++;
		if(j==8)
		{
			i++;
			j=0;
		}
		if(strcmp(filename,"")!=0)
		{
			printf("%s\n",filename);
			temp+=32;
		}
	}
	return 0;
}


extern int showfile_myfs(char* filename)
{
	char buff[257];
	buff[256]=0;
	int currfileinode = searchinodewithfilename(filename);
	if(currfileinode==-1)
	{
		printf("No file with filename %s exists\n",filename);
		return -1;
	}
	inodelist[currfileinode]->lastaccessed = time(NULL);
	int rem_size = inodelist[currfileinode]->file_size;
	int i=0;
	while(rem_size > 0)
	{
		bzero(buff,256);
		memcpy(buff,inodelist[currfileinode]->data_pntrs[i],256);
		printf("%s",buff);
		i++;
		rem_size-=256;
		if(i==8)
			break;
	}
	i=0;
	while(rem_size > 0)
	{
		int datablock = *(inodelist[currfileinode]->indirect_pntr + i);
		bzero(buff,256);
		memcpy(buff,data+datablock*256,256);
		printf("%s",buff);
		i++;
		rem_size-=256;
		if(i==64)
			break;
	}
	int j=0;
	while(rem_size > 0)
	{
		int indirectblock = *(inodelist[currfileinode]->dbl_ind_pntr + j);
		int i=0;
		while(rem_size > 0)
		{
			int datablock = *(data+indirectblock*256+i);
			bzero(buff,256);
			memcpy(buff,data+datablock*256,256);
			printf("%s",buff);
			i++;
			rem_size-=256;
			if(i==64)
				break;
		}
		j++;
	}
	return 0;
}


int srch_del_myfs(char* filename)
{
	int temp=0;
	int i=0,j=0;
	while(temp<inodelist[current_directory]->file_size)
	{
		char tempfile[30];
		memcpy(tempfile,inodelist[current_directory]->data_pntrs[i]+32*j,30);
		tempfile[strlen(tempfile)]='\0';
		int inode=*(inodelist[current_directory]->data_pntrs[i]+32*j+30);
		if(!strcmp(filename,tempfile))
		{
			inodelist[current_directory]->file_size -= 32;
			inodelist[current_directory]->lastmodified = time(NULL);
			inodelist[current_directory]->lastaccessed = time(NULL);
			memset(inodelist[current_directory]->data_pntrs[i]+32*j,0,32);
			/*if(j==0)
			{
				clearbit(super_block->db_bitmap,((char *)inodelist[current_directory]->data_pntrs[i] - data)/256);
				inodelist[current_directory]->data_pntrs[i]=NULL;
			}*/
			// Implement removing of datablocks with no filenames and making datapointers null
			
			return inode;
		}
		if(strcmp(tempfile,""))
			temp+=32;
		j++;
		if(j==8)
		{
			i++;
			j=0;
		}
	}
	// implement indirect pointer and double indirect pointer
	return -1;	
}

extern int rm_myfs(char* filename)
{
	int file_inode;
	if((file_inode = srch_del_myfs(filename))==-1)
		return -1;
	else
	{
		//clearbit corresponding to the following inode
		//clearbits corresponding to the datablocks
		//update super block inodes_used--;  dblocks_used-=(filesize)/256;
		
		//update parent inode - done in srch_del_myfs() function
		
		//clean all the datablocks occupied by removed file
		//clean all the info in inodelist[file_inode]
		
		int temp=0;
		int i=0;
		while(temp<inodelist[file_inode]->file_size)
		{
			clearbit(super_block->db_bitmap,((char*)inodelist[file_inode]->data_pntrs[i]-data)/256);
			super_block->dblocks_used--;
			temp+=256;
			i++;
			if(i==8)
				break;
		}
		if((i==8)&&(temp<inodelist[file_inode]->file_size))
		{
			i=0;
			while(*(inodelist[file_inode]->indirect_pntr+i)!=0)
			{
				clearbit(super_block->db_bitmap,*(inodelist[file_inode]->indirect_pntr+i));
				super_block->dblocks_used--;
				temp+=256;
				i++;
				if(i==64)
					break;
			}
			clearbit(super_block->db_bitmap,((char*)inodelist[file_inode]->indirect_pntr-data)/256);
			super_block->dblocks_used--;
		}
		if((i==64)&&(temp<inodelist[file_inode]->file_size))
		{
			i=0;
			while(temp<inodelist[file_inode]->file_size)
			{
				int ind_block = *(inodelist[file_inode]->dbl_ind_pntr+i);
				int j=0;
				while(temp < inodelist[file_inode]->file_size)
				{
					clearbit(super_block->db_bitmap,*(data+ind_block*256+j));
					super_block->dblocks_used--;
					temp+=256;
					j++;
					if(j==64)
						break;
				}
				i++;
				if(i==64)
					break;
			}
		}
	}
	clearbit(super_block->inode_bitmap,file_inode);
	super_block->inodes_used--;
	return 0;	
}

extern int copy_myfs2pc(char* source,char* dest)
{
	char buff[257];
	buff[256]=0;
	FILE* fp;
	fp = fopen(dest,"rb");
	if(fp!=NULL)
	{
		printf("File with name %s already exists in pc\n",dest);
		fclose(fp);
		return -1;
	}
	fp = fopen(dest,"w");
	if(fp==NULL)
		return -1;
	int currfileinode = searchinodewithfilename(source);
	if(currfileinode==-1)
		return -1;
	inodelist[currfileinode]->lastaccessed = time(NULL);
	int rem_size = inodelist[currfileinode]->file_size;
	int i=0;
	while(rem_size > 0)
	{
		bzero(buff,256);
		memcpy(buff,inodelist[currfileinode]->data_pntrs[i],256);
		if(rem_size<256)
			fwrite(buff,rem_size,1,fp);
		else
			fwrite(buff,256,1,fp);
		//printf("%s",buff);
		i++;
		rem_size-=256;
		if(i==8)
			break;
	}
	i=0;
	while(rem_size > 0)
	{
		int datablock = *(inodelist[currfileinode]->indirect_pntr + i);
		bzero(buff,256);
		memcpy(buff,data+datablock*256,256);
		if(rem_size<256)
			fwrite(buff,rem_size,1,fp);
		else
			fwrite(buff,256,1,fp);
		//printf("%s",buff);
		i++;
		rem_size-=256;
		if(i==64)
			break;
	}
	int j=0;
	while(rem_size > 0)
	{
		int indirectblock = *(inodelist[currfileinode]->dbl_ind_pntr + j);
		int i=0;
		while(rem_size > 0)
		{
			int datablock = *(data+indirectblock*256+i);
			bzero(buff,256);
			memcpy(buff,data+datablock*256,256);
			if(rem_size<256)
				fwrite(buff,rem_size,1,fp);
			else
				fwrite(buff,256,1,fp);
			//printf("%s",buff);
			i++;
			rem_size-=256;
			if(i==64)
				break;
		}
		j++;
	}
	fprintf(fp,"\0");
	fclose(fp);
	return 0;
}

extern int mkdir_myfs(char* dirname)
{
	if(searchinodewithfilename(dirname)!=-1)
	{
		printf("Directory with name %s already exists\n",dirname);
		return -1;
	}
	int currfileinode = getandsetfreeinode(super_block->inode_bitmap);
	if(currfileinode == -1)
		return -1;
	inodelist[current_directory]->lastaccessed = time(NULL);
	inodelist[current_directory]->lastmodified = time(NULL);
	int j=0,k=0;
	while(inodelist[current_directory]->data_pntrs[j]!=NULL)
	{
		char z[30];
		memcpy(z,inodelist[current_directory]->data_pntrs[j]+32*k,30);
		if(strcmp(z,"")==0)
			break;
		else
		{
			k++;
			if(k==8)
			{
				k=0;
				j++;
			}
		}
		if(j==8)
			break;
	}
	if(j!=8)
	{
		if(inodelist[current_directory]->data_pntrs[j]==NULL)
		{
			int freedblock = getandsetfreeblock(super_block->db_bitmap);
			if(freedblock==-1)
				return -1;
			inodelist[current_directory]->data_pntrs[j] = (char *)(data+freedblock*256);
			memset(inodelist[current_directory]->data_pntrs[j],0,256);
			memcpy(inodelist[current_directory]->data_pntrs[j],dirname,strlen(dirname)+1);
			//printf("%d\n",inodelist[current_directory]->data_pntrs[j]);
			short int *num = (short int *)(inodelist[current_directory]->data_pntrs[j]+30);
			*num = currfileinode;
		}
		else
		{
			//printf("%d\n",inodelist[current_directory]->data_pntrs[j]+k*32);
			//printf("%s\n",dest);
			memcpy(inodelist[current_directory]->data_pntrs[j]+k*32,dirname,strlen(dirname)+1);
			short int *num = (short int *)(inodelist[current_directory]->data_pntrs[j]+k*32+30);
			*num = currfileinode;
		}
	}
	else
	{
		//implement indirect and double indirect pointers
	}
	inodelist[current_directory]->file_size += 32;

	inodelist[currfileinode]->file_type=true;
	inodelist[currfileinode]->file_size=0;
	inodelist[currfileinode]->lastmodified = time(NULL);
	inodelist[currfileinode]->lastaccessed = time(NULL);
	for(int i=0;i<8;i++)
		inodelist[currfileinode]->data_pntrs[i]=NULL;
	inodelist[currfileinode]->indirect_pntr = NULL;
	inodelist[currfileinode]->dbl_ind_pntr = NULL;
	inodelist[currfileinode]->parent_inode_no = current_directory;
	return 0;
}

extern int chdir_myfs (char *dirname)
{
	int dirinode;
	if(strcmp(dirname,"..")==0)
	{
		current_directory = inodelist[current_directory]->parent_inode_no;
		return 0;
	}
	if((dirinode = searchinodewithfilename(dirname))==-1)
	{
		printf("Directory with name %s doesnot exists\n",dirname);
		return -1;
	}
	inodelist[current_directory]->lastaccessed = time(NULL);
	current_directory = dirinode;
}

extern int removedir(char* dirname,int dirinode)
{
	int actual_dir = current_directory;
	current_directory = dirinode;	
	int temp=0;
	int i=0,j=0;
	while(temp<inodelist[current_directory]->file_size)
	{
		char tempfile[30];
		memcpy(tempfile,inodelist[current_directory]->data_pntrs[i]+32*j,30);
		tempfile[strlen(tempfile)]='\0';
		int inode=*(inodelist[current_directory]->data_pntrs[i]+32*j+30);
		if(strcmp(tempfile,""))
		{
			inodelist[current_directory]->file_size -= 32;
			memset(inodelist[current_directory]->data_pntrs[i]+32*j,0,32);
			if(inodelist[inode]->file_type == true)
			{
				rmdir_myfs(tempfile);
			}
			else
			{
				rm_myfs(tempfile);
			}
		}
		if(strcmp(tempfile,""))
			temp+=32;
		j++;
		if(j==8)
		{
			i++;
			j=0;
		}
	}
	i=0;
	while(inodelist[current_directory]->data_pntrs[i]!=NULL)
	{
		clearbit(super_block->db_bitmap,(inodelist[current_directory]->data_pntrs[i]-data)/256);
		super_block->dblocks_used--;
		i++;
		if(i==8)
			break;
	}
	clearbit(super_block->inode_bitmap,current_directory);
	super_block->inodes_used--;
	current_directory = actual_dir;
}

extern int rmdir_myfs(char* dirname)
{
	int dirinode;
	if((dirinode = searchinodewithfilename(dirname))==-1)
	{
		printf("Directory with name %s doesnot exists\n",dirname);
		return -1;
	}
	if(inodelist[dirinode]->file_type == false)
	{
		printf("%s is not a directory\n",dirname);
		return -1;
	}
	removedir(dirname,dirinode);
	int i=0;
	int j=0,temp=0;
	
	while(temp < inodelist[current_directory]->file_size)
	{
		char tempfile[30];
		memcpy(tempfile,inodelist[current_directory]->data_pntrs[i]+32*j,30);
		tempfile[strlen(tempfile)]='\0';
		int inode=*(inodelist[current_directory]->data_pntrs[i]+32*j+30);
		if(strcmp(tempfile,dirname)==0)
		{
			inodelist[current_directory]->file_size -= 32;
			memset(inodelist[current_directory]->data_pntrs[i]+32*j,0,32);
		}
		if(strcmp(tempfile,""))
			temp+=32;
		j++;
		if(j==8)
		{
			i++;
			j=0;
		}
	}
}

extern int getfirstunoccfd(int fileinode,char x)
{
	for(int i=0;i<100;i++)
	{
		if(filetable[i].is_occupied == false)
		{
			filetable[i].is_occupied = true;
			filetable[i].inode_no = fileinode;
			filetable[i].mode = x;
			filetable[i].offset = 0;
			return i;
		}
	}
	return -1;
}
extern int restore_myfs(char* dumpfile){}
extern int createfile(char* filename)
{
	int currfileinode = getandsetfreeinode(super_block->inode_bitmap);
	if(currfileinode == -1)
		return -1;
	inodelist[current_directory]->lastaccessed = time(NULL);
	inodelist[current_directory]->lastmodified = time(NULL);
	int j=0,k=0;
	while(inodelist[current_directory]->data_pntrs[j]!=NULL)
	{
		char z[30];
		memcpy(z,inodelist[current_directory]->data_pntrs[j]+32*k,30);
		if(strcmp(z,"")==0)
			break;
		else
		{
			k++;
			if(k==8)
			{
				k=0;
				j++;
			}
		}
		if(j==8)
			break;
	}
	if(j!=8)
	{
		if(inodelist[current_directory]->data_pntrs[j]==NULL)
		{
			int freedblock = getandsetfreeblock(super_block->db_bitmap);
			if(freedblock==-1)
				return -1;
			inodelist[current_directory]->data_pntrs[j] = (char *)(data+freedblock*256);
			memset(inodelist[current_directory]->data_pntrs[j],0,256);
			memcpy(inodelist[current_directory]->data_pntrs[j],filename,strlen(filename)+1);
			short int *num = (short int *)(inodelist[current_directory]->data_pntrs[j]+30);
			*num = currfileinode;
		}
		else
		{
			memcpy(inodelist[current_directory]->data_pntrs[j]+k*32,filename,strlen(filename)+1);
			short int *num = (short int *)(inodelist[current_directory]->data_pntrs[j]+k*32+30);
			*num = currfileinode;
		}
	}
	else
	{
		//implement indirect and double indirect pointers
	}
	
	inodelist[current_directory]->file_size += 32;

	inodelist[currfileinode]->file_type=false;
	inodelist[currfileinode]->file_size=0;
	inodelist[currfileinode]->lastmodified = time(NULL);
	inodelist[currfileinode]->lastaccessed = time(NULL);
	inodelist[currfileinode]->parent_inode_no = current_directory;
	return currfileinode;
}

extern int open_myfs(char* filename,char mode)
{
	int fileinode;
	int fd;
	if((mode == 'r')||(mode == 'w'))
	{
		if(mode == 'r')
		{
			if((fileinode = searchinodewithfilename(filename))==-1)
			{
				printf("file with name %s doesnot exists\n",filename);
				return -1;
			}
			if(inodelist[fileinode]->file_type == true)
			{
				printf("%s is a directory. Cannot be opened\n",filename);
				return -1;
			}
			if(inodelist[fileinode]->access_perm[0]==inodelist[fileinode]->access_perm[3]==inodelist[fileinode]->access_perm[6]==1)
			{
				
				fd = getfirstunoccfd(fileinode,mode);
			}
			else
			{
				printf("Access denied to open file\n");
				printf("Access permissions : ");
				for(int i=0;i<9;i++)
					printf("%d",inodelist[fileinode]->access_perm[i]);
				return -1;
			}
		}
		else
		{
			if((fileinode = searchinodewithfilename(filename))==-1)
			{
				
				fileinode = createfile(filename);
				fd = getfirstunoccfd(fileinode,mode);
				return fd;
			}
			else
			{
				if(inodelist[fileinode]->access_perm[1]==inodelist[fileinode]->access_perm[4]==inodelist[fileinode]->access_perm[7]==1)
				{
					rm_myfs(filename);
					fileinode = createfile(filename);
					fd = getfirstunoccfd(fileinode,mode);
					return fd;
				}
				else
				{
					printf("Access denied to open file\n");
					return -1;
				}
				
			}
		}
	}
	else
	{
		printf("Inappropriate mode chosen. Choose:'r' or 'w'\n");
		return -1;
	}
	return fd;
}

extern int close_myfs(int fd)
{
	if(fd < 0 || fd >= fdmax)
	{
		printf("Invalid file descriptor\n");
		return -1;
	}
	if(filetable[fd].is_occupied == false)
	{
		printf("Invalid file descriptor\n");
		return -1;
	}
	
	filetable[fd].is_occupied = false;
	return 0;
}

int read_myfs (int fd, int nbytes, char *buff)
{
	if(nbytes < 0)
	{
		printf("Choose appropriate bytes to read\n");
		return -1;
	}
	if(fd < 0 || fd >= fdmax)
	{
		printf("Invalid file descriptor\n");
		return -1;
	}
	if(filetable[fd].is_occupied == false)
	{
		printf("Invalid file descriptor\n");
		return -1;
	}
	if(filetable[fd].mode != 'r')
	{
		printf("file opened for writing.Cannot read in it\n");
		return -1;
	}
	if(filetable[fd].offset >= inodelist[filetable[fd].inode_no]->file_size)
	{
		//printf("file reached end of the file\n");
		return 0;
	}

	int i,j=0,z=0;
	int * b_num;
	int * sip;
	int i_num = filetable[fd].inode_no;
	int block_number = filetable[fd].offset/(256);
	int block_offset = filetable[fd].offset%(256);
	if(block_number < 8)
	{
		while(block_number<8)
		{
			while(block_offset<256)
			{
				if(j==nbytes) return nbytes;
				if(filetable[fd].offset == inodelist[i_num]->file_size) return j;
				memcpy(buff+j, inodelist[i_num]->data_pntrs[block_number] + block_offset, 1);
				block_offset++;
				j++;
				filetable[fd].offset++;
			}
			block_offset = 0;
			block_number++;
		}
	}
	if(j==nbytes) return nbytes;
	if(filetable[fd].offset == inodelist[i_num]->file_size) return j;
	
	block_offset = filetable[fd].offset%(256);
	if(block_number>=8 && block_number<72)
	{
		b_num = (int*)(inodelist[i_num]->indirect_pntr);
		for(i=block_number-8;i<256/4;i++,block_number++)
		{
			for(;block_offset<256; block_offset++,j++,filetable[fd].offset++)
			{
				if(j==nbytes) return nbytes;
				if(filetable[fd].offset == inodelist[i_num]->file_size) return j;
				memcpy(buff+j, data + *(b_num+i)*256 + block_offset, 1);
			}
			block_offset = 0;
		}
	}
	if(j==nbytes) return nbytes;
	if(filetable[fd].offset == inodelist[i_num]->file_size) return j;
	
	block_offset = filetable[fd].offset%(256);
	if(block_number>=72 && block_number<4168)
	{
		block_number-=72;
		sip = (int*)(inodelist[i_num]->dbl_ind_pntr);
		for(z=block_number/(256/4); z<256/4; z++)
		{
			b_num = (int*)(data + *(sip+z)*256);
			for(i=block_number%(256/4); i<256/4; i++,block_number++)
			{
				for(;block_offset<256; block_offset++,j++,filetable[fd].offset++)
				{
					if(j==nbytes) return nbytes;
					if(filetable[fd].offset == inodelist[i_num]->file_size) return j;
					memcpy(buff+j, data + *(b_num+i)*256 + block_offset, 1);
				}
				block_offset = 0;
			}
		}
	}
	if(j==nbytes) return nbytes;
	if(filetable[fd].offset == inodelist[i_num]->file_size) return j;
	return -1;

}

extern int write_myfs(int fd, int nbytes, char* buff)
{
	if(fd < 0 || fd >= fdmax)
	{
		printf("Invalid file descriptor\n");
		return -1;
	}
	if(filetable[fd].is_occupied == false)
	{
		printf("Invalid file descriptor\n");
		return -1;
	}
	if(filetable[fd].mode != 'w')
	{
		printf("file opened for reading.Cannot write in it\n");
		return -1;
	}
	
	int i,j=0,z=0;
	int * b_num;
	int * sip;
	int temp;
	int i_num = filetable[fd].inode_no;
	int file_inode = i_num;
	int block_number = filetable[fd].offset/(256);
	int block_offset = filetable[fd].offset%(256);
	
	if(block_number < 8)
	{
		for(;block_number<8;block_number++)
		{
			if(inodelist[i_num]->data_pntrs[block_number]==NULL)
			{
				int freedblock =getandsetfreeblock(super_block->db_bitmap);
				inodelist[file_inode]->data_pntrs[block_number]=(data+freedblock*256);
			}
			for(; block_offset<256; block_offset++,j++,filetable[fd].offset++)
			{
				if(j==nbytes)
				{
					return nbytes;
				}
				memcpy(inodelist[file_inode]->data_pntrs[block_number] + block_offset, buff+j, 1);
				inodelist[i_num]->file_size++;
				
			}
			block_offset = 0;
		}
	}
	if(j==nbytes)
	{
		return nbytes;
	}
	if(inodelist[i_num]->indirect_pntr==NULL)
	{
		int freedblock = getandsetfreeblock(super_block->db_bitmap);
		inodelist[file_inode]->indirect_pntr=(int *)(data+freedblock*256);
	}
	block_offset = filetable[fd].offset%(256);
	if(block_number>=8 && block_number<72)
	{
		b_num = (int*)(inodelist[file_inode]->indirect_pntr);
		for(i=block_number-8;i<256/4;i++,block_number++)
		{
			if(*(b_num+i)==0)
			{
				int freedblock = getandsetfreeblock(super_block->db_bitmap);
				*(b_num+i)=freedblock;
			}
			for(;block_offset<256; block_offset++,j++,filetable[fd].offset++)
			{
				if(j==nbytes) return nbytes;
				memcpy(data + (*(b_num+i))*256 + block_offset, buff+j, 1);
				inodelist[i_num]->file_size++;
			}
			block_offset = 0;
		}
	}
	
	if(j==nbytes) 
		return nbytes;
		
	if(inodelist[i_num]->dbl_ind_pntr==NULL)
	{
		int freedblock =getandsetfreeblock(super_block->db_bitmap);
		inodelist[i_num]->dbl_ind_pntr=(int *)(data+freedblock*256);
	}

	block_offset = filetable[fd].offset%(256);
	if(block_number>=72 && block_number<4168)
	{
		block_number-=72;
		sip = (int *)inodelist[i_num]->dbl_ind_pntr;
		for(z=block_number/(256/4); z<256/4; z++)
		{
			if(*(sip+z)==0)
			{
				int freedblock =getandsetfreeblock(super_block->db_bitmap);
				*(sip+z) = freedblock;
			}
			b_num = (int*)(data + *(sip+z)*256);
			for(i=block_number%(256/4); i<256/4; i++,block_number++)
			{
				if(*(b_num+i)==0)
				{
					int freedblock =getandsetfreeblock(super_block->db_bitmap);
					*(b_num+i) = freedblock;
				}
				for(;block_offset<256; block_offset++,j++,filetable[fd].offset++)
				{
					if(j==nbytes) return nbytes;
					memcpy(data + *(b_num+i)*256 + block_offset, buff+j, 1);
					inodelist[i_num]->file_size++;
				}
				block_offset = 0;
			}
		}
	}
	
	
	if(j==nbytes) return nbytes;
	printf("write_myfs error\n");
	return -1;
}


extern int eof_myfs (int fd)
{
	if(fd < 0 || fd >= fdmax)
	{
		printf("Invalid file descriptor\n");
		return -1;
	}
	if(filetable[fd].is_occupied == false)
	{
		printf("Invalid file descriptor\n");
		return -1;
	}
	if(filetable[fd].offset < inodelist[filetable[fd].inode_no]->file_size)
		return 0;
	else 
		return 1;
}

extern int dump_myfs (char *dumpfile)
{
	FILE* fp = fopen(dumpfile,"wb");
	fwrite(&(super_block->total_size), 1, 4, fp);
	int n = fwrite(pointer, 1, super_block->total_size, fp);
	fclose(fp);
	if(n == super_block->total_size)
		return 0;
	else
		return -1;
}

extern int restore_myfs (char dumpfile)
{
	char* dumfile;
	int myfs_size;
	FILE* fp = fopen(dumfile,"r");
	fread(&myfs_size, 1, 4, fp);
	if(!(pointer =(char *)malloc(myfs_size*sizeof(char))))
		return -1;
	fread(pointer, 1, myfs_size, fp);
	super_block = (super_block_struct *)pointer;
	for(int i=0;i<myfs_inodes;i++)
	{
		inodelist[i] = (inode_struct *)(pointer+sizeof(super_block_struct)+myfs_inodes/8+myfs_size/(256*8)+sizeof(inode_struct)*i);
	}
	current_directory = 0;
	for(int i=0; i<100; i++)
		filetable[i].is_occupied = false;
	super_block->inode_bitmap = (unsigned int *)(pointer+sizeof(super_block_struct));
	super_block->db_bitmap = (unsigned int *)(pointer+sizeof(super_block_struct)+myfs_inodes/8+1);
	return 0;
}

extern int status_myfs ()
{
	int used_space;
	used_space = (data-pointer)-(super_block->max_no_of_inodes-super_block->inodes_used)*sizeof(inode_struct)+(super_block->dblocks_used)*256;
	printf("MRFS(total size) : %d\n",super_block->total_size);
	printf("MRFS(Used space) : %d\n",used_space);
	printf("MRFS(Free space) : %d\n",super_block->total_size-used_space);
	printf("MRFS(File count) : %d\n",super_block->inodes_used);
	return 0;
}

extern int chmod_myfs(char* filename, int mode)
{
	int inode = searchinodewithfilename(filename);
	if(inode == -1)
	{
		printf("No file with name %s\n",filename);
		return -1;
	}
	for(int i=9;i>0;i--)
	{
		inodelist[inode]->access_perm[i] = mode%10;
		mode/=10;
	}
}


#endif




