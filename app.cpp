#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<math.h>

double komi=2.5;
const int B_SIZE=9;
const int WIDTH=B_SIZE+2;
const int BOARD_MAX=WIDTH*WIDTH;

int board[BOARD_MAX]={
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,0,
    3,3,3,3,3,3,3,3,3,3,3
};


int dir4[4]={+1,-1,+WIDTH,-WIDTH};
int hama[2];
int kifu[1000];
int ko_z;
int all_playouts;

int get_z(int x, int y)
{
    return (y+1)*WIDTH+(x+1); //0<=x <=8, 0<=y<=8
}

int get81(int z)
{
    if(z==0)return 0;
    int y=z/WIDTH; 
    int x=z-y*WIDTH; 
    return x*10+y; 
}


int get_t(int z)
{
    int t;
    int i;
    int x;
    int y;
    
    
    x=z/10;
    i=x*10;
    y=z-i;
    
    
    t=(y*11)+x;
    
    return t;
    
}


int flip_color(int col){
    return 3-col;
}

int check_board[BOARD_MAX];


void count_dame_sub(int tz,int color,int *p_dame,int *p_ishi)
{
    int z,i;
    check_board[tz]=1;
    (*p_ishi)++;
    for(i=0;i<4;i++){
        z=tz+dir4[i];
        if(check_board[z])continue;
        if(board[z]==0){
            check_board[z]=1;
            (*p_dame)++;
        }
        
        if(board[z]==color)count_dame_sub(z,color,p_dame,p_ishi);
    }
}


void count_dame(int tz,int *p_dame,int *p_ishi)
{
    int i;
    *p_dame=*p_ishi=0;
    for(i=0;i<BOARD_MAX;i++)check_board[i]=0;
    count_dame_sub(tz,board[tz],p_dame,p_ishi);
}


void kesu(int tz,int color)
{
    int z,i;
    
    board[tz]=0;
    for(i=0;i<4;i++){
        z=tz+dir4[i];
        if(board[z]==color)kesu(z,color);
    }
}



int move(int tz,int color)
{
    if(tz==0){ko_z=0;return 0;}
    
    int around[4][3];
    int un_col=flip_color(color);
    
    
    int space=0;
    int kabe=0;
    int mikata_safe=0;
    int take_sum=0;
    int ko_kamo=0;
    int i;
    for(i=0;i<4;i++){
        around[i][0]=around[i][1]=around[i][2]=0;
        int z=tz+dir4[i];
        int c=board[z];
        if(c==0)space++;
        if(c==3)kabe++;
        if(c==0||c==3)continue;
        int dame;
        int ishi;
        count_dame(z,&dame,&ishi);
        around[i][0]=dame;
        around[i][1]=ishi;
        around[i][2]=c;
        if(c==un_col &&dame==1){take_sum+=ishi;ko_kamo=z;}
        if(c==color && dame>=2)mikata_safe++;
    }
    
    if(take_sum==0&&space==0&&mikata_safe==0)return 1;
    if(tz==ko_z) return 2;
    if(kabe+mikata_safe==4)return 3;
    if(board[tz]!=0)return 4;
    
    for(i=0;i<4;i++){
        int d=around[i][0];
        int n=around[i][1];
        int c=around[i][2];
        if(c==un_col&&d==1&&board[tz+dir4[i]]){
            kesu(tz+dir4[i],un_col);
            hama[color-1]+=n;
        }
    }
    board[tz]=color;
    int dame,ishi;
    count_dame(tz,&dame,&ishi);
    if(take_sum==1&&ishi==1&&dame==1){
        ko_z=ko_kamo;
    }else{
        ko_z=0;
    }
    return 0;
}


void print_board()
{
    int x,y;
    char *str[3]={"・","○","●"};
    
    printf("  ");
    for(x=0;x<B_SIZE;x++)printf(" %d",x+1);
    printf("\n");
    for(y=0;y<B_SIZE;y++){
        printf("%d:",y+1);
        for(x=0;x<B_SIZE;x++){
            printf("%s",str[board[get_z(x,y)]]);
        }
        printf("\n");
    }
}


int count_score(int turn_color)
{
    int score=0;
    int kind[3];
    kind[0]=kind[1]=kind[2]=0;
    int x,y,i;
    for(y=0;y<B_SIZE;y++)for(x=0;x<B_SIZE;x++){
        int z=get_z(x,y);
        int c=board[z];
        kind[c]++;
        if(c!=0)continue;
        int mk[4];
        mk[1]=mk[2]=0;
        for(i=0;i<4;i++)mk[board[z+dir4[i]] ]++;
        if(mk[1]&&mk[2]==0)score++;
        if(mk[2]&&mk[1]==0)score--;
    }
    score+=kind[1]-kind[2];
    
    double final_score=score-komi;
    int win=0;
    if(final_score>0)win=1;
    //printf("win=%d,score=%d\n",win,score);
    
    if(turn_color==2)win=-win;
    return win;
}


