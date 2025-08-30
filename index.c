#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INCOME 0
#define EXPENSE 1
#define MAX_CAT 24
#define MAX_NOTE 64

typedef struct {
	int y, m, d;
	int type;
	double amount;
	char category[MAX_CAT];
	char note[MAX_NOTE];
} Transaction;

typedef struct {
	Transaction *a;
	size_t n, cap;
} TxList;

void push(TxList *L, Transaction t) {
	if (L->n == L->cap) {
		size_t newcap = L->cap ? L->cap * 2 : 8;
		Transaction *newa = realloc(L->a, newcap * sizeof(Transaction));
		if (!newa) { perror("realloc"); exit(1); }
		L->a = newa;
		L->cap = newcap;
	}
	L->a[L->n++] = t;
}

int cmp_date(const void *a, const void *b) {
	const Transaction *ta = (const Transaction *)a;
	const Transaction *tb = (const Transaction *)b;
	if (ta->y != tb->y) return ta->y - tb->y;
	if (ta->m != tb->m) return ta->m - tb->m;
	return ta->d - tb->d;
}

void save_csv(const char *path, TxList *L){
FILE *f=fopen(path,"w"); if(!f){perror("open"); return;}
fprintf(f,"y,m,d,type,amount,category,note\n");
for(size_t i=0;i<L->n;i++){
Transaction t=L->a[i];
fprintf(f,"%d,%d,%d,%d,%.2f,%s,%s\n",t.y,t.m,t.d,t.type,t.amount,t.category,t.note);
}
fclose(f);
}


void load_csv(const char *path, TxList *L){
FILE *f=fopen(path,"r"); if(!f){perror("open"); return;}
char line[256]; fgets(line,sizeof line,f); // header
while(fgets(line,sizeof line,f)){
Transaction t; char cat[MAX_CAT], note[MAX_NOTE];
if(sscanf(line, "%d,%d,%d,%d,%lf,%23[^,],%63[^\n]", &t.y,&t.m,&t.d,(int*)&t.type,&t.amount,cat,note)==7){
strncpy(t.category,cat,MAX_CAT); t.category[MAX_CAT-1]='\0';
strncpy(t.note,note,MAX_NOTE); t.note[MAX_NOTE-1]='\0';
push(L,t);
}
}
fclose(f);
}


void filter_expenses_over(TxList *L, double thr){
for(size_t i=0;i<L->n;i++) if(L->a[i].type==EXPENSE && L->a[i].amount>thr){
Transaction *t=&L->a[i];
printf("%04d-%02d-%02d | %-10s | %8.2f | %s\n", t->y,t->m,t->d,t->category,t->amount,t->note);
}
}


void ascii_month_chart(TxList *L, int year){
double msum[13]={0};
for(size_t i=0;i<L->n;i++) if(L->a[i].type==EXPENSE && L->a[i].y==year) msum[L->a[i].m]+=L->a[i].amount;
double mx=0; for(int m=1;m<=12;m++) if(msum[m]>mx) mx=msum[m];
if(mx==0){ puts("No expenses to chart."); return; }
for(int m=1;m<=12;m++){
int bars=(int)(40*msum[m]/mx);
printf("%02d | ",m);
for(int i=0;i<bars;i++) putchar('#');
printf(" %.2f\n",msum[m]);
}
}


int main(){
TxList L={0};
// sample data
push(&L,(Transaction){2025,8,1,INCOME,1200.00,"Salary","August"});
push(&L,(Transaction){2025,8,2,EXPENSE,150.50,"Food","Groceries"});
push(&L,(Transaction){2025,8,3,EXPENSE,80.00,"Transport","Bus pass"});


qsort(L.a,L.n,sizeof *L.a,cmp_date);
filter_expenses_over(&L,100.0);


save_csv("data/transactions.csv",&L);
ascii_month_chart(&L,2025);


free(L.a);
return 0;
}