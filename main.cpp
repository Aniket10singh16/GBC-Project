
// This is an experimental version and contains problems regarding multithreading 

#include <iostream>
#include "CSR.cpp"
#include <deque>
#include <list>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <vector>
#include "concurrent_queue.hpp"

#define NUM_THREADS 8

using namespace std;
using namespace tq;

int th_complete,threaddie=1,FrontPhase=1,BackPhase=0,BCAccumulate=0;
int Start,StackV, th_count =0;
pthread_mutex_t mux, update, innerUp, BCUpdate;
pthread_cond_t cond, upadateCond;
list<int> *th;
ThreadQueue<int> q;
vector<int> S;
int *visited;
int *sigma;
vector<float> delta;
float *BC;
list<int> *parent;
struct timespec start, finish;
double elapsed;
float gbc;
int *group;
int group_size;

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
            }
            pthread_mutex_lock(&update);
            th_complete++;
            pthread_cond_signal(&upadateCond);
            pthread_mutex_unlock(&update);
        }

        if(BackPhase==1){
            while (!th[id].empty()) {
               int w = th[id].front();
               th[id].pop_front();
                pthread_mutex_lock(&innerUp);
                    float I;
                    if(group[StackV]==1){I=0;}
                    else{I = delta[StackV];}
                    delta[w] += (((float)sigma[w]/(float)sigma[StackV])*(1+I));
                pthread_mutex_unlock(&innerUp);

            }
            pthread_mutex_lock(&update);
            th_complete++;
            pthread_cond_signal(&upadateCond);
            pthread_mutex_unlock(&update);
        }
    }
    pthread_exit(nullptr);
}

// ========= Forward Phase =========== //
void Forward(int cv){
    //cout << "\nstarting vertex : "<<Start<<" pushed into queue"<<"\n==================================================="<<endl;
    while(true){
        th_complete=0;
        while (!q.empty()) {
            int currvertex=q.front();
            //cout<< "\nCurrent  vertex: "<< currvertex;
            q.pop_front();
            S.push_back(currvertex);
            th[th_count%(NUM_THREADS)].push_back(currvertex);
            th_count++;
            cv++;
        }
        th_count = th_count%(NUM_THREADS);
        wakeSignal();
        pthread_mutex_lock(&update);
        while (th_complete != (NUM_THREADS)){
            pthread_cond_wait(&upadateCond,&update);
        }
        pthread_mutex_unlock(&update);
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
        auto j = parent[SV].begin();
        //cout<< "\nStack vertex: "<< SV;
        while(j!=parent[SV].end()){
            th[th_count % (NUM_THREADS)].push_back(*j);
            //cout<< "\nPushed in thread Queue: "<< *j;
            th_count++;
            itr++;
            j++;
        }
        th_count = th_count % (NUM_THREADS);
        wakeSignal();
        pthread_mutex_lock(&update);
        while (th_complete != (NUM_THREADS)){
            pthread_cond_wait(&upadateCond,&update);
        }
        pthread_mutex_unlock(&update);
        if (itr != parent[StackV].size()-1 ||S.empty() || parent[StackV].empty()) {
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
    cout<< "\nStart vertex ["<<s<<"]: ";
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
    th_count=0;
    BackPhase=1;
    while (!S.empty()) {
        th_count=0;
        StackV = S.back();
        S.pop_back();
        BackPropagation(StackV);
        if(group[StackV]==1 & StackV != Start){
            gbc += delta[StackV];
        }
    }
    BackPhase=0;
    delta.clear();
    S.clear();
    FrontPhase=1;
}

// ============== INITIALIZE ============== //
void Initialize(){
    int n = csr->v_count;
    visited = (int *) calloc(n,sizeof (int));
    sigma = (int *) calloc(n, sizeof (int));
    group = (int *)calloc(csr->v_count, sizeof(int));
    cout << "Enter the length of the group: ";
    cin >> group_size;
    gbc = 0;
    int temp;
    cout << "Enter vertices for the group: " << endl;
    for (int i = 0; i < group_size; i++)
    {
        cin >> temp;
        group[temp] = 1;
    }
    cout<<"\nCalculating GBC";
    th = new list<int>[NUM_THREADS];

    pthread_mutex_init(&mux, nullptr);
    pthread_mutex_init(&update, nullptr);
    pthread_mutex_init(&innerUp, nullptr);
    pthread_mutex_init(&BCUpdate, nullptr);
    pthread_cond_init(&cond, nullptr);
    pthread_cond_init(&upadateCond, nullptr);
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
        pthread_cancel(i);
    }

    clock_gettime(CLOCK_REALTIME, &finish);
    elapsed = ((double)finish.tv_sec - (double)start.tv_sec);
    elapsed += ((double)finish.tv_nsec - (double)start.tv_nsec) / 1000000000.0;
    printf("\nPthread implementation time: %f\n", elapsed);

       cout<< "\n-------Group Betweenness centrality-------\n";

    gbc = gbc / 2;
    printf("GBC: %f \n", gbc);
    gbc = gbc/(float)((csr->v_count-group_size)*(csr->v_count-group_size-1));
    printf("GBC: %f \n", gbc);
    for(int i=0;i<5;i++){
        cout<< "\nwait("<<i<<")";
        sleep(i);
    }
    free(visited);
    delta.clear();
    free(sigma);
    free(group);
    parent->clear();
    th->clear();
    pthread_mutex_destroy(&mux);
    pthread_mutex_destroy(&update);
    pthread_mutex_destroy(&innerUp);
    pthread_mutex_destroy(&BCUpdate);
    pthread_cond_destroy(&cond);
    pthread_cond_destroy(&upadateCond);
    return 0;
}

