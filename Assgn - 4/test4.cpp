#include "myfs.h"
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/wait.h>
#include<semaphore.h>
#include<fcntl.h>

int main()
{
	int status,pid;
	if(create_myfs(10*1024*1024)==0)
		printf("Myfs created\n");
	else 
		return 0;
	if(mkdir_myfs("mydocs")==-1)
	{
		printf("Error in mkdir()\n");
		return 0;
	}
	if(chdir_myfs("mydocs")==-1)
	{
		printf("Error in chdir()\n");
		return 0;
	}
	if(mkdir_myfs("mytext")==-1)
	{
		printf("Error in mkdir()\n");
		return 0;
	}
	if(mkdir_myfs("mypapers")==-1)
	{
		printf("Error in mkdir()\n");
		return 0;
	}
	if(chdir_myfs("..")==-1)
	{
		printf("Error in chdir()\n");
		return 0;
	}
	if(mkdir_myfs("mycode")==-1)
	{
		printf("Error in mkdir()\n");
		return 0;
	}
	int shmid = shmget(IPC_PRIVATE,10*1024*1024,0777|IPC_CREAT);
	char * sh_myfs = (char*)shmat(shmid,0,0);
	memcpy(sh_myfs, pointer, 10*1024*1024);
	super_block = (super_block_struct *)sh_myfs;
	int myfs_size = super_block->total_size;
	for(int i=0;i<myfs_inodes;i++)
	{
	  inodelist[i] = (inode_struct *)(sh_myfs+sizeof(super_block_struct)+myfs_inodes/8+myfs_size/(256*8)+2+sizeof(inode_struct)*i);
	}
	data = (char *)inodelist[myfs_inodes-1]+sizeof(inode_struct);
	current_directory = 0;
	for(int i=0; i<100; i++)
		filetable[i].is_occupied = false;
	super_block->inode_bitmap = (unsigned int *)(sh_myfs+sizeof(super_block_struct));
	super_block->db_bitmap = (unsigned int *)(sh_myfs+sizeof(super_block_struct)+myfs_inodes/8+1);
	free(pointer);
	sem_t * sem = sem_open("psem", O_CREAT|O_EXCL, 0644, 1);
	pid = fork();
	if(pid==0)
	{
		sh_myfs = (char*)shmat(shmid,0,0);
		chdir_myfs("mydocs");
		chdir_myfs("mytext");
		sem_wait(sem);
		int fd = open_myfs("text1.txt",'w');
		sem_post(sem);
		char buf;
		for(int i=65; i<91; i++)
		{
			buf = (char)i;
			sem_wait(sem);
			write_myfs(fd, 1, &buf);
			sem_post(sem);
		}
		close_myfs(fd);
		sem_wait(sem);
		printf("Output of text1.txt : ");
		showfile_myfs("text1.txt");
		printf("\n");
		sem_post(sem);
		exit(0);
	}
	pid = fork();
	if(pid==0)
	{
		sh_myfs = (char*)shmat(shmid,0,0);
		chdir_myfs("mycode");
		sem_wait(sem);
		copy_pc2myfs("Text1.txt","Text1.txt");
		sem_post(sem);
		sem_wait(sem);
		showfile_myfs("Text1.txt");
		sem_post(sem);	
		exit(0);
	}
	wait(&status);
	wait(&status);
	shmdt(sh_myfs);
	shmctl (shmid, IPC_RMID, 0);
	sem_unlink("psem");
	sem_close(sem);
}
