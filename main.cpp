#include <iostream>
#include "CSR.cpp"
#include <deque>
#include <list>
#include <stack>
#include <pthread.h>
#include <semaphore.h>
#include <vector>
#include "concurrent_queue.hpp"

#define NUM_THREADS 8

using namespace std;
using namespace tq;

int th_complete,threaddie=1,FrontPhase=1,BackPhase=0,BCAccumulate=0;
int Start,StackV, th_count =0;
pthread_mutex_t mux, update, innerUp, BCUpdate;
pthread_cond_t cond;
list<int> *th;
ThreadQueue<int> q;
stack<int> S;
int *visited;
int *sigma;
vector<float> delta;
float *BC;
list<int> *parent;
struct timespec start, finish;
double elapsed;

void wakeSignal(){
    pthread_mutex_lock(&mux);
           pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mux);
}

void ThreadUpdate(){
    pthread_mutex_lock(&update);
    th_complete++;
    pthread_mutex_unlock(&update);
}

//<========== THREAD POOL ===========>
void *t_pool(void *i) {
    int id = *((int *) i);
    int end;
    while (threaddie) {
        pthread_mutex_lock(&mux);
        while (true) {
            pthread_cond_wait(&cond, &mux);
            break;
        }
        pthread_mutex_unlock(&mux);
        if(FrontPhase==1){
        while (!th[id].empty()) {
                int ver = th[id].front();
                th[id].pop_front();

                if (ver == (csr->v_count - 1)) {
                    end = 2 * csr->e_count;
                } else {
                    end = csr->vptr[ver + 1];
                }
                for (int j = csr->vptr[ver]; j < end; ++j) {
                    int w = csr->eptr[j];
                    if (visited[w] == -1) {
                        visited[w] = visited[ver] + 1;
                        q.push_back(w);
                    }
                    if ((visited[w] - 1) == visited[ver]) {
                        sigma[w] += sigma[ver];
                        parent[w].emplace_back(ver);
                    }
                }
                delta[ver]=1/(float)sigma[ver];
            }
        }

        if(BackPhase==1){
            while (!th[id].empty()) {
               int w = th[id].front();
               th[id].pop_front();
               delta[w] += delta[StackV];
            }
        }

        if(BCAccumulate==1){
            while (!th[id].empty()) {
                int v = th[id].front();
                th[id].pop_front();
                if(v!=Start){
                    pthread_mutex_lock(&BCUpdate);
                    BC[v] += (delta[v] * (float) sigma[v] - 1);
                    pthread_mutex_unlock(&BCUpdate);
                }
            }
        }
        ThreadUpdate();
    }
    pthread_exit(nullptr);
}

// ======== BetweennessCentrality Accumulation ========= //
void BwcAccumulate(int n){

    while(true) {
        th_count=0;
        th_complete=0;
        int v=0;
        while(v<n){
            th[th_count % (NUM_THREADS)].push_back(v);
            th_count++;
            v++;
        }
        th_count = th_count % (NUM_THREADS);
        wakeSignal();
        while (th_complete != (NUM_THREADS));
        if (v >= n) {
            break;
        }
    }
}

// ========= Forward Phase =========== //
void Forward(int cv){
    cout << "\nstarting vertex : "<<Start<<" pushed into queue"<<"\n==================================================="<<endl;
    while(true){
        th_complete=0;
        while (!q.empty()) {
            int currvertex=q.front();
            q.pop_front();
            S.push(currvertex);
            th[th_count%(NUM_THREADS)].push_back(currvertex);
            cout << "Visited[" << cv << "] : -> "<< currvertex << " \n";
            th_count++;
            cv++;
        }
        th_count = th_count%(NUM_THREADS);
        wakeSignal();
        while(th_complete!=(NUM_THREADS));
        if(q.empty() ){
            break;
        }
    }
}

