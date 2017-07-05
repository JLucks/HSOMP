#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

#define LIM 1001313

void DownHeapS(int *pS,int k, int nS) {
    int j, v;
    if (k <= nS/2) {
        v = pS[k];
        for (j=2*k, j+=(pS[j-1] > pS[j])?1:0; pS[j] > v; k=j, j=2*k, j+=(pS[j-1] > pS[j])?1:0){
            pS[k] = pS[j];
            if (j > nS/2) {
                k = j;
                break;
            }
        }
        pS[k] = v;
    }
}

void DownHeapL(int *pL, int k, int nL) {
    int j, v;
    if (k <= nL/2) {
        v = pL[k];
        for (j=2*k, j+=(pL[j+1] < pL[j])?1:0; pL[j] < v; k=j, j=2*k, j+=(pL[j+1] < pL[j])?1:0){
            pL[k] = pL[j];
            if (j > nL/2) {
                k = j;
                break;
            }
        }
        pL[k] = v;
    }
}

void TreeSwap(int *pS, int kS,int nS, int *pL, int kL, int nL) {
    int jS, jL, tmp;
    jS = 2 * kS;
    jL = 2 * kL;
    if ((jS<=nS) && (jL<=nL)) {
        jS += (pS[jS-1] > pS[jS])?1:0;
        jL += (pL[jL+1] < pL[jL])?1:0;
        if (pS[jS] > pL[jL]) {
            TreeSwap(pS, jS, nS, pL, jL, nL);
            if (pS[jS^1] > pL[jL^1])
                TreeSwap(pS, jS^1, nS, pL, jL^1, nL);
        }
    }
    tmp = pS[kS];
    pS[kS] = pL[kL];
    pL[kL] = tmp;
    DownHeapS(pS,kS,nS);
    DownHeapL(pL,kL,nL);
}

void PartitionHeap(int *ph, int n, int numThread) {
    int i, j, tmp;
    int nS, nL;
    int *pS, *pL;
#pragma omp parallel num_threads(numThread)
{
    /* Construct small subheap. */
    nS = (n / 2) & ~1;
    pS = ph + nS + 1;
#pragma omp for
    for (i=nS/2; i>0; i--)
        DownHeapS(pS,i,nS);
    /* Construct large subheap. */
    nL = n - nS;
    pL = ph + nS;
#pragma omp for
    for (j=nL/2; j>0; j--)
        DownHeapL(pL,j,nL);
}
    /* Exchange (TreeSwap phase). */
    while (pS[1] > pL[1])
        TreeSwap(pS, 1, nS, pL, 1, nL);
    /* Set aside sorted elements. */
    if (pS[2] < pS[3]) {
        tmp=pS[2];pS[2]=pS[3];pS[3]=tmp;
    }
    if (pL[2] > pL[3]) {
        tmp=pL[2]; pL[2]=pL[3]; pL[3]=tmp;
    }
    /* Continue partitioning. */
    if (nS > 3)
        PartitionHeap(ph, nS-2, numThread);
    if (nL > 3)
        PartitionHeap(ph+nS+2, nL-2, numThread);
}

void DualheapSort(int *ph, int n, int numThread) {
    int i, tmp, nL;
    /* Construct initial heap. */
    int *pL;
    pL = ph;
    nL = n;
    for (i=nL/2; i>0; i--)
        DownHeapL(pL,i,nL);
    /* Set aside two sorted elements. */
    if (pL[2] > pL[3]) {
        tmp=pL[2]; pL[2]=pL[3]; pL[3]=tmp;
    }
    /* Recursively partition. */
    if (n > 3)
        PartitionHeap(pL+2, n-2, numThread);
}

void lerValores(char *url, int *C, int tam){
    FILE *arq;
    int i = 0;
	arq = fopen(url, "r");
	if(arq == NULL)
			printf("Erro, nao foi possivel abrir o arquivo\n");
	else{
		while( (fscanf(arq,"%d\n", &C[i]))!=EOF ){
            i++;
            if(i == tam)
                break;
		}
	}
	fclose(arq);
}

char *replace_str(char *str, char *orig, char *rep)
{
  static char buffer[4096];
  char *p;

  if(!(p = strstr(str, orig)))  // Is 'orig' even in 'str'?
    return str;

  strncpy(buffer, str, p-str); // Copy characters from 'str' start to 'orig' st$
  buffer[p-str] = '\0';

  sprintf(buffer+(p-str), "%s%s", rep, p+strlen(orig));

  return buffer;
}

void imprimir(int *C, int tam, float temp, int numThread){
    int i;
    FILE *arq;
    char *url;
    char str[10];
    sprintf(str,"%d-%d",tam,numThread);
    url = (char*)malloc(sizeof(char)*100);
    strcpy(url, "hsomp-");
    strcat(url, str);
    strcat(url, "elementos-");
    strcat(url, __DATE__);
    strcat(url, "-");
    strcat(url, replace_str(replace_str(__TIME__,":","-"),":","-"));
    strcat(url, ".txt");
    printf("Arquivo de Saida:%s\n",url);
    arq = fopen(url,"r");
    if(arq == NULL)
        arq = fopen(url,"wt");
    else
        arq = fopen(url,"a");
    if(arq == NULL){
        printf("Erro, nao foi possivel criar o arquivo\n");
        exit(1);
    }
    fprintf(arq,"Tempo:%f\n",temp);
    fprintf(arq,"\n-------------------\n");
    fprintf(arq,"Valores:\n");
    fprintf(arq,"-------------------\n");
    for(i=0;i < tam;i++)
        fprintf(arq,"%d\n",C[i]);
    fprintf(arq,"-------------------\n\n");
    fclose(arq);
}

int main(int argc, char *argv[]){
    int tam = atoi(argv[1]);
    char* url = argv[2];
    int numThread = atoi(argv[3]);
	if(tam > LIM || tam <= 2 || argc != 4){
        printf("Erro, parametros invalidos\n");
        exit(1);
	}
	int *C;
	C = (int*) malloc(sizeof(int)* tam);
	if(C == NULL){
        printf("Erro, nao foi possivel criar vetor");
        exit(1);
	}
	lerValores(url,C,tam);
	clock_t tempo;
	tempo = clock();
    DualheapSort(C,tam,numThread);
    float temp = (clock() - tempo) / (double)CLOCKS_PER_SEC;
    imprimir(C,tam,temp,numThread);
    free(C);
	exit(1);
}
