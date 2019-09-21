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

int is_there(MyRecord rec,char* s) {
	char temp[SIZEofBUFF],b1[SIZEofBUFF],b2[SIZEofBUFF],b3[SIZEofBUFF];

	sprintf(b1,"%ld",rec.custid);	
	sprintf(b2,"%d",rec.HouseID);
	sprintf(b3,"%f",rec.amount);
	
	if (strstr(rec.LastName,s)!=NULL||strstr(rec.FirstName,s)!=NULL
		||strstr(rec.Street,s)!=NULL||strstr(rec.City,s)!=NULL
		||strstr(rec.postcode,s)!=NULL||strstr(b1,s)!=NULL
		||strstr(b2,s)!=NULL||strstr(b3,s)!=NULL)
	{
		return 1;
	}
	return 0;
}

void rec_print(MyRecord rec) {
	printf("%ld %s %s  %s %d %s %s %-9.2f\n", 
		rec.custid, rec.LastName, rec.FirstName, 
		rec.Street, rec.HouseID, rec.City, rec.postcode, 
		rec.amount);
}

typedef struct{
	double mytime;
	double min_search;
	double max_search;
	double avg_search;
	double min_split;
	double max_split;
	double avg_split;
}stats_struct;

void set_stats(stats_struct * s,double t) {
	s->mytime = t;
	s->min_search = t;
	s->max_search = t;
	s->avg_search = t;
}

int share_func(int n,int k,int num_s) {
	int i,sum=0;
	for (i=1; i<=n-1; i++) {
		sum += k*i/(num_s*(num_s+1)/2);
	}
	return sum;
}


int main (int argc, char** argv) {
	FILE *fpb,*temp_fp;
	MyRecord rec;
	stats_struct mystats;
	long file_size;
	int i,start,end,pid,fd_w,fd_w2,num_searchers,searcher,k,sum,prev,curr_share,flag;
	char filename[SIZEofBUFF],pattern[SIZEofBUFF],fd_write[10],fd_write2[10];
	char root_pid[BUFF_SIZE];
	clock_t start_t, end_t;
	double total_t;

	start_t = clock();

	if (argc!=9) {
		printf("Correct syntax:%s file start end pattern\n", argv[0]);
		exit(-1);
	}
	else {
		strcpy(filename,argv[1]);
		searcher = atoi(argv[2]);
		strcpy(pattern,argv[3]);
		strcpy(fd_write,argv[4]);
		strcpy(fd_write2,argv[5]);
		fd_w = atoi(fd_write);
		fd_w2 = atoi(fd_write2);
		strcpy(root_pid,argv[6]);
		num_searchers = atoi(argv[7]);
		flag = atoi(argv[8]); //used to know if -s option was used
	}
	fpb = fopen (filename,"rb"); //filename
	if (fpb==NULL) {printf("leaf_error: cant open bin file \n");exit(-2);}
	
	fseek (fpb , 0 , SEEK_END);
	file_size = ftell (fpb);
	rewind(fpb);
	k = (int) file_size/sizeof(MyRecord); //number of records

	searcher++; //we add 1,because the range of the searchers is [0,n-1]

	if (flag == 1) { //with -s option
		prev = share_func(searcher,k,num_searchers);
		curr_share = k*searcher/(num_searchers*(num_searchers+1)/2);
			//in case share is zero,this leaf doesnt do any work
		start = prev + 1;
		end = start + curr_share -1 ;

		start--;
		end--;
		if (searcher == num_searchers) {end=k-1;} //last searcher takes remaining records
	}
	else {	//without -s option
		searcher--;
		curr_share = k / num_searchers;
		start = curr_share*searcher;
		end = start + curr_share - 1;
		if (searcher == num_searchers-1) {end=k-1;} //last searcher takes remaining records
	}
	printf("%d %d %d %d\n",searcher,start,end,end-start+1);
	fseek (fpb , start*sizeof(rec) , SEEK_SET); 
	for (i=0; i < end-start+1; i++) {
		fread(&rec, sizeof(rec), 1, fpb);
		if (is_there(rec,pattern) == 1) {
			//rec_print(rec);
			write(fd_w,&rec,sizeof(rec));
		}
	}
	close(fd_w);
	fclose (fpb);

	end_t = clock();
	total_t = (double) (end_t - start_t) / CLOCKS_PER_SEC ;
	set_stats(&mystats,total_t);

	write(fd_w2,&mystats,sizeof(stats_struct));
	close(fd_w2);

	kill(atoi(root_pid),SIGUSR2);
	exit(0);
}
