//�͌�i�X�H�Ձj�@�����e�J�����@���x�[�X�ɍ쐬
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<math.h>

double komi=2.5;
const int B_SIZE=9;  //��Ղ̑傫��
const int WIDTH=B_SIZE+2; //�g���܂߂�����
const int BOARD_MAX=WIDTH*WIDTH;

int board[BOARD_MAX]={//�����ՖʁA�ՊO��3�Ŏ���
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


int dir4[4]={+1,-1,+WIDTH,-WIDTH}; //�E�A���A���A��ւ̈ړ���
int hama[2]; //������΂̐�
int kifu[1000]; //���܂ł̎菇
int ko_z; //�R�E�őłĂȂ��ʒu
int all_playouts; //pleyout���s������

//(x,y)���W��1�����\�L�ɕϊ�
int get_z(int x, int y)
{
    return (y+1)*WIDTH+(x+1); //0<=x <=8, 0<=y<=8
}

//�\���p�ɍ��W��ϊ�
int get81(int z)
{
    if(z==0)return 0;
    int y=z/WIDTH; 
    int x=z-y*WIDTH; 
    return x*10+y; 
}

//�\���p�\�L����ꎟ���\�L�ɕϊ�
int get_t(int z)
{
    int t;
    int i;
    int x;
    int y;
    
    //�\���p�\�L��K�؂�x,y�ɕ�������
    x=z/10;
    i=x*10;
    y=z-i;
    
    //x,y���g���Ĉꎟ���\�L�����߂�
    t=(y*11)+x;
    
    return t;
    
}

//�F�̔��]
int flip_color(int col){
    return 3-col; //�΂̐F�𔽓]������1->2,2->1
}

int check_board[BOARD_MAX]; //�����ς݃t���O.�΂̂���Ȃ��ŕ��p

//�_���̐��𐔂���֐�

//�_���Ɛΐ��𐔂���ċA�֐�
//�S�����𒲂ׂāA�󔒂Ȃ�+1,�����̐΂Ȃ�ċA��.����̐΁A�ǂ͂��̂܂�
void count_dame_sub(int tz,int color,int *p_dame,int *p_ishi)
{
    int z,i;
    check_board[tz]=1; //���̈ʒu�i�΁j�͌����ς�
    (*p_ishi)++; //�΂̐�
    for(i=0;i<4;i++){
        z=tz+dir4[i]; //�S�����𒲂ׂ�
        if(check_board[z])continue;
        if(board[z]==0){
            check_board[z]=1;//���̈ʒu�i��_�j�̓J�E���g�ς݂�
            (*p_dame)++;  //�_���̐�
        }
        //���T���̎����̐�
        if(board[z]==color)count_dame_sub(z,color,p_dame,p_ishi);
    }
}


//�ʒu�� tz �ɂ�����_���̐��Ɛ΂̐����v�Z
void count_dame(int tz,int *p_dame,int *p_ishi)
{
    int i;
    *p_dame=*p_ishi=0;
    for(i=0;i<BOARD_MAX;i++)check_board[i]=0;
    count_dame_sub(tz,board[tz],p_dame,p_ishi);
}

//�΂������֐�
void kesu(int tz,int color)
{
    int z,i;
    
    board[tz]=0; //�΂�����
    for(i=0;i<4;i++){
        z=tz+dir4[i];
        if(board[z]==color)kesu(z,color);
    }
}


//�΂�u���֐�
int move(int tz,int color)
{
    if(tz==0){ko_z=0;return 0;} //�p�X�̏ꍇ
    
    int around[4][3]; //�S�����̃_�����A�ΐ��A�F
    int un_col=flip_color(color); //����̐΂̐�
    
    //�S�����̐΂̃_���Ɛΐ��𒲂ׂ�
    int space=0; //�S�����̋󔒂̐�
    int kabe=0; //4�����̔ՊO�̐�
    int mikata_safe=0; //�_���Q�ȏ�ň��S�Ȗ����̐�
    int take_sum=0; //����΂̍��v
    int ko_kamo=0; //�R�E�ɂȂ邩������Ȃ��ꏊ
    int i;
    for(i=0;i<4;i++){
        around[i][0]=around[i][1]=around[i][2]=0;
        int z=tz+dir4[i];
        int c=board[z]; //�΂̐�
        if(c==0)space++;
        if(c==3)kabe++;
        if(c==0||c==3)continue;
        int dame; //�_���̐�
        int ishi; //�΂̐�
        count_dame(z,&dame,&ishi);
        around[i][0]=dame;
        around[i][1]=ishi;
        around[i][2]=c;
        if(c==un_col &&dame==1){take_sum+=ishi;ko_kamo=z;}
        if(c==color && dame>=2)mikata_safe++;
    }
    //���E��
    if(take_sum==0&&space==0&&mikata_safe==0)return 1;
    if(tz==ko_z) return 2; //�R�E
    if(kabe+mikata_safe==4)return 3; //��i���[���ᔽ�ł͂Ȃ��j
    if(board[tz]!=0)return 4; //���ɐ΂�����
    
    for(i=0;i<4;i++){
        int d=around[i][0];
        int n=around[i][1];
        int c=around[i][2];
        if(c==un_col&&d==1&&board[tz+dir4[i]]){
            kesu(tz+dir4[i],un_col);  //�΂�����
            hama[color-1]+=n;
        }
    }
    board[tz]=color; //�΂�u��
    int dame,ishi;
    count_dame(tz,&dame,&ishi);
    if(take_sum==1&&ishi==1&&dame==1){//�R�E�ɂȂ�
        ko_z=ko_kamo;
    }else{
        ko_z=0;
    }
    return 0;
}


//�Ֆʂ�\������֐�
void print_board()
{
    int x,y;
    char *str[3]={"�E","��","��"};
    
    printf("  ");  //���̕����̋󔒂̐����Ֆʂ����ꂢ�ɕ\��������|�C���g
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


//�n�𐔂��ď�������������Ԃ��֐�
int count_score(int turn_color)
{
    int score=0;
    int kind[3]; //�Տ�Ɏc���Ă�ΐ�
    kind[0]=kind[1]=kind[2]=0;
    int x,y,i;
    for(y=0;y<B_SIZE;y++)for(x=0;x<B_SIZE;x++){
        int z=get_z(x,y);
        int c=board[z];
        kind[c]++;
        if(c!=0)continue;
        int mk[4]; //��_�͂S�����̐΂���ޕʂɐ�����
        mk[1]=mk[2]=0; //mk[0] ��,[1] ��, [2] ��, [3] �ՊO
        for(i=0;i<4;i++)mk[board[z+dir4[i]] ]++;
        if(mk[1]&&mk[2]==0)score++;//�������Ɉ͂܂�Ă���΍��n
        if(mk[2]&&mk[1]==0)score--;//�������Ɉ͂܂�Ă���Δ��n
    }
    score+=kind[1]-kind[2]; //�n+�Տ�̐ΐ�
    
    double final_score=score-komi;
    int win=0;
    if(final_score>0)win=1; //���������Ă���΂P
    //printf("win=%d,score=%d\n",win,score);
    //win=score; //�����ł͂Ȃ��ڐ�����Ԃ��ꍇ
    if(turn_color==2)win=-win; //���Ԃł͔��]������
    return win;
}

//�v���C�A�E�g���s���֐�
int playout(int turn_color)
{
    all_playouts++; //�v���C�A�E�g�֐����Ă΂ꂽ��
    int color=turn_color;
    int before_z=-1; //1��O�̎�
    int loop;
    int loop_max=B_SIZE*B_SIZE+200; //�R�R�E�΍�Ŏ萔�𐧌�
    for(loop=0;loop<loop_max;loop++){
        //���ׂĂ̋�_�𒅎���ɂ���
        int kouho[BOARD_MAX];
        int kouho_num=0;
        int x,y;
        for (y=0;y<B_SIZE;y++)for(x=0;x<B_SIZE;x++){
            int z=get_z(x,y);
            if (board[z]!=0)continue;
            kouho[kouho_num]=z;
            kouho_num++;
        }
        int z,r=0; //r�����������ĂȂ��G���[���ł��̂ŁA�����Ƃ�����������
        for(;;){//����\�Ȏ���P�茩����܂Ń��[�v
            if(kouho_num==0){
                z=0;
            }else{
                r=rand()%kouho_num; //�����łP��I��
                z=kouho[r];
            }
            int err=move(z,color);
            if(err==0)break; //����\�Ȃ̂ł��̎��I��
            kouho[r]=kouho[kouho_num-1]; //���̎���폜.�����̎����
            kouho_num--;
        }
        if(z==0&&before_z==0)break; //�A���p�X
        before_z=z;
        //print_board();//�v���C�A�E�g�r���̗l�q��\��
        //printf("loop=%d,z=%d,c=%d,kouho_num=%d,ko_z=%d\n",loop,get81(z),color,kouho_num,get81(ko_z));
        color=flip_color(color); //��Ԃ����ւ���
    }
    return count_score(turn_color);//�v���C�A�E�g���J�n������Ԃ�n��
}

//�\�Ȏ�ɑ΂��ăv���C�A�E�g���s����
int select_best_move(int color)
{
    int try_num=10000; //�v���C�A�E�g���J��Ԃ���
    int best_z=0; //�őP��
    double best_value=-100;
    
    int board_copy[BOARD_MAX];
    memcpy(board_copy,board,sizeof(board)); //���ǖʂ�ۑ�
    int ko_z_copy=ko_z;
    
    //���ׂĂ̋�_�𒅎����
    int x,y;
    for(y=0;y<B_SIZE;y++)for(x=0;x<B_SIZE;x++){
        int z=get_z(x,y);
        if(board[z]!=0)continue; //�΂�����
        
        int err=move(z,color); //�ł��Ă݂�
        if(err!=0)continue;
        
        int win_sum=0; //�������񐔂̍��v
        int i;
        for(i=0;i<try_num;i++){ //�v���C�A�E�g���J��Ԃ�
            int board_copy2[BOARD_MAX]; //�ǖʂ�����̂ŕۑ�
            memcpy(board_copy2,board,sizeof(board));
            int ko_z_copy2=ko_z;
            
            int win=-playout(flip_color(color));
            win_sum+=win;
            //print_board(); //�v���C�A�E�g��ł��؂����ǖʂ��\�������
            //printf("win=%d,%d\n",win,win_sum);
            memcpy(board,board_copy2,sizeof(board)); //�ǖʂ�߂�
            ko_z=ko_z_copy2;
        }
        double win_rate=(double)win_sum/try_num; //���������߂�
        //print_board();
        //printf("z=%d,win=%5.3f\n",get81(z),win_rate);
        
        if(win_rate>best_value){    //�őP����X�V
            best_value=win_rate;
            best_z=z;
            printf("best_z=%d,v=%5.3f,try_num=%d\n",get81(best_z),best_value,try_num);
        }
        
        memcpy(board,board_copy,sizeof(board)); //�ŏ��̋ǖʂɖ߂�
        ko_z=ko_z_copy;
    }
    return best_z;
}

//�l�Ԃ̎����͂���֐�
int i_move(int t)
{
    
    char bb[80];
    int i;
    int g;
    
    do{
        fgets(bb,79,stdin);
        i=atoi(bb);
    }while(i>99);//�O�i�p�X)�̓��͂��\�ɂ��邽�߁Ai<11�Ƃ͂��Ȃ������B�v�΍�
    
    g=get_t(i);
    
    return g;
    
}

//��������̒ǉ��f�[�^�\���́AUCT�ŒT������v���O������p

//1��̏���ێ�����\����
typedef struct child{
    int z;//��̏ꏊ
    int games;//���̎��I�񂾉�
    double rate;//���̎�̏���
    int next;//���̎��ł������Ƃ̃m�[�h�ԍ�
}CHILD;

const int CHILD_MAX=B_SIZE*B_SIZE+1;//+1��PASS�p

//�����̋ǖʏ�ԁi�m�[�h�j��ێ�����\����
typedef struct node{
    int child_num;//�q�ǖʂ̐�
    CHILD child[CHILD_MAX];
    int games_sum;//�q�ǖʂ̉񐔂̍��v
}NODE;

const int NODE_MAX=10000;//�ő�o�^�m�[�h��
NODE node[NODE_MAX];

int node_num=0;//�o�^�m�[�h��
const int NODE_EMPTY=-1;//���̃m�[�h�����݂��Ȃ��ꍇ
const int ILLEGAL_Z=-1;//���[���ᔽ�̎�

//�m�[�h���쐬����֐�
void add_child(NODE *pN,int z)
{
    int n=pN->child_num;
    pN->child[n].z=z;
    pN->child[n].games=0;
    pN->child[n].rate=0;
    pN->child[n].next=NODE_EMPTY;
    pN->child_num++;
}

//�m�[�h���쐬����֐�

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
    add_child(pN,0);//PASS���ǉ�
    node_num++;
    return node_num-1;//�쐬�����m�[�h�ԍ���Ԃ�
}

//UCB�Ŏ��I�э~��Ă����֐�

int search_uct(int color,int node_n)
{
    NODE *pN=&node[node_n];
re_try:
    //UCB����ԍ������I��
    int select=-1;
    double max_ucb=-999;
    int i;
    for(i=0;i<pN->child_num;i++){
        CHILD *c=&pN->child[i];
        if(c->z==ILLEGAL_Z)continue;
        double ucb=0;
        if(c->games==0){
            ucb=10000+rand();//���W�J
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
    int err=move(z,color);//�ł��Ă݂�
    if(err!=0){//�G���[
        c->z=ILLEGAL_Z;
        goto re_try;//�ʂȎ�𐶐�
    }
    
    int win;
    if(c->games==0){//�ŏ��̂P��ڂ̓v���C�A�E�g
        win=-playout(flip_color(color));
    }else{
        if(c->next==NODE_EMPTY)c->next=create_node();
        win=-search_uct(flip_color(color),c->next);
    }
    //�������X�V
    c->rate=(c->rate*c->games+win)/(c->games+1);
    c->games++;//���̎�̉񐔂��X�V
    pN->games_sum++;//�S�̂̉񐔂��X�V
    return win;
}


//UCT���J��Ԃ��֐�

int select_best_uct(int color)
{
    node_num=0;
    int next=create_node();//root�ǖʂ̃m�[�h���쐬
    
    int uct_loop=10000;//UCT���J��Ԃ���
    int i;
    for(i=0;i<uct_loop;i++){
        int board_copy[BOARD_MAX];//�ǖʂ�ۑ�
        memcpy(board_copy,board,sizeof(board));
        int ko_z_copy=ko_z;
        
        search_uct(color,next);
        
        memcpy(board,board_copy,sizeof(board));//�ǖʂ�߂�
        ko_z=ko_z_copy;
    }
    
    int best_i=-1;
    int max=-999;
    NODE *pN=&node[next];
    for(i=0;i<pN->child_num;i++){
        CHILD *c=&pN->child[i];
        if(c->games>max){//�ő�񐔂̎��I��
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
    int color=1; //���݂̎�Ԃ̐F�A����1�Ŕ���2
    int tesuu=0; //�萔
    char bb[80];
    int i;
    
    srand((unsigned)time(NULL)); //���ݎ����ŗ�����������
    //loop:
    //�Ֆʏ�����
    {int i,x,y;for(i=0;i<BOARD_MAX;i++)board[i]=3;
        for(y=0;y<B_SIZE;y++)for(x=0;x<B_SIZE;x++)
            board[get_z(x,y)]=0;}
    
    
    
    //static int score_sum;
    //static int loop_count;
    
    
    
    //���������I��
    do{
        printf("1:��\n");
        printf("2:��\n");
		printf("�������������ꍇ�͂P���A�������������ꍇ�͂Q����͂��Ă�������\n");
        
        fgets(bb,79,stdin);
        
        i=atoi(bb);
    }while(i!=1&&i!=2);
    
    
    switch(i){
            //�������̔��̏ꍇ�i�R���s���[�^�����j
        case 2:for(;;){
            color = 1;
			print_board();
            
			printf("�R���s���[�^�v�l��\n");
            clock_t bt=clock();
            all_playouts=0; //playout�̉񐔂�������
            //int z=select_best_move(color);
            int z=select_best_uct(color);
            int err=move(z,color); //�ł�
            if(err!=0){printf("Err!\n");exit(0);}
            kifu[tesuu++]=z;
            print_board();
            printf("play_z=%d, �萔=%d, �F=%d, all_playouts=%d\n",get81(z),tesuu,color,all_playouts);
            double t=(double)(clock()+1-bt)/CLOCKS_PER_SEC;
            printf("%.1f�b,%.0f playout/�b\n",t,all_playouts/t);
            if(z==0&&tesuu>1&&kifu[tesuu-2]==0)break;
            if(tesuu>300)break;//3�R�E�ł̃��[�v�΍�
            color=flip_color(color);
            
			printf("���I�����Ă�������\n");
            int zi=i_move(color);
            err=move(zi,color);
            if(err!=0){printf("Err!\n");exit(0);}
            kifu[tesuu++]=zi;
            print_board();
            if(zi==0&&tesuu>1&&kifu[tesuu-2]==0)break;
            color=flip_color(color);
        };
            break;
            
            //�������� �̏ꍇ�i�R���s���[�^����)
        case 1:for(;;){
            color = 1;
			print_board();
			printf("���I�����Ă�������\n");
            int zi=i_move(color);
            int err=move(zi,color);
            if(err!=0){printf("Err!\n");exit(0);}
            kifu[tesuu++]=zi;
            print_board();
            if(zi==0&&tesuu>1&&kifu[tesuu-2]==0)break;
            color=flip_color(color);
            
			printf("�R���s���[�^�v�l��\n");
            clock_t bt=clock();
            all_playouts=0; 
            //int z=select_best_move(color);
            int z=select_best_uct(color);
            err=move(z,color); 
            if(err!=0){printf("Err!\n");exit(0);}
            kifu[tesuu++]=z;
            print_board();
            printf("play_z=%d, �萔=%d, �F=%d, all_playouts=%d\n",get81(z),tesuu,color,all_playouts);
            double t=(double)(clock()+1-bt)/CLOCKS_PER_SEC;
            printf("%.1f�b,%.0f playout/�b\n",t,all_playouts/t);
            if(z==0&&tesuu>1&&kifu[tesuu-2]==0)break;
            if(tesuu>300)break;
            color=flip_color(color);
            
            
            
        };
            break;
            
        default: printf("Err!\n");
            
    }
    
    
    
    
   
     
    return 0;
    
    
}




