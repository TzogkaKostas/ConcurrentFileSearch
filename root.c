#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <math.h>

#define BUFF_SIZE 100
#define SIZEofBUFF 20
#define SSizeofBUFF 6
#define READ 0
#define WRITE 1

int sig_count = 0;
typedef struct{
	long  	custid;
	char 	FirstName[SIZEofBUFF];
	char 	LastName[SIZEofBUFF];
	char	Street[SIZEofBUFF];
	int 	HouseID;
	char	City[SIZEofBUFF];
	char	postcode[SSizeofBUFF];
	float  	amount;
} MyRecord;
				//This struct is used for statistics reading
typedef struct{
	double mytime;
	double min_search;
	double max_search;
	double avg_search;
	double min_split;
	double max_split;
	double avg_split;
}stats_struct;

void rec_print(MyRecord rec) {
	printf("%ld %s %s  %s %d %s %s %-9.2f\n", 
		rec.custid, rec.LastName, rec.FirstName, 
		rec.Street, rec.HouseID, rec.City, rec.postcode, 
		rec.amount);
}

void sig_handler() {
	signal(SIGUSR2,sig_handler);
	sig_count++;
}

int main (int argc, char** argv) {
	MyRecord rec;
	stats_struct mystats;
	FILE *fp,*temp_fp;
	pid_t pid,pid2;
	int fd[2],fd2[2],r_bytes,i,h,status,numOfrecords,r_stats,num_searchers;
	long file_size;
	char height[10],filename[BUFF_SIZE],pattern[BUFF_SIZE],root_pid[BUFF_SIZE];
	char fdw_buff[10],fdw_buff2[10],buffer[BUFF_SIZE];
	char numRecs[BUFF_SIZE],l_searcher[10],numS_buffer[10],flag[2];
	clock_t start_t, end_t;
	double total_t;

	signal(SIGUSR2,sig_handler);
	start_t = clock();

	if (argc != 8 && argc != 7) {
		printf("Correct syntax:%s -h height -d Datafile -p Pattern -s\n", argv[0]);
		exit(-1);
	}
	else {
		strcpy(flag,"0");
		for (i=0; i<argc; i++) {
			if (strcmp(argv[i],"-h") == 0){
				h = atoi(argv[i+1]);
				if (h<1 || h>5 ) {
					printf("root_error:Height must be in range [1,5]\n");
					exit(-5);
				}
				strcpy(height,argv[i+1]);
				num_searchers = pow(2,h);
			}
			if (strcmp(argv[i],"-d") == 0){
				strcpy(filename,argv[i+1]);
			}
			if (strcmp(argv[i],"-p") == 0){
				strcpy(pattern,argv[i+1]);
			}
			if (strcmp(argv[i],"-s") == 0){
				strcpy(flag,"1");
			}
		}
	}
	
	fp = fopen ("myfile","w"); //this is used for the sort operation
	if (fp==NULL) {printf("root_error w_file open\n");exit(-4);}

	if(pipe(fd) == -1) {printf("root_error pipe\n");exit(-1);}
	if(pipe(fd2) == -1) {printf("root_error pipe\n");exit(-1);}
	if ((pid=fork()) == -1) {printf("root_fork1 error\n");exit(-2);}

	if (pid == 0) { 
		close(fd[READ]);
		close(fd2[READ]);
		sprintf(fdw_buff,"%d",fd[WRITE]);
		sprintf(fdw_buff2,"%d",fd2[WRITE]);
		sprintf(root_pid,"%d",getppid());
		sprintf(l_searcher,"%d",num_searchers-1);
		sprintf(numS_buffer,"%d",num_searchers);
		execl("splitter","splitter",filename
			,"0",l_searcher,pattern,height,fdw_buff,fdw_buff2,root_pid,numS_buffer,flag,NULL);
		printf("execl-r-splitter error\n");
		exit(-3);
	}
	else{

		close(fd[WRITE]);
			//reading of data
		while ( (r_bytes = read(fd[READ],&rec,sizeof(MyRecord))) > 0 ) {
			sprintf(buffer,"%ld %s %s  %s %d %s %s %f", 
				rec.custid, rec.LastName, rec.FirstName, 
				rec.Street, rec.HouseID, rec.City, rec.postcode, 
				rec.amount);
			//printf("root,%s\n",buffer );
			fprintf(fp,"%s\n",buffer);
		}
		if ((pid2=fork()) == -1) {printf("root_fork2 error\n");exit(-4);}
		if (pid2 == 0) {
			execlp("sort","sort","myfile",NULL);
			printf("root_error execlp-sort\n");
		}
		else {
			wait(&status);
				//reading of stats
			close(fd2[WRITE]);
			r_stats = read(fd2[READ],&mystats,sizeof(stats_struct));
			
			fclose(fp);
		}
	}
	wait(&status);
	end_t = clock();
	total_t = (double) (end_t - start_t) / CLOCKS_PER_SEC ;

	printf("Turnaround Time :%f\nMin searcher:%f, Max searcher%f, Avg searcher:%f\n"
		"Min splitter:%f, Max splitter:%f, Avg splitter:%f\n",mystats.mytime+total_t
		,mystats.min_search,mystats.max_search,mystats.avg_search
		,mystats.min_split,mystats.max_split,mystats.avg_split);
	printf("SIGUSR2 received: %d\n",sig_count);

	if (remove("myfile") != 0) {
		printf("root_error delete temp_file\n");
		exit(-5);
	}
	exit(0);
}