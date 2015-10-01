//囲碁（９路盤）　モンテカルロ法をベースに作成
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<math.h>

double komi=2.5;
const int B_SIZE=9;  //碁盤の大きさ
const int WIDTH=B_SIZE+2; //枠を含めた横幅
const int BOARD_MAX=WIDTH*WIDTH;

int board[BOARD_MAX]={//初期盤面、盤外は3で示す
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


int dir4[4]={+1,-1,+WIDTH,-WIDTH}; //右、左、下、上への移動量
int hama[2]; //取った石の数
int kifu[1000]; //今までの手順
int ko_z; //コウで打てない位置
int all_playouts; //pleyoutを行った回数

//(x,y)座標を1次元表記に変換
int get_z(int x, int y)
{
    return (y+1)*WIDTH+(x+1); //0<=x <=8, 0<=y<=8
}

//表示用に座標を変換
int get81(int z)
{
    if(z==0)return 0;
    int y=z/WIDTH; 
    int x=z-y*WIDTH; 
    return x*10+y; 
}

//表示用表記から一次元表記に変換
int get_t(int z)
{
    int t;
    int i;
    int x;
    int y;
    
    //表示用表記を適切なx,yに分離する
    x=z/10;
    i=x*10;
    y=z-i;
    
    //x,yを使って一次元表記を求める
    t=(y*11)+x;
    
    return t;
    
}

//色の反転
int flip_color(int col){
    return 3-col; //石の色を反転させる1->2,2->1
}

int check_board[BOARD_MAX]; //検索済みフラグ.石のありなしで併用

//ダメの数を数える関数

//ダメと石数を数える再帰関数
//４方向を調べて、空白なら+1,自分の石なら再帰で.相手の石、壁はそのまま
void count_dame_sub(int tz,int color,int *p_dame,int *p_ishi)
{
    int z,i;
    check_board[tz]=1; //この位置（石）は検索済み
    (*p_ishi)++; //石の数
    for(i=0;i<4;i++){
        z=tz+dir4[i]; //４方向を調べる
        if(check_board[z])continue;
        if(board[z]==0){
            check_board[z]=1;//この位置（空点）はカウント済みに
            (*p_dame)++;  //ダメの数
        }
        //未探索の自分の石
        if(board[z]==color)count_dame_sub(z,color,p_dame,p_ishi);
    }
}


//位置の tz におけるダメの数と石の数を計算
void count_dame(int tz,int *p_dame,int *p_ishi)
{
    int i;
    *p_dame=*p_ishi=0;
    for(i=0;i<BOARD_MAX;i++)check_board[i]=0;
    count_dame_sub(tz,board[tz],p_dame,p_ishi);
}

//石を消す関数
void kesu(int tz,int color)
{
    int z,i;
    
    board[tz]=0; //石を消す
    for(i=0;i<4;i++){
        z=tz+dir4[i];
        if(board[z]==color)kesu(z,color);
    }
}


//石を置く関数
int move(int tz,int color)
{
    if(tz==0){ko_z=0;return 0;} //パスの場合
    
    int around[4][3]; //４方向のダメ数、石数、色
    int un_col=flip_color(color); //相手の石の数
    
    //４方向の石のダメと石数を調べる
    int space=0; //４方向の空白の数
    int kabe=0; //4方向の盤外の数
    int mikata_safe=0; //ダメ２以上で安全な味方の数
    int take_sum=0; //取れる石の合計
    int ko_kamo=0; //コウになるかもしれない場所
    int i;
    for(i=0;i<4;i++){
        around[i][0]=around[i][1]=around[i][2]=0;
        int z=tz+dir4[i];
        int c=board[z]; //石の数
        if(c==0)space++;
        if(c==3)kabe++;
        if(c==0||c==3)continue;
        int dame; //ダメの数
        int ishi; //石の数
        count_dame(z,&dame,&ishi);
        around[i][0]=dame;
        around[i][1]=ishi;
        around[i][2]=c;
        if(c==un_col &&dame==1){take_sum+=ishi;ko_kamo=z;}
        if(c==color && dame>=2)mikata_safe++;
    }
    //自殺手
    if(take_sum==0&&space==0&&mikata_safe==0)return 1;
    if(tz==ko_z) return 2; //コウ
    if(kabe+mikata_safe==4)return 3; //眼（ルール違反ではない）
    if(board[tz]!=0)return 4; //既に石がある
    
    for(i=0;i<4;i++){
        int d=around[i][0];
        int n=around[i][1];
        int c=around[i][2];
        if(c==un_col&&d==1&&board[tz+dir4[i]]){
            kesu(tz+dir4[i],un_col);  //石が取れる
            hama[color-1]+=n;
        }
    }
    board[tz]=color; //石を置く
    int dame,ishi;
    count_dame(tz,&dame,&ishi);
    if(take_sum==1&&ishi==1&&dame==1){//コウになる
        ko_z=ko_kamo;
    }else{
        ko_z=0;
    }
    return 0;
}


//盤面を表示する関数
void print_board()
{
    int x,y;
    char *str[3]={"・","○","●"};
    
    printf("  ");  //この部分の空白の数が盤面をきれいに表示させるポイント
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


//地を数えて勝ちか負けかを返す関数
int count_score(int turn_color)
{
    int score=0;
    int kind[3]; //盤上に残ってる石数
    kind[0]=kind[1]=kind[2]=0;
    int x,y,i;
    for(y=0;y<B_SIZE;y++)for(x=0;x<B_SIZE;x++){
        int z=get_z(x,y);
        int c=board[z];
        kind[c]++;
        if(c!=0)continue;
        int mk[4]; //空点は４方向の石を種類別に数える
        mk[1]=mk[2]=0; //mk[0] 空,[1] 黒, [2] 白, [3] 盤外
        for(i=0;i<4;i++)mk[board[z+dir4[i]] ]++;
        if(mk[1]&&mk[2]==0)score++;//黒だけに囲まれていれば黒地
        if(mk[2]&&mk[1]==0)score--;//白だけに囲まれていれば白地
    }
    score+=kind[1]-kind[2]; //地+盤上の石数
    
    double final_score=score-komi;
    int win=0;
    if(final_score>0)win=1; //黒が勝っていれば１
    //printf("win=%d,score=%d\n",win,score);
    //win=score; //勝率ではなく目数差を返す場合
    if(turn_color==2)win=-win; //白番では反転させる
    return win;
}

//プレイアウトを行う関数
int playout(int turn_color)
{
    all_playouts++; //プレイアウト関数が呼ばれた回数
    int color=turn_color;
    int before_z=-1; //1手前の手
    int loop;
    int loop_max=B_SIZE*B_SIZE+200; //３コウ対策で手数を制限
    for(loop=0;loop<loop_max;loop++){
        //すべての空点を着手候補にする
        int kouho[BOARD_MAX];
        int kouho_num=0;
        int x,y;
        for (y=0;y<B_SIZE;y++)for(x=0;x<B_SIZE;x++){
            int z=get_z(x,y);
            if (board[z]!=0)continue;
            kouho[kouho_num]=z;
            kouho_num++;
        }
        int z,r=0; //rを初期化してないエラーがでたので、両方とも初期化した
        for(;;){//着手可能な手を１手見つけるまでループ
            if(kouho_num==0){
                z=0;
            }else{
                r=rand()%kouho_num; //乱数で１手選ぶ
                z=kouho[r];
            }
            int err=move(z,color);
            if(err==0)break; //着手可能なのでこの手を選ぶ
            kouho[r]=kouho[kouho_num-1]; //この手を削除.末尾の手を代入
            kouho_num--;
        }
        if(z==0&&before_z==0)break; //連続パス
        before_z=z;
        //print_board();//プレイアウト途中の様子を表示
        //printf("loop=%d,z=%d,c=%d,kouho_num=%d,ko_z=%d\n",loop,get81(z),color,kouho_num,get81(ko_z));
        color=flip_color(color); //手番を入れ替える
    }
    return count_score(turn_color);//プレイアウトを開始した手番を渡す
}

//可能な手に対してプレイアウトを行う回数
int select_best_move(int color)
{
    int try_num=10000; //プレイアウトを繰り返す回数
    int best_z=0; //最善手
    double best_value=-100;
    
    int board_copy[BOARD_MAX];
    memcpy(board_copy,board,sizeof(board)); //現局面を保存
    int ko_z_copy=ko_z;
    
    //すべての空点を着手候補に
    int x,y;
    for(y=0;y<B_SIZE;y++)for(x=0;x<B_SIZE;x++){
        int z=get_z(x,y);
        if(board[z]!=0)continue; //石がある
        
        int err=move(z,color); //打ってみる
        if(err!=0)continue;
        
        int win_sum=0; //勝った回数の合計
        int i;
        for(i=0;i<try_num;i++){ //プレイアウトを繰り返す
            int board_copy2[BOARD_MAX]; //局面が壊れるので保存
            memcpy(board_copy2,board,sizeof(board));
            int ko_z_copy2=ko_z;
            
            int win=-playout(flip_color(color));
            win_sum+=win;
            //print_board(); //プレイアウトを打ち切った局面が表示される
            //printf("win=%d,%d\n",win,win_sum);
            memcpy(board,board_copy2,sizeof(board)); //局面を戻す
            ko_z=ko_z_copy2;
        }
        double win_rate=(double)win_sum/try_num; //勝率を求める
        //print_board();
        //printf("z=%d,win=%5.3f\n",get81(z),win_rate);
        
        if(win_rate>best_value){    //最善手を更新
            best_value=win_rate;
            best_z=z;
            printf("best_z=%d,v=%5.3f,try_num=%d\n",get81(best_z),best_value,try_num);
        }
        
        memcpy(board,board_copy,sizeof(board)); //最初の局面に戻す
        ko_z=ko_z_copy;
    }
    return best_z;
}

//人間の手を入力する関数
int i_move(int t)
{
    
    char bb[80];
    int i;
    int g;
    
    do{
        fgets(bb,79,stdin);
        i=atoi(bb);
    }while(i>99);//０（パス)の入力も可能にするため、i<11とはしなかった。要対策
    
    g=get_t(i);
    
    return g;
    
}

//ここからの追加データ構造は、UCTで探索するプログラム専用

//1手の情報を保持する構造体
typedef struct child{
    int z;//手の場所
    int games;//この手を選んだ回数
    double rate;//この手の勝率
    int next;//この手を打ったあとのノード番号
}CHILD;

const int CHILD_MAX=B_SIZE*B_SIZE+1;//+1はPASS用

//ある一つの局面状態（ノード）を保持する構造体
typedef struct node{
    int child_num;//子局面の数
    CHILD child[CHILD_MAX];
    int games_sum;//子局面の回数の合計
}NODE;

const int NODE_MAX=10000;//最大登録ノード数
NODE node[NODE_MAX];

int node_num=0;//登録ノード数
const int NODE_EMPTY=-1;//次のノードが存在しない場合
const int ILLEGAL_Z=-1;//ルール違反の手

//ノードを作成する関数
void add_child(NODE *pN,int z)
{
    int n=pN->child_num;
    pN->child[n].z=z;
    pN->child[n].games=0;
    pN->child[n].rate=0;
    pN->child[n].next=NODE_EMPTY;
    pN->child_num++;
}

//ノードを作成する関数

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
    add_child(pN,0);//PASSも追加
    node_num++;
    return node_num-1;//作成したノード番号を返す
}

//UCBで手を選び降りていく関数

int search_uct(int color,int node_n)
{
    NODE *pN=&node[node_n];
re_try:
    //UCBが一番高い手を選ぶ
    int select=-1;
    double max_ucb=-999;
    int i;
    for(i=0;i<pN->child_num;i++){
        CHILD *c=&pN->child[i];
        if(c->z==ILLEGAL_Z)continue;
        double ucb=0;
        if(c->games==0){
            ucb=10000+rand();//未展開
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
    int err=move(z,color);//打ってみる
    if(err!=0){//エラー
        c->z=ILLEGAL_Z;
        goto re_try;//別な手を生成
    }
    
    int win;
    if(c->games==0){//最初の１回目はプレイアウト
        win=-playout(flip_color(color));
    }else{
        if(c->next==NODE_EMPTY)c->next=create_node();
        win=-search_uct(flip_color(color),c->next);
    }
    //勝率を更新
    c->rate=(c->rate*c->games+win)/(c->games+1);
    c->games++;//この手の回数を更新
    pN->games_sum++;//全体の回数を更新
    return win;
}


//UCTを繰り返す関数

int select_best_uct(int color)
{
    node_num=0;
    int next=create_node();//root局面のノードを作成
    
    int uct_loop=10000;//UCTを繰り返す回数
    int i;
    for(i=0;i<uct_loop;i++){
        int board_copy[BOARD_MAX];//局面を保存
        memcpy(board_copy,board,sizeof(board));
        int ko_z_copy=ko_z;
        
        search_uct(color,next);
        
        memcpy(board,board_copy,sizeof(board));//局面を戻す
        ko_z=ko_z_copy;
    }
    
    int best_i=-1;
    int max=-999;
    NODE *pN=&node[next];
    for(i=0;i<pN->child_num;i++){
        CHILD *c=&pN->child[i];
        if(c->games>max){//最大回数の手を選ぶ
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
    int color=1; //現在の手番の色、黒が1で白が2
    int tesuu=0; //手数
    char bb[80];
    int i;
    
    srand((unsigned)time(NULL)); //現在時刻で乱数を初期化
    //loop:
    //盤面初期化
    {int i,x,y;for(i=0;i<BOARD_MAX;i++)board[i]=3;
        for(y=0;y<B_SIZE;y++)for(x=0;x<B_SIZE;x++)
            board[get_z(x,y)]=0;}
    
    
    
    //static int score_sum;
    //static int loop_count;
    
    
    
    //黒か白か選択
    do{
        printf("1:黒\n");
        printf("2:白\n");
		printf("黒を持ちたい場合は１を、白を持ちたい場合は２を入力してください\n");
        
        fgets(bb,79,stdin);
        
        i=atoi(bb);
    }while(i!=1&&i!=2);
    
    
    switch(i){
            //自分がの白の場合（コンピュータが黒）
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
            if(tesuu>300)break;//3コウでのループ対策
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
            
            //自分が黒 の場合（コンピュータが白)
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
            //int z=select_best_move(color);
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




