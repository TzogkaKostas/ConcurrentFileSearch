#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define BUFF_SIZE 100
#define SIZEofBUFF 20
#define SSizeofBUFF 6
#define READ 0
#define WRITE 1

double max(double x,double y) {return x>y? x:y;}
double min(double x,double y) {return x<y? x:y;}

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

typedef struct{
	double mytime;
	double min_search;
	double max_search;
	double avg_search;
	double min_split;
	double max_split;
	double avg_split;
}stats_struct;

int main (int argc, char** argv) {
	MyRecord rec1,rec2;
	stats_struct stats1,stats2,mystats;
	pid_t pid1,pid2;
	int status,s,e,d,e1,s2,fd1[2],fd2[2],fd3[2],fd4[2],fd_w,fd_w2;
	int r_bytes1,r_bytes2,r_stats1,r_stats2;
	char depth[10],filename[BUFF_SIZE],pattern[BUFF_SIZE],root_pid[BUFF_SIZE];
	char start[10],end[10],end1[10],start2[10],b1[10],b2[10],b3[10],b4[10];
	char fd_write[10],fd_write2[10],num_searchers[10],flag[2];
	clock_t start_t, end_t;
	double total_t;

	start_t = clock();
	if (argc != 11) {
		printf("Correct syntax:./%s file start end pattern depth root_pid\n", argv[0]);
		exit(-1);
	}
	else {
		strcpy(filename,argv[1]);
		strcpy(start,argv[2]);
		s = atoi(start);	//This is the first searcher
		strcpy(end,argv[3]);
		e = atoi(end);		//This is the last searcher
		strcpy(pattern,argv[4]);
		d = atoi(argv[5]);	//This is the current depth
		strcpy(fd_write,argv[6]);
		strcpy(fd_write2,argv[7]);
		fd_w = atoi(fd_write);	//Write end of parent's data pipe
		fd_w2 = atoi(fd_write2);  //Write end of parent's stats pipe
		strcpy(root_pid,argv[8]);
		strcpy(num_searchers,argv[9]);
		strcpy(flag,argv[10]); //flag for -s option
	}

	if (d == 0) {
		execl("leaf","leaf",filename
			,start,pattern,fd_write,fd_write2,root_pid,num_searchers,flag,NULL);
		printf("split_error execl-leaf\n");
		exit(-2);
	}
	if ( pipe(fd1) == -1 ) {printf("split_error pipe1\n"); exit(-3);}
	if ( pipe(fd2) == -1 ) {printf("split_error pipe2\n"); exit(-3);}
	if ( pipe(fd3) == -1 ) {printf("split_error pipe3\n"); exit(-4);}
	if ( pipe(fd4) == -1 ) {printf("split_error pipe4\n"); exit(-4);}
			//range of the searchers
	if (e-s > 1 ) {
		e1 = ceil((e+s)/2);
		sprintf(end1,"%d",e1);
		s2 = ceil((e+s)/2) + 1;
		sprintf(start2,"%d",s2);
	}
	else {
		strcpy(end1,start);
		strcpy(start2,end);
	}

	d = d-1;
	sprintf(depth,"%d",d);

	if ( (pid1 = fork()) == -1 ) {printf("split_error Fork1\n");exit(-5);}
	if (pid1 > 0) {
		close(fd1[WRITE]); //reading of first child's data
		while( (r_bytes1 = read(fd1[READ],&rec1,sizeof(MyRecord)))>0 ) {
			write(fd_w,&rec1,sizeof(MyRecord)); //and writing it to the parent
		}
		if (r_bytes1 ==-1) {printf("splitter_error:Read1 failed\n");}

		close(fd2[WRITE]); //reading of first child's statistics
		r_stats1 = read(fd2[READ],&stats1,sizeof(stats_struct));
		if (r_stats1 ==-1) {printf("splitter_error:Read2 failed\n");}
		
		if ( (pid2 = fork()) == -1 ) {printf("split_error Fork2\n");exit(-6);}
		if (pid2 > 0) {
			close(fd3[WRITE]); //reading of second child's data
			while ( (r_bytes2 = read(fd3[READ],&rec2,sizeof(MyRecord))) >0 ) {
				write(fd_w,&rec2,sizeof(MyRecord)); //and writing it to the parent
			}
			if (r_bytes2 ==-1) {printf("splitter_error:Read3 failed\n");}

			close(fd4[WRITE]);	//reading of second child's statistics
			r_stats2 = read(fd4[READ],&stats2,sizeof(stats_struct));
			if (r_stats2 ==-1) {printf("splitter_error:Read4 failed\n");}

			close(fd_w);
		}
		else {
			close(fd3[READ]);
			close(fd4[READ]);
			sprintf(b3,"%d",fd3[WRITE]);
			sprintf(b4,"%d",fd4[WRITE]);
			execl("splitter","splitter",filename
				,start2,end,pattern,depth,b3,b4,root_pid,num_searchers,flag,NULL);
			printf("split_error execl-splitter\n");
			exit(-7);
		}
	}
	else {
		close(fd1[READ]);
		close(fd2[READ]);
		sprintf(b1,"%d",fd1[WRITE]);
		sprintf(b2,"%d",fd2[WRITE]);
		execl("splitter","splitter",filename
			,start,end1,pattern,depth,b1,b2,root_pid,num_searchers,flag,NULL);
		printf("split_error execl-splitter\n");
		exit(-8);
	}
	wait(&status);
	wait(&status);

	end_t = clock();
	total_t = (double) (end_t - start_t) / CLOCKS_PER_SEC ;
	mystats.mytime = total_t + stats1.mytime + stats2.mytime;
	if (d+1 == 1) { //height=1
		mystats.min_split = mystats.mytime;
		mystats.max_split = mystats.mytime;
		mystats.avg_split = mystats.mytime;
	}
	else {
		mystats.min_split = min(stats1.min_split,stats2.min_split);
		mystats.min_split = min(mystats.min_split,mystats.mytime);
		mystats.max_split = max(stats1.max_split,stats2.max_split);
		mystats.max_split = max(mystats.max_split,mystats.mytime);
		mystats.avg_split = (stats1.avg_split+stats2.avg_split+mystats.mytime)/3.0;
	}
	mystats.min_search = min(stats1.min_search,stats2.min_search);
	mystats.max_search = max(stats1.max_search,stats2.max_search);
	mystats.avg_search = (stats1.avg_search+stats2.avg_search)/2.0;

	write(fd_w2,&mystats,sizeof(stats_struct)); //writing stats to parent
	close(fd_w2);
	exit(0);
}