int playout(int turn_color)
{
    all_playouts++;
    int color=turn_color;
    int before_z=-1;
    int loop;
    int loop_max=B_SIZE*B_SIZE+200;
    for(loop=0;loop<loop_max;loop++){
        
        int kouho[BOARD_MAX];
        int kouho_num=0;
        int x,y;
        for (y=0;y<B_SIZE;y++)for(x=0;x<B_SIZE;x++){
            int z=get_z(x,y);
            if (board[z]!=0)continue;
            kouho[kouho_num]=z;
            kouho_num++;
        }
        int z,r=0;
        for(;;){
            if(kouho_num==0){
                z=0;
            }else{
                r=rand()%kouho_num;
                z=kouho[r];
            }
            int err=move(z,color);
            if(err==0)break;
            kouho[r]=kouho[kouho_num-1];
            kouho_num--;
        }
        if(z==0&&before_z==0)break;
        before_z=z;
        
        color=flip_color(color);
    }
    return count_score(turn_color);
}

int select_best_move(int color)
{
    int try_num=10000;
    int best_z=0;
    double best_value=-100;
    
    int board_copy[BOARD_MAX];
    memcpy(board_copy,board,sizeof(board));
    int ko_z_copy=ko_z;
    
    
    int x,y;
    for(y=0;y<B_SIZE;y++)for(x=0;x<B_SIZE;x++){
        int z=get_z(x,y);
        if(board[z]!=0)continue;
        
        int err=move(z,color);
        if(err!=0)continue;
        
        int win_sum=0;
        int i;
        for(i=0;i<try_num;i++){
            int board_copy2[BOARD_MAX];
            memcpy(board_copy2,board,sizeof(board));
            int ko_z_copy2=ko_z;
            
            int win=-playout(flip_color(color));
            win_sum+=win;
            
            
            memcpy(board,board_copy2,sizeof(board));
            ko_z=ko_z_copy2;
        }
        double win_rate=(double)win_sum/try_num;
        
        
        if(win_rate>best_value){
            best_value=win_rate;
            best_z=z;
            printf("best_z=%d,v=%5.3f,try_num=%d\n",get81(best_z),best_value,try_num);
        }
        
        memcpy(board,board_copy,sizeof(board));
        ko_z=ko_z_copy;
    }
    return best_z;
}

int i_move(int t)
{
    
    char bb[80];
    int i;
    int g;
    
    do{
        fgets(bb,79,stdin);
        i=atoi(bb);
    }while(i>99);
    
    g=get_t(i);
    
    return g;
    
}


typedef struct child{
    int z;
    int games;
    double rate;
    int next;
}CHILD;

const int CHILD_MAX=B_SIZE*B_SIZE+1;


typedef struct node{
    int child_num;
    CHILD child[CHILD_MAX];
    int games_sum;
}NODE;

const int NODE_MAX=10000;
NODE node[NODE_MAX];

int node_num=0;
const int NODE_EMPTY=-1;
const int ILLEGAL_Z=-1;


void add_child(NODE *pN,int z)
{
    int n=pN->child_num;
    pN->child[n].z=z;
    pN->child[n].games=0;
    pN->child[n].rate=0;
    pN->child[n].next=NODE_EMPTY;
    pN->child_num++;
}



int create_node(){
    if(node_num==NODE_MAX){printf("node over\n");exit(0);}
    NODE *pN=&node[node_num];
    pN->child_num=0;
    int x,y;
    for(y=0;y<B_SIZE;y++)for(x=0;x<B_SIZE;x++){
        int z=get_z(x,y);
        if(board[z]!=0)continue;
        add_child(pN,z);
    }
    add_child(pN,0);
    node_num++;
    return node_num-1;
}



int search_uct(int color,int node_n)
{
    NODE *pN=&node[node_n];
re_try:
    int select=-1;
    double max_ucb=-999;
    int i;
    for(i=0;i<pN->child_num;i++){
        CHILD *c=&pN->child[i];
        if(c->z==ILLEGAL_Z)continue;
        double ucb=0;
        if(c->games==0){
            ucb=10000+rand();
        }else{
            const double C =0.80;
            ucb=c->rate + C * sqrt(log((double)(pN->games_sum)/c->games));
        }
        if(ucb>max_ucb){
            max_ucb=ucb;
            select=i;
        }
    }
    if(select==-1){printf("Err! select\n");exit(0);}
    
    CHILD *c=&pN->child[select];
    int z=c->z;
    int err=move(z,color);
    if(err!=0){
        c->z=ILLEGAL_Z;
        goto re_try;
    }
    
    int win;
    if(c->games==0){
        win=-playout(flip_color(color));
    }else{
        if(c->next==NODE_EMPTY)c->next=create_node();
        win=-search_uct(flip_color(color),c->next);
    }
    
    c->rate=(c->rate*c->games+win)/(c->games+1);
    c->games++;
    pN->games_sum++;
    return win;
}



