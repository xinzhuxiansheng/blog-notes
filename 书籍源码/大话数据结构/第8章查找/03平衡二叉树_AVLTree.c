#include "stdio.h"    
#include "stdlib.h"   
#include "io.h"  
#include "math.h"  
#include "time.h"

#define OK 1
#define ERROR 0
#define TRUE 1
#define FALSE 0
#define MAXSIZE 100 /* �洢�ռ��ʼ������ */

typedef int Status;	/* Status�Ǻ���������,��ֵ�Ǻ������״̬���룬��OK�� */ 


/* �������Ķ���������ṹ���� */
typedef  struct BiTNode	/* ���ṹ */
{
	int data;	/* ������� */
	int bf; /*  ����ƽ������ */ 
	struct BiTNode *lchild, *rchild;	/* ���Һ���ָ�� */
} BiTNode, *BiTree;


/* ����pΪ���Ķ������������������� */
/* ����֮��pָ���µ�������㣬����ת����֮ǰ���������ĸ���� */
void R_Rotate(BiTree *P)
{ 
	BiTree L;
	L=(*P)->lchild; /*  Lָ��P������������� */ 
	(*P)->lchild=L->rchild; /*  L���������ҽ�ΪP�������� */ 
	L->rchild=(*P);
	*P=L; /*  Pָ���µĸ���� */ 
}

/* ����PΪ���Ķ������������������� */
/* ����֮��Pָ���µ�������㣬����ת����֮ǰ���������ĸ����0  */
void L_Rotate(BiTree *P)
{ 
	BiTree R;
	R=(*P)->rchild; /*  Rָ��P������������� */ 
	(*P)->rchild=R->lchild; /* R���������ҽ�ΪP�������� */ 
	R->lchild=(*P);
	*P=R; /*  Pָ���µĸ���� */ 
}

#define LH +1 /*  ��� */ 
#define EH 0  /*  �ȸ� */ 
#define RH -1 /*  �Ҹ� */ 

/*  ����ָ��T��ָ���Ϊ���Ķ���������ƽ����ת���� */
/*  ���㷨����ʱ��ָ��Tָ���µĸ���� */
void LeftBalance(BiTree *T)
{ 
	BiTree L,Lr;
	L=(*T)->lchild; /*  Lָ��T������������� */ 
	switch(L->bf)
	{ /*  ���T����������ƽ��ȣ�������Ӧƽ�⴦�� */ 
		 case LH: /*  �½�������T�����ӵ��������ϣ�Ҫ������������ */ 
			(*T)->bf=L->bf=EH;
			R_Rotate(T);
			break;
		 case RH: /*  �½�������T�����ӵ��������ϣ�Ҫ��˫������ */ 
			Lr=L->rchild; /*  Lrָ��T�����ӵ��������� */ 
			switch(Lr->bf)
			{ /*  �޸�T�������ӵ�ƽ������ */ 
				case LH: (*T)->bf=RH;
						 L->bf=EH;
						 break;
				case EH: (*T)->bf=L->bf=EH;
						 break;
				case RH: (*T)->bf=EH;
						 L->bf=LH;
						 break;
			}
			Lr->bf=EH;
			L_Rotate(&(*T)->lchild); /*  ��T��������������ƽ�⴦�� */ 
			R_Rotate(T); /*  ��T������ƽ�⴦�� */ 
	}
}

