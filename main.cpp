#include <iostream>
#include "CSR.cpp"
#include <deque>
#include <list>
#include <pthread.h>
#include <semaphore.h>
#include "concurrent_queue.hpp"

#define NUM_THREADS  8


using namespace std;
using namespace tq;

int th_complete,threadend=1;
int th_count =0;
pthread_mutex_t mux, update;
pthread_mutex_t thd;
pthread_cond_t cond;
list<int> *th;
ThreadQueue<int> q;
int *visited;

void wakeSignal(){
    pthread_mutex_lock(&mux);
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mux);
}

//<========== THREAD POOL ===========>
void *t_pool(void *i) {
    int id = *((int *) i);
    int end;
    if(id>=csr->v_count) {
        cout << "\nThread id Error!! id >= total node count";
        exit(0);
    }

    while (threadend) {
        pthread_mutex_lock(&mux);
        while (true) {
            pthread_cond_wait(&cond, &mux);
            break;
        }
        pthread_mutex_unlock(&mux);

        while (!th[id].empty()) {
            int ver = th[id].front();
            th[id].pop_front();
            if(ver == (csr->v_count-1)){
                end = csr->e_count;
            }
            else{
                end=csr->vptr[ver+1];
            }
            for (int j = csr->vptr[ver]; j < end; ++j) {
                if (visited[csr->eptr[j]]==0) {
                    visited[csr->eptr[j]] = visited[ver]+1;
                    //parent[j]=ver;
                    q.push_back(csr->eptr[j]);
                }
            }
        }

        pthread_mutex_lock(&update);
        th_complete++;
        pthread_mutex_unlock(&update);
    }
    pthread_exit(nullptr);
}

// ============ BFS CODE ============= //
void BFS(int s){
    cout << "<=== BFS ===>\n";
    int x=0;
    q.clear();
    for (int i=0;i<csr->v_count;i++){
        visited[i] =  0;
    }
    cout << "\nMarked all as false" << endl;

    q.push_back(s);
    visited[s] = 1;
    cout << "starting vertex pushed into queue"<<endl;
    while(1){
        th_complete=0;
        while (!q.empty()) {
            int currvertex=q.front();
            q.pop_front();
            cout << "Visited : " << currvertex << " \n";
            th[th_count%(NUM_THREADS-1)].push_back(currvertex);
            th_count++;
        }
        th_count = th_count%(NUM_THREADS-1);
        wakeSignal();
        while(th_complete!=(NUM_THREADS)-1);
        if(q.empty()){
            break;
        }
    }
}

int main () {
    csr = (Graph *) malloc(sizeof(Graph));
    if (csr == nullptr) {
        printf("\nDynamic memory allocation failed.\n");
        exit(0);
    }
    readCSR();
    int n = csr->v_count;
    visited = (int *) calloc(n, sizeof(int));
    //parent = (int*)calloc(n, sizeof(int));

    pthread_t p[NUM_THREADS];
    th = new list<int>[NUM_THREADS];
    pthread_mutex_init(&mux, nullptr);
    pthread_mutex_init(&update, nullptr);
    pthread_mutex_init(&thd, nullptr);
    pthread_cond_init(&cond, nullptr);
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    int arg[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        arg[i]=i;
        pthread_create(&p[i], &attr, t_pool, (void *)(arg+i));
    }

    for (int k = 0; k < csr->v_count; k++) {
        cout << "[" << k << "] -- " << csr->vptr[k] << " : ";

        if (csr->vptr[k + 1] < 2 * csr->e_count) {
            for (int l = csr->vptr[k]; l < csr->vptr[k + 1]; l++) {
                cout << " -> " << csr->eptr[l];
            }
        } else {
            int l = csr->vptr[k];
            while(l<2*csr->e_count) {
                cout << " -> " << csr->eptr[l];
                l++;
            }
        }
        cout << endl;
    }
    BFS(2);
    threadend =0;
    wakeSignal();

    for(auto & i : p){
        pthread_join(i,nullptr);
    }

    free(visited);
    //free(parent);
    return 0;
}