int select_best_uct(int color)
{
    node_num=0;
    int next=create_node();
    
    int uct_loop=10000;
    int i;
    for(i=0;i<uct_loop;i++){
        int board_copy[BOARD_MAX];
        memcpy(board_copy,board,sizeof(board));
        int ko_z_copy=ko_z;
        
        search_uct(color,next);
        
        memcpy(board,board_copy,sizeof(board));
        ko_z=ko_z_copy;
    }
    
    int best_i=-1;
    int max=-999;
    NODE *pN=&node[next];
    for(i=0;i<pN->child_num;i++){
        CHILD *c=&pN->child[i];
        if(c->games>max){
            best_i=i;
            max=c->games;
        }
    }
    int ret_z=pN->child[best_i].z;
    printf("z=%2d,rate=%.4f,games=%d,playouts=%d,nodes=%d\n",get81(ret_z),pN->child[best_i].rate,
           max,all_playouts,node_num);
    return ret_z;
    
}





int main()
{
    int color=1;
    int tesuu=0;
    char bb[80];
    int i;
    
    srand((unsigned)time(NULL));
    //loop:
    
    {int i,x,y;for(i=0;i<BOARD_MAX;i++)board[i]=3;
        for(y=0;y<B_SIZE;y++)for(x=0;x<B_SIZE;x++)
            board[get_z(x,y)]=0;}
    
    
    
    //static int score_sum;
    //static int loop_count;
    
    
    
    
    do{
        printf("1:黒\n");
        printf("2:白\n");
		printf("黒を持ちたい場合は１を、白を持ちたい場合は２を入力してください\n");
        
        fgets(bb,79,stdin);
        
        i=atoi(bb);
    }while(i!=1&&i!=2);
    
    
    switch(i){
            
        case 2:for(;;){
            color = 1;
			print_board();
            
			printf("コンピュータ思考中\n");
            clock_t bt=clock();
            all_playouts=0; //playoutの回数を初期化
            //int z=select_best_move(color);
            int z=select_best_uct(color);
            int err=move(z,color); //打つ
            if(err!=0){printf("Err!\n");exit(0);}
            kifu[tesuu++]=z;
            print_board();
            printf("play_z=%d, 手数=%d, 色=%d, all_playouts=%d\n",get81(z),tesuu,color,all_playouts);
            double t=(double)(clock()+1-bt)/CLOCKS_PER_SEC;
            printf("%.1f秒,%.0f playout/秒\n",t,all_playouts/t);
            if(z==0&&tesuu>1&&kifu[tesuu-2]==0)break;
            if(tesuu>300)break;
            color=flip_color(color);
            
			printf("手を選択してください\n");
            int zi=i_move(color);
            err=move(zi,color);
            if(err!=0){printf("Err!\n");exit(0);}
            kifu[tesuu++]=zi;
            print_board();
            if(zi==0&&tesuu>1&&kifu[tesuu-2]==0)break;
            color=flip_color(color);
        };
            break;
            
            
        case 1:for(;;){
            color = 1;
			print_board();
			printf("手を選択してください\n");
            int zi=i_move(color);
            int err=move(zi,color);
            if(err!=0){printf("Err!\n");exit(0);}
            kifu[tesuu++]=zi;
            print_board();
            if(zi==0&&tesuu>1&&kifu[tesuu-2]==0)break;
            color=flip_color(color);
            
			printf("コンピュータ思考中\n");
            clock_t bt=clock();
            all_playouts=0; 
            
            int z=select_best_uct(color);
            err=move(z,color); 
            if(err!=0){printf("Err!\n");exit(0);}
            kifu[tesuu++]=z;
            print_board();
            printf("play_z=%d, 手数=%d, 色=%d, all_playouts=%d\n",get81(z),tesuu,color,all_playouts);
            double t=(double)(clock()+1-bt)/CLOCKS_PER_SEC;
            printf("%.1f秒,%.0f playout/秒\n",t,all_playouts/t);
            if(z==0&&tesuu>1&&kifu[tesuu-2]==0)break;
            if(tesuu>300)break;
            color=flip_color(color);
            
            
            
        };
            break;
            
        default: printf("Err!\n");
            
    }
    
    
    
    
   
     
    return 0;
    
    
}




