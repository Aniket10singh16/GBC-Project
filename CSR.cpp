#include <iostream>
#include <fstream>


using namespace std;


typedef struct
{
    int v_count;
    int e_count;
    int* vptr;
    int* eptr;
}Graph;
Graph* csr;
int left_v,right_v,prev_v;


//<========== CSR ============>
void readCSR(){
    ifstream file(R"(D:\WORK\Sem VI\Final_year_Project\dolphins1.txt)");
    int i=0,j=0;
    if(!file.is_open()){
        cerr << "failed to open";
    }
    else {
        cout << "opened successfully" << endl;
        file >> csr->v_count;
        file >> csr->e_count;
        cout << csr->v_count<<"\n"<<csr->e_count<<"\n";
        csr->vptr = (int*) malloc(csr->v_count * sizeof(int));
        if(csr->vptr == nullptr){
            cout << "\nDynamic Memory Allocation Failed!\n";
            exit(0);
        }
        csr->eptr = (int*)malloc(2*csr->e_count * sizeof(int));
        if(csr->eptr == nullptr){
            cout << "\nDynamic Memory Allocation Failed!\n";
            exit(0);
        }
        file >> left_v;
        file.ignore();
        file >> right_v;
        while(!file.eof()){
            prev_v = left_v;
            csr->vptr[i] = j;
            while(prev_v == left_v && !file.eof()){
                csr->eptr[j] = right_v;
                file >> left_v;
                file.ignore();
                file >> right_v;
                j++;
                if(file.eof()){
                    if(prev_v != left_v){
                        csr->vptr[++i] = j;
                    }
                    csr->eptr[j] = right_v;
                }
            }
            i++;
        }


    }
    file.close();
}



