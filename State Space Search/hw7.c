/******************************************************************************
 CS288 HW7 three search strategies: depth, breadth, and intelligent
 command example: command 16 numbers and search strategy

 fifteen 1 2 3 4 5 6 7 8 9 10 11 12 13 14 0 15 {dfs|bfs|astar}
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#define BF 4			/* Branching factor of the search tree */
#define N 4
#define NxN 16
#define DFS 1                   /* depth first search */
#define BFS 2                   /* breadth first search */
#define BEST 3                  /* best first search */
#define BB 4                    /* branch and bound */
#define ASTAR 5                 /* A* search */
#define UNKNOWN 9		/* unknown search strategy */

#define MAX_PATH 1000

#define DN 0			/* for finding path */
#define RT 1
#define UP 2
#define LT 3
#define UK 9

#define FVAL 0			/* f=g+h, keep it simple for now */
#define GVAL 1
#define HVAL 2
#define PATH 3			/* array index, how it got to this state */

#define TRUE 1
#define FALSE 0

int level,strategy;

int nodes_same(),str_compare(),count();
void swap(),exit_proc(),print_a_node(),print_nodes();
int toggle_dir(), solvable();
void find_path(),print_path();
int path[MAX_PATH],path_buf[MAX_PATH];
char *prep_dir_to_str();
char *dir_to_sdir[4],strategy_s[10]; /* four directions */

struct node {
  int board[N+1][N];
  struct node *next;
};

struct node *start,*goal;
struct node *initialize(),*expand(),*merge(),*filter(),*move(),*append();
struct node *insert_node(),*check_list(),*goal_found(),*find_parent();

int main(int argc,char **argv) {

  long int sec;
  struct timeval tv_s,tv_e;
  
  int iter,cnt=0,total=1,ocnt=0,ccnt=0;
  int perm;		/* perm=odd=no solution, perm=even=solvable */
  struct node *cp,*open,*closed,*succ,*tp;
  
  open=closed=succ=NULL;
  start=initialize(argc,argv);	/* init initial and goal states */
  
  //start timer
  gettimeofday(&tv_s, NULL);
  
  perm=solvable(start);		/* check if solvable permutation */
  if (perm & 1){
  	printf("Not solvable :(\n");
  	return 1;
  }
  open=start; 
  iter=0;

  while (open) {
    //printf("%d: open=%d + clsd=%d = total=%d\n",iter,ocnt,ccnt,ocnt+ccnt);
    ocnt=count(open);
    ccnt=count(closed);
    cp=open; open=open->next; cp->next=NULL; /* get the first node from open */
    succ = expand(cp);			     /* Find new successors */	        
    succ = filter(succ,open);			
    succ = filter(succ,closed);		     /* New succ list */   
    cnt=count(succ);
    total=total+cnt;
    if (succ) open=merge(succ,open,strategy); /* New open list */
    closed=append(cp,closed);		      /* New closed */
    if ((cp=goal_found(succ,goal))) break;
    iter++;
  }
  printf("%s strategy: %d iterations %d nodes\n",strategy_s,iter+1,total);
  find_path(cp,open,closed);
  
  //end timer
  gettimeofday(&tv_e, NULL);
  
  sec = ((tv_e.tv_sec) + (tv_e.tv_usec/1000000)) - ((tv_s.tv_sec) + (tv_s.tv_usec/1000000));
  printf("Seconds Elapsed %ld\n", sec);
  
  
  return 0;
} /* end of main */

int toggle_dir(int dir){
  int opp_dir;
  
  if(dir < 2){
  	opp_dir = dir + 2;	
  }
  else{
  	opp_dir = dir - 2;
  }
  return opp_dir;
}

void print_path(int size, struct node *path){
  int i,p;
  char *sdir;
  path = path->next;
  printf("PATH FROM INITIAL NODE\n");
  while(path){
  	sdir = prep_dir_to_str(path);
  	printf("%s\n", sdir);	
  	path = path->next;
  }
}
char *prep_dir_to_str(struct node *cp){
  char *sdir;
  
  if(cp->board[N][3] == 0){
  	sdir = "DN";
  }
  else if(cp->board[N][3] == 1){
  	sdir = "RT";
  }
  else if(cp->board[N][3] == 2){
  	sdir = "UP";
  }
  else{
  	sdir = "LT";
  }
  return sdir;
}

