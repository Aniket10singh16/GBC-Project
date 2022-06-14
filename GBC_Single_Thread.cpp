#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <queue>
#include <vector>
#include <list>
#include "CSR.cpp"
using namespace std;

float gbc;
int *group;
int group_size;
queue<int> q;
vector<int> S;
vector<float> back;
list<int> *parent;

int main()
{
    csr = (Graph *) malloc(sizeof(Graph));
    if (csr == nullptr) {
        printf("\nDynamic memory allocation failed.\n");
        exit(0);
    }
    readCSR();
    int *nos = (int *)calloc(csr->v_count, sizeof(int));
    int *level = (int *)calloc(csr->v_count, sizeof(int));
    //cout << "No of vertices: " << csr->v_count << endl;
    //cout << "No of Edges: " << csr->e_count << endl;

    cout << "Enter the length of the group: ";
    cin >> group_size;
    group = (int *)calloc(group_size, sizeof(int));
    gbc = 0;

    cout << "Enter vertices for the group: " << endl;
    int temp;
    for (int i = 0; i < group_size; i++)
    {
        cin >> temp;
        group[temp] = 1;
    }

    for (int s = 0; s < csr->v_count; s++) // adding task to taskQueue
    {
        if (s > csr->v_count || s < 0)
        {
            cout << "-----Starting node out of bounds-----" << endl;
            cout << "EXITING..." << endl;
            exit(0);
        }

        int w, top;
        int n = csr->v_count;

        parent = new list<int>[n];
         // to calc no of total shortest path from root

        back.assign(n,0);

        int start, end, j;

        for (int i = 0; i < n; i++)
        {
            level[i] = -1;
            nos[i]=0;
        }

        level[s] = 0;
        nos[s] = 1;
        q.push(s);

        while (!q.empty())
        {
            w = q.front();
            q.pop();
            S.push_back(w);
            start = csr->vptr[w];
            if (w == (n - 1))
            {
                end = 2*csr->e_count;
            }
            else
            {
                end = csr->vptr[w + 1];
            }

            for (int i = start; i < end; i++)
            {
                j = csr->eptr[i];
                if (level[j] == -1)
                {
                    level[j] = level[w] + 1;
                    q.push(j);
                }
                if ((level[j] - 1) == (level[w]))
                {
                    nos[j] = nos[j] + nos[w];
                    parent[j].push_back(w);
                }
            }
        }

        while (!S.empty())
        {
            top = S.back();
            S.pop_back();

            for (auto v : parent[top])
            {
                float I = back[top];
                if (group[top] == 1)
                {
                    I = 0;
                }
                back[v] += (((float)nos[v] / (float)nos[top]) * (1 + I));
            }
            if (group[top] == 1 && top != s)
            {
                gbc += back[top];
            }
        }

        // for (int v = 0; v < n; v++)
        // {
        //     if (v != s)
        //     {
        //         bwc[v] = bwc[v] + back[v];
        //     }
        // }
        S.clear();
        back.clear();
    }

    printf("\n-------Group Betweeness centrality-------\n");

    gbc = gbc / 2;
    printf("GBC: %f\n", gbc);
    free(group);
    free(level);
    free(nos);
    back.clear();
    parent->clear();
    return 0;
}