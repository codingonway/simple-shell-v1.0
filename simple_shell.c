#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define M 1024

void print_prompt();
int get_input(char *[]);
int do_cmd(char *[],int arg_count);

int main(void)
{
	char *arg[M];
	int  arg_count,ret;
	while(1)
	{
		ret  = 1;
		print_prompt();
		arg_count = get_input(arg);
		if(!arg_count) continue;
		ret = do_cmd(arg,arg_count);
		if (!ret)
			break;
	}
	return 0;
}

void print_prompt()
{
	char *username,hostname[M],path[M],*token,*dir;
	
	username = getlogin();
	gethostname(hostname,M-1);
	getcwd(path,M-1);
	token = strtok(path,"/");
	dir = token;
	if (token == NULL) dir="/";
	while((token = strtok(NULL,"/"))!=NULL)
			dir = token;
	if (!strcmp(dir,username))
		dir="~";
	
	printf("<%s@%s %s>",username,hostname,dir);
}


int get_input(char * arg[])
{
	int i,j,n;
	char ch,*buf,command[M];

	memset(command,0,sizeof(command));
	memset(arg,0,sizeof(arg));
	n=0;
	
	for (i=0;i<M;i++)
		{
			if((ch=getchar())=='\n')
				break;
			command[i]=ch;
		}
	for (j=0;j<i;j++)	
		if (command[j]!=' ') break;
	if (j == i) 
		return 0;
	buf = strtok(command," ");
	while(buf != NULL)
	{
		arg[n++] = buf;
		buf = strtok(NULL," ");
	}
	arg[n] = NULL;
	return n;
}
int do_cmd(char * arg[],int arg_count)
{
	int flag_in=0,flag_out=0,flag_pipe=0,flag_back=0;
	int i,j,fd_out,fd_in,status,fd2;
	char * outfile,*infile,*arg_pipe[M];
	pid_t  pid,pid2;

	if(!strcmp(arg[0],"cd"))	
		{
			if(arg_count == 1)	
				chdir(getenv("HOME"));
			else 
				{
					if(!strcmp(arg[1],"~"))
						chdir(getenv("HOME"));
					else{
						if(chdir(arg[1]) == -1)
							perror("cd ");
						return 1;
					 	}
				}
		}
	else if (!strcmp(arg[0],"kill")&&(arg_count==3))  // ***  output information  ***
		{
			kill(atoi(arg[2]),~atoi(arg[1])+1);
		}
	else if (!strcmp(arg[0],"exit") || !strcmp(arg[0],"logout"))
		return 0;	
	else
	{
		for(i=0;i<arg_count;i++){
			if(!strcmp(arg[i],"&")){
				flag_back=1;
				arg[i] = NULL;
				arg_count--;
			}
			if(!strcmp(arg[i],"<")){
				flag_in = 1;
				infile = arg[i+1];
				for (j=i;j<arg_count-2;j++)
					arg[j] = arg[j+2];
				arg[arg_count-2] = NULL;
				arg_count -=2;
				i--;
			}	
			if(!strcmp(arg[i],">")){
				flag_out = 1;
				outfile = arg[i+1];
				for(j=i;j<arg_count-2;j++)
					arg[j] = arg[j+2];
				arg[arg_count-2] = NULL;
				arg_count -=2;
				i--;
			}
		}
			
		for (i=0;i<arg_count;i++)	              
		{
			
			if(!strcmp(arg[i],"|")){
				flag_pipe = 1;
				for (j=0;j<i;j++)
					arg_pipe[j] = arg[j];
				arg_pipe[i] = NULL;
				for (j=0;j<arg_count-i-1;j++)
					arg[j]=arg[j+i+1];
				arg[arg_count-i-1] = NULL;
				arg_count = arg_count-i-1;
				break;
			}
		}
		if ((pid=fork())<0){
			printf("fork error\n");
			return 1;
		}
	if(pid == 0){
	
		if (flag_out)
		{
			fd_out = open(outfile,O_RDWR|O_CREAT|O_TRUNC,0644);
			dup2(fd_out,1);
		}
		if (flag_in)
		{
			fd_in = open(infile,O_RDONLY);
			dup2(fd_in,0);
		}  	
		if(flag_pipe)
		{
			pid2 = fork();
			if(!pid2){
				fd2  = open("/tmp/file",O_WRONLY|O_CREAT|O_TRUNC,0644);
				dup2(fd2,1);
				execvp(arg_pipe[0],arg_pipe);
				exit(0);
				}
			waitpid(pid2,&status,0);
			fd2 = open("/tmp/file",O_RDONLY);
			dup2(fd2,0);
		}
		execvp(arg[0],arg);
		remove("/tmp/file");
		exit(0);
		}
	else {
		if(flag_back)
			{
				printf("[process id %d]\n",pid);
				return 1;
			}
		else 
			{
				if (waitpid(pid,&status,0) == -1) { printf("waitpid error\n");}
			}
		}
		
	}	
	return 1;
}