// ========= Backward Phase =========== //
void BackPropagation(int SV){
    while(true) {
        int itr =0;
        th_complete=0;
        th_count=0;
        for (auto j: parent[SV]) {
            th[th_count % (NUM_THREADS)].push_back(j);
            th_count++;
            itr++;
        }
        th_count = th_count % (NUM_THREADS);
        wakeSignal();
        while (th_complete != (NUM_THREADS));
        if (itr != parent[StackV].size()-1 || parent[StackV].empty()) {
            break;
        }
    }
}



// ============ Print No. Of Shortest Path ============= //
void ShortPath(int s){
    printf("-----------------  No. of shortest paths from node %d ------------------\n", s);
    for (int a = 0; a < csr->v_count; a++)
    {
        printf("Node %d is %d \n", a, sigma[a]);
    }
}

// ============ BetweennessCentrality CODE ============= //
void BetweennessCentrality(int s){
    q.clear();
    Start = s;
    int cv =0;
    int n = csr->v_count;
    delta.assign(n,0);
    parent = new list<int>[n];

    for (int i=0;i<n;i++){
        visited[i]=-1;
        sigma[i]=0;
    }

    visited[s] = 0;
    sigma[s]=1;
    q.push_back(s);
    Forward(cv);
    FrontPhase=0;
    BackPhase=1;
    th_count=0;
    while(!S.empty()){
        StackV = S.top();
        S.pop();
        BackPropagation(StackV);
    }
    BackPhase=0;
    BCAccumulate=1;
    BwcAccumulate(n);
    BCAccumulate=0;
    //ShortPath(s);
    delta.clear();
    FrontPhase=1;
}
// ============== PRINT CSR =============== //
void PrintCSR(){
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
}

// ============== PRINT BWC =============== //
void PrintBWC(){
    cout<< " \n\nprint Betweenness Centrality==>";
    for(int i=0; i<csr->v_count;i++){
        cout << " \nvertex: " << i<<" ==> " << BC[i]/2;
    }
}

// ============== INITIALIZE ============== //
void Initialize(){
    int n = csr->v_count;
    visited = (int *) calloc(n,sizeof (int));
    sigma = (int *) calloc(n, sizeof (int));
    //delta = (float *) calloc(n, sizeof(float));
    BC = (float *) calloc(n, sizeof(float));
    for(int i=0;i<n;i++){ BC[i]=0;}
    th = new list<int>[NUM_THREADS];
    //th1 = new list<int>[NUM_THREADS];
    pthread_mutex_init(&mux, nullptr);
    pthread_mutex_init(&update, nullptr);
    pthread_mutex_init(&innerUp, nullptr);
    pthread_mutex_init(&BCUpdate, nullptr);
    pthread_cond_init(&cond, nullptr);
}

int main () {
    csr = (Graph *) malloc(sizeof(Graph));
    if (csr == nullptr) {
        printf("\nDynamic memory allocation failed.\n");
        exit(0);
    }
    readCSR();
    Initialize();
    pthread_t p[NUM_THREADS];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    int arg[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        arg[i]=i;
        pthread_create(&p[i], &attr, t_pool, (void *)(arg+i));
    }
    //PrintCSR();
    clock_gettime(CLOCK_REALTIME, &start);

    for(int b=0; b<csr->v_count;b++){
        BetweennessCentrality(b);
    }

    threaddie =0;
    wakeSignal();

    for(auto & i : p){
        pthread_join(i,nullptr);
    }
    for(auto & i : p){
        pthread_kill(i,NUM_THREADS);
    }

    clock_gettime(CLOCK_REALTIME, &finish);
    elapsed = ((double)finish.tv_sec - (double)start.tv_sec);
    elapsed += ((double)finish.tv_nsec - (double)start.tv_nsec) / 1000000000.0;
    printf("\nPthread implementation time: %f\n", elapsed);

    PrintBWC();

    free(visited);
    delta.clear();
    free(sigma);
    free(BC);
    parent->clear();
    th->clear();
    pthread_mutex_destroy(&mux);
    pthread_mutex_destroy(&update);
    pthread_mutex_destroy(&innerUp);
    pthread_mutex_destroy(&BCUpdate);
    pthread_cond_destroy(&cond);
    return 0;
}