/*  ����ָ��T��ָ���Ϊ���Ķ���������ƽ����ת���� */ 
/*  ���㷨����ʱ��ָ��Tָ���µĸ���� */ 
void RightBalance(BiTree *T)
{ 
	BiTree R,Rl;
	R=(*T)->rchild; /*  Rָ��T������������� */ 
	switch(R->bf)
	{ /*  ���T����������ƽ��ȣ�������Ӧƽ�⴦�� */ 
	 case RH: /*  �½�������T���Һ��ӵ��������ϣ�Ҫ������������ */ 
			  (*T)->bf=R->bf=EH;
			  L_Rotate(T);
			  break;
	 case LH: /*  �½�������T���Һ��ӵ��������ϣ�Ҫ��˫������ */ 
			  Rl=R->lchild; /*  Rlָ��T���Һ��ӵ��������� */ 
			  switch(Rl->bf)
			  { /*  �޸�T�����Һ��ӵ�ƽ������ */ 
				case RH: (*T)->bf=LH;
						 R->bf=EH;
						 break;
				case EH: (*T)->bf=R->bf=EH;
						 break;
				case LH: (*T)->bf=EH;
						 R->bf=RH;
						 break;
			  }
			  Rl->bf=EH;
			  R_Rotate(&(*T)->rchild); /*  ��T��������������ƽ�⴦�� */ 
			  L_Rotate(T); /*  ��T������ƽ�⴦�� */ 
	}
}

/*  ����ƽ��Ķ���������T�в����ں�e����ͬ�ؼ��ֵĽ�㣬�����һ�� */ 
/*  ����Ԫ��Ϊe���½�㣬������1�����򷵻�0����������ʹ���������� */ 
/*  ʧȥƽ�⣬����ƽ����ת������������taller��ӳT������� */
Status InsertAVL(BiTree *T,int e,Status *taller)
{  
	if(!*T)
	{ /*  �����½�㣬�������ߡ�����tallerΪTRUE */ 
		 *T=(BiTree)malloc(sizeof(BiTNode));
		 (*T)->data=e; (*T)->lchild=(*T)->rchild=NULL; (*T)->bf=EH;
		 *taller=TRUE;
	}
	else
	{
		if (e==(*T)->data)
		{ /*  �����Ѵ��ں�e����ͬ�ؼ��ֵĽ�����ٲ��� */ 
			*taller=FALSE; return FALSE;
		}
		if (e<(*T)->data)
		{ /*  Ӧ������T���������н������� */ 
			if(!InsertAVL(&(*T)->lchild,e,taller)) /*  δ���� */ 
				return FALSE;
			if(*taller) /*   �Ѳ��뵽T�����������������������ߡ� */ 
				switch((*T)->bf) /*  ���T��ƽ��� */ 
				{
					case LH: /*  ԭ�����������������ߣ���Ҫ����ƽ�⴦�� */ 
							LeftBalance(T);	*taller=FALSE; break;
					case EH: /*  ԭ�����������ȸߣ��������������߶�ʹ������ */ 
							(*T)->bf=LH; *taller=TRUE; break;
					case RH: /*  ԭ�����������������ߣ������������ȸ� */  
							(*T)->bf=EH; *taller=FALSE; break;
				}
		}
		else
		{ /*  Ӧ������T���������н������� */ 
			if(!InsertAVL(&(*T)->rchild,e,taller)) /*  δ���� */ 
				return FALSE;
			if(*taller) /*  �Ѳ��뵽T���������������������ߡ� */ 
				switch((*T)->bf) /*  ���T��ƽ��� */ 
				{
					case LH: /*  ԭ�����������������ߣ������������ȸ� */ 
							(*T)->bf=EH; *taller=FALSE;	break;
					case EH: /*  ԭ�����������ȸߣ��������������߶�ʹ������  */
							(*T)->bf=RH; *taller=TRUE; break;
					case RH: /*  ԭ�����������������ߣ���Ҫ����ƽ�⴦�� */ 
							RightBalance(T); *taller=FALSE; break;
				}
		}
	}
	return TRUE;
}

int main(void)
{    
	int i;
	int a[10]={3,2,1,4,5,6,7,10,9,8};
	BiTree T=NULL;
	Status taller;
	for(i=0;i<10;i++)
	{
		InsertAVL(&T,a[i],&taller);
	}
	printf("����������ϵ���ٲ鿴ƽ��������ṹ");
	return 0;
}