void find_path(struct node *cp,struct node *opnp,struct node *cldp){
  int i,j,dir,opp_dir,initFnd, cnt, Zcnt;
  struct node *tp, *clp, *rpath, *path, *tpath;
  rpath = malloc(sizeof(struct node));
  path = malloc(sizeof(struct node));
  Zcnt = 0;
  rpath = NULL;
  path = NULL;
  //clp = cldp;
  initFnd = FALSE;
  rpath = append(cp, rpath);
  //start from the very last node that matches the goal state because
  //that has the direction info
  //toggle the direction, get the parent
  //look for that parent in closed
  //repeat until you find the initial state
  while(!initFnd){
  	//print_a_node(cp);
  	dir = cp->board[N][3];
  	opp_dir = toggle_dir(dir);
  	cp = find_parent(cp, opp_dir);
  	
  	clp = cldp;
	while(clp){
		if(nodes_same(cp, clp)){
			clp->next = NULL;
			//print_a_node(clp);
			rpath = append(clp, rpath);
			cp = clp;
			break;
		}
		clp = clp->next;
	}
	if(clp != NULL){
		for(i = 0; i < 4; i++){
			if(clp->board[N][i] == 0){
				Zcnt++;		
			}	
		}
		//printf("%d\n", Zcnt);
		if(Zcnt == 4){
			initFnd = TRUE;
		}
		else{
			Zcnt = 0;
		} 
	}
		
  }
  cnt = count(rpath);
  tpath = rpath;
  for(j = 0; j < cnt; j++){
  	if(j == cnt - 1){
  		path = append(tpath, path);
  	}
  	else{
	  	while(tpath->next->next != NULL){
	  		tpath = tpath->next;
	  	}
	  	path = append(tpath->next, path);
	  	tpath->next = NULL;
	  	tpath = rpath;
  	}
  } 
  print_path(count(path), path);  
}

struct node *find_parent(struct node *cp,int prev_dir){
  int i,j,k,cnt,row=0,col=j;
  
  struct node *tp;
  
  for(i = 0; i < N; i++){
  	for(j = 0; j < N; j++){
  		if (cp->board[i][j] == 0){
  			row = i;
  			col = j;
  			break;
  		}
  	}
  }
  
  /* DOWN null is at row i column j swap jawn is at row i + 1 column j*/
  if(prev_dir == 0){
  	tp = move(cp, row, col, row+1, col, DN);
  }
  /* RIGHT swap is j + 1*/
  else if(prev_dir == 1){
  	tp = move(cp, row, col, row, col+1, RT);
  }
  /* UP swap is i - 1*/
  else if(prev_dir == 2){
  	tp = move(cp, row, col, row-1, col, UP);
  }
  /* LEFT swap is j - 1*/
  else if(prev_dir == 3){
  	tp = move(cp, row, col, row, col-1, LT);
  }
  return tp;
}

// Expand: generate successors of the current node
struct node *expand(struct node *cp) {
  int i,j,k,cnt,row=0,col=j;
  struct node *succ,*tp;
  succ=NULL;

  /* check where 0 is. find indices i,j */
  for(i = 0; i < N; i++){
  	for(j = 0; j < N; j++){
  		if (cp->board[i][j] == 0){
  			row = i;
  			col = j;
  			break;
  		}
  	}
  }
   
  /* DOWN null is at row i column j swap jawn is at row i + 1 column j*/
  if(row+1 < N){
  	tp = move(cp, row, col, row+1, col, DN);
  	succ = append(tp, succ);
  }
  
  /* RIGHT swap is j + 1*/
  if(col+1 < N){
  	//printf("%d\n", i);
  	tp = move(cp, row, col, row, col+1, RT);
  	succ = append(tp, succ);
  }
  /* UP swap is i - 1*/
  if(row-1 >= 0){
  	tp = move(cp, row, col, row-1, col, UP);
  	succ = append(tp, succ);
  }
  /* LEFT swap is j - 1*/
  if(col-1 >= 0){
  	tp = move(cp, row, col, row, col-1, LT);
  	succ = append(tp, succ);
  }
  return succ;
}
/* attach at the end */
struct node *append(struct node *tp,struct node *sp) {
  struct node *cp;
  if(sp == NULL){
  	sp = tp;
  }
  else{
  	cp = sp;
  	while(cp->next != NULL){
  		cp = cp->next; 
  	}
  	cp->next = tp; 
  }
  return sp;
}

