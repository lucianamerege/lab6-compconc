#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int NTHREADS;
int contador;
pthread_mutex_t mutex;
pthread_cond_t cond;

int *vetor;

void *tarefa(void *arg) {

    long int id = (long int) arg; //identificador da thread passado na criação dela
    long int *somatorio;
    
    somatorio = (long int*) malloc(sizeof(long int));
    if(somatorio==NULL) {exit(1);}
    *somatorio = 0;

    printf("\nInício da thread %ld\n", id);

    pthread_mutex_lock(&mutex);

    for (int i=0; i<NTHREADS; i++){
        
        for(int j=0; j<NTHREADS; j++){
            *somatorio += vetor[j];
        }
        contador--;
        if(contador > 0){
            printf("\nThread %ld esperando pelo sinal\n", id);
            pthread_cond_wait(&cond, &mutex);
        } else{
            contador = NTHREADS;
            pthread_cond_broadcast(&cond);
        }
        printf("\nThread %ld substituindo o valor no vetor\n", id);
        vetor[id]=rand()%10;

        contador--;
        if(contador > 0){
            pthread_cond_wait(&cond, &mutex);
        } else{
            contador = NTHREADS;
            pthread_cond_broadcast(&cond);
        }
        printf("\nThread %ld indo para a próxima iteração\n", id);
    }

    contador--;
    if(contador > 0){
        pthread_cond_wait(&cond, &mutex);
    } else{
        contador = NTHREADS;
        pthread_cond_broadcast(&cond);
    }

    printf("\nThread %ld retornando para a main\n", id);  
    pthread_mutex_unlock(&mutex);
    //retorna o resultado da soma local
    pthread_exit((void *) somatorio); 
}

int main(int argc, char* argv[]){

    if(argc<2) {
        printf("Digite: %s <numero de threads>\n", argv[0]);
        return 1;
    }

    NTHREADS = atoi(argv[1]);
    contador = NTHREADS;

    /* Inicilaiza o mutex (lock de exclusao mutua) e a variavel de condicao */
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init (&cond, NULL);  

    long int *resultados, *retorno;
    pthread_t *tid; //identificadores das threads no sistema


    tid = (pthread_t*) malloc(sizeof(pthread_t)*NTHREADS);
    if(tid==NULL) {puts("ERRO--malloc"); return 2;}

    vetor = (int *) malloc(sizeof(int) * NTHREADS);
    if (vetor == NULL) {printf("ERRO--malloc\n"); return 2;} //isso é para caso ele não consiga alocar espaço na memória para esse vetor.

    resultados = (long int *) malloc(sizeof(long int) * NTHREADS);
    if (resultados == NULL) {printf("ERRO--malloc\n"); return 2;} //isso é para caso ele não consiga alocar espaço na memória para esse vetor.

    for (int i=0; i<NTHREADS; i++){
        vetor[i] = (rand() % 10); //populando o vetor inicial com valores randomicos entre 0  e 9
    }

    //criacao das threads
    for(long int i=0; i<NTHREADS; i++) {
        if(pthread_create((tid+i), NULL, tarefa, (void*) i)){
            puts("ERRO--pthread_create"); return 3;
        }
    }
 
    //aguardar o termino das threads
    for( int i=0; i<NTHREADS; i++) {
        if(pthread_join(*(tid+i), (void**) &retorno)){
            fprintf(stderr, "ERRO--pthread_create\n");
            return 3;
        }

        resultados[i] = *retorno;
    }

    int desigual=0;

    for (int i=0; i<NTHREADS; i++){
        printf("%ld ", resultados[i]);
        if(i>0 && resultados[i]!=resultados[i-1]){
            printf("\nValor diferente encontrado.\n");
            desigual++;
            break;
        }
    }
    
    if(desigual==0){
        printf("\nTodos os valores corretos.\n");
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    free(vetor);
    free(resultados);
    free(tid);
}