void swap(struct node *cp,int i,int j,int k,int l){
  int tmp;
  //save newp board[k][l]
  tmp = cp->board[k][l];
  //move newp[i][j] to newp[k][l]
  cp->board[k][l] = cp->board[i][j];
  //move temp to newp[i][j]
  cp->board[i][j] = tmp;
}

struct node *move(struct node *cp,int a,int b,int x,int y,int dir) {
  struct node *newp;
  int i,j,k,l, f,g,h,pos;
  pos = 0;
  // malloc
  newp = malloc(sizeof(struct node));
  // copy from cp
  for(i = 0; i <= N; i++){
  	for(j = 0; j <= N; j++){
  		newp->board[i][j] = cp->board[i][j];
  	}
  }
  // swap two vals
  swap(newp, a, b, x, y);
  // compute g
  g = newp->board[N][1] + 1;
  newp->board[N][1] = g;
  // compute h
  h = 0;
  for(i = 0; i < N; i++){
  	for(j = 0; j < N; j++){
  		pos++;
  		if(newp->board[i][j] != pos && pos < (N * N)){
  			for(k = 0; k < N; k++){
  				for(l = 0; l < N; l++){
  					if(newp->board[k][l] == pos){
  						h = h + (k - i) + (l - j);
  						break;
  					}
  				}
  			}	
  		}
  		else if(newp->board[i][j] != pos && pos == N * N){
  			for(k = 0; k < N; k++){
  				for(l = 0; l < N; l++){
  					if(newp->board[k][l] == 0){
  						h = h + (i - k) + (j - l);
  						break;
  					}
  				}
  			}
  		}
  	}
  }
  newp->board[N][2] = h;
  // compute f 
  f = g + h;
  newp->board[N][0] = f;
  // insert the direction that resulted in this node, used for printing path
  newp->board[N][3] = dir;
  return newp;
}

struct node *goal_found(struct node *cp,struct node *gp){
  int flg=FALSE;
  struct node *tp = cp;
  // check if succ list has goal
  while(tp){
  	if(nodes_same(tp,gp)){
  		//found goal
  		cp = tp;
  		cp->next = NULL;
  		break;
  	}
  	tp = tp->next;
  }
  if(tp == NULL){
  	//goal not found
  	return NULL;
  }
  // if found, return that for finding path else return NULL
  return cp;
}

int count(struct node *cp) {
  int cnt=0;
  struct node *tp;
  tp = cp;
  if(tp == NULL){
  	return cnt;
  }
  else{
  	cnt++;
  	while(tp->next != NULL){
  		cnt++;
  		tp = tp->next;	
  	}
  }
  return cnt;
}

struct node *merge(struct node *succ,struct node *open,int flg) {

  if (flg==DFS) {
    open = append(open, succ);
  }
  else if (flg==BFS) {
    open = append(succ, open);
  }
  else if (flg==ASTAR) {
  	//uses f
  	open = insert_node(succ, open, 0); 
  }
  else if (flg == BEST){
  	//uses h		
  	open = insert_node(succ, open, 2);
  }
  else{
  	//BB using g
  	open = insert_node(succ,open,1);	
  }
  return open;
}

/* insert succ into open in ascending order of x value, where x is an array 
   index: 0=f,1=g,h=2 of board[N][x]
 */
struct node *insert_node(struct node *succ,struct node *open,int x) {
  
  struct node *succ_next;
  while (succ) {
    succ_next = succ->next;

    if (!open || succ->board[N][x] <= open->board[N][x]) {  
      succ->next = open;
      open = succ;
    } 
    else {
      struct node *copen;
      for (copen = open; copen->next; copen = copen->next) {
        if (succ->board[N][x] <= copen->next->board[N][x]) {
          succ->next = copen->next;
          copen->next = succ;
          break;
        }
      }
      if (!copen->next) {  
        copen->next = succ;
        succ->next = NULL;
      }
    }

    succ = succ_next;
  }

  return open;
  //end
}

int nodes_same(struct node *xp,struct node *yp) {
  int i,j,flg=TRUE;
  for(i = 0; i < N; i++){
  	for(j = 0; j < N; j++){
  		if(xp->board[i][j] != yp->board[i][j]){
  			flg = FALSE;
  			break;
  		}	
  	}
  	if(flg == FALSE){
  		break;
  	}
  }
  return flg;
}

/******************************************************************************
  Check succ against open and closed. Remove those succ nodes that are in open or closed.
******************************************************************************/
struct node *filter(struct node *succ,struct node *hp){ 
   struct node *lsp,*rsp;
   struct node *tp;	
   
   tp = hp;
   lsp = succ;
   rsp = succ;
   while(rsp){
   	while(tp && !nodes_same(tp, rsp)){
   		tp = tp->next;
   	}
   	if(tp == NULL){
   		//no matches in hp
   		lsp = rsp;
   		rsp = rsp->next;
   		tp = hp;
   	}
   	else{
   		//nodes are the same
   		if(lsp == rsp){
   			//first elemet
   			succ = lsp = rsp = rsp->next;
   		}
   		else{
   			//not first
   			lsp->next = rsp->next;
   			rsp = rsp->next;
   		}
   		
   	}	
   }
   return succ;
}

void print_nodes(struct node *cp,char name[20]) {
  int i;
  printf("%s:\n",name);
  while (cp) { print_a_node(cp); cp=cp->next; }
}

void print_a_node(struct node *np) {
  int i,j;
  for (i=0;i<N+1;i++) {
    for (j=0;j<N;j++) printf("%2d ",np->board[i][j]);
    printf("\n");
  }
  printf("\n");
}

//cnt=odd -> no solution, cnt=even=solvable
int solvable(struct node *cp) {
  int i,j,k=0,lst[N*N],cnt=0,total=0,row=0,nullrow=0;
  
  for (i = 0; i < N; i++){
  	for(j = 0; j < N; j++){
  		lst[cnt] = cp->board[i][j];
  		cnt++;
  	}	
  }
  //find inv count
  for(i = 0; i < N * N; i++){
  	if(i % N == 0){
  		row++;
  	}
  	if(lst[i] == 0){
  		nullrow = row;
  		continue;
  	}
  	for(j= i + 1; j < N * N; j++){
  		if(lst[j] != 0 && lst[i] > lst[j]){
  			//printf("%d > %d\n", lst[i], lst[j]);
  			total++;
  		}
  	}
  }
  if(N & 1){
  	return total;
  }
  else{
  	//N even
  	
  	total = total + nullrow;
  }
  return total;	
}

/* fif 0 1 2 4 5 6 3 8 9 10 7 12 13 14 11 15 astar */
struct node *initialize(int argc, char **argv){
  int i,j,k,npe,n,idx,gidx,inv;
   struct node *tp;

   tp=(struct node *) malloc(sizeof(struct node));
   idx = 1;
   for (j=0;j<N;j++)
     for (k=0;k<N;k++) tp->board[j][k]=atoi(argv[idx++]);
   for (k=0;k<N;k++) tp->board[N][k]=0;	/* set f,g,h of initial state to 0 */
   tp->next=NULL;
   start=tp;

   printf("init state: \n");
   print_a_node(start);

   tp=(struct node *) malloc(sizeof(struct node));
   gidx = 1;
   for (j=0;j<N;j++)
     for (k=0;k<N;k++) tp->board[j][k] = gidx++;
   tp->board[N-1][N-1] = 0;		/* empty tile=0 */
   for (k=0;k<N;k++) tp->board[N][k]=0;	/* set f,g,h of goal state to 0 */
   tp->next=NULL;
   goal=tp;

   printf("goal state: \n");
   print_a_node(goal);

   strcpy(strategy_s,argv[idx]);
   if (strcmp(strategy_s,"dfs")==0) strategy=DFS;
   else if (strcmp(strategy_s,"bfs")==0) strategy = BFS;
   else if (strcmp(strategy_s,"best")==0) strategy=BEST;
   else if (strcmp(strategy_s,"bb")==0) strategy=BB;
   else if (strcmp(strategy_s,"astar")==0) strategy=ASTAR;
   else strategy=UNKNOWN;
   printf("strategy=%s\n",strategy_s);

   return start;
}

void exit_proc(char *msg){
   printf("Error: %s\n",msg);
   exit(1);
}

/*****************************************************************************
 End of file: hw7.c. Fifteen Puzzle, Sequential A* 1 processor version.
*****************************************************************************/
