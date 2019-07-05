//Algoritmo para solucação do problema clássico de concorrência. Leitores e Escritores
//Autor: Giovani Bertuzzo
//Teoria baseada em: http://www.ic.unicamp.br/~islene/mc514/l-e/l-e.pdf

#include <iostream>
#include <pthread.h>
#include <stdlib.h>
#include <list>
#include <mutex>
#include <thread>
#include <random>
#include <iterator>
#include <unistd.h>
#include <ctime>
#include <condition_variable>

std::list<int> some_list;   //dados
std::mutex data_mutex;      //mutex sobre os dados
std::mutex nlocks;          //mutex sobre o contador de leitores lendo
std::condition_variable cv; //variavel que emite/recebe signal para escrita quando numero de leitores = 0
int SLEEPERSEC=15;          //tempo de execucao do programa em segundos
int nlock=0;                //contador de numero de leitores lendo

void write_list(){
    srand((unsigned)time(0));
    int maior = 100;
    int menor = 0;
    int aleatorio = rand()%(maior-menor+1) + menor;
    some_list.push_back(aleatorio);
    std::cout << "Write value: " << aleatorio << std::endl;
}

void read_list(){
    std::list<int>::iterator it = some_list.begin();
    std::cout<<std::endl;
    std::cout<< "Readers On = " << nlock << " Read Values: " <<std::endl;
    while(it != some_list.end())
    {
        std::cout<<(*it)<<" ";
        it++;
    }
    std::cout<<std::endl;
}
void *writer(void *)
{
    while(true){
        std::unique_lock<std::mutex> rlk(data_mutex);   //tenta obter lock dos dados
        std::unique_lock<std::mutex> nlk(nlocks);       //tenta obter lock sobre o contador de leitores
        while(nlock>0)                                  //se o contador de leitores for maior que 0, aguarda
            cv.wait(nlk);                               //se for igual a 0 libera o lock sobre o contador de leitores e escreve
        nlk.unlock();                                   //depois libera o lock sobre os dados
        write_list();
        rlk.unlock();
        std::this_thread::sleep_for (std::chrono::milliseconds(20));
    }
    return NULL;
}
void *reader(void *)
{
    while(true){
        std::unique_lock<std::mutex> rlk(data_mutex);       //tenta obter lock dos dados
        std::unique_lock<std::mutex> nlk(nlocks);           //tenta obter lock do contador de leitores
        nlock++;                                            //incrementa contador de leitores
        nlk.unlock();                                       //libera lock do contador de leitores
        rlk.unlock();                                       //libera lock  dos dados
        read_list();                                        //le dados
        nlk.lock();                                         //tenta obter lock do contador de leitores
        nlock--;                                            //decrementa contador de leitores
        if(nlock==0)                                        //verifica se o numero de leitores é igual a 0
            cv.notify_all();                                //se for envia signal para os escritores
        nlk.unlock();                                       //libera lock do contador de leitores
        std::this_thread::sleep_for (std::chrono::milliseconds(200));
    }
    return NULL;
}
void *sleeper(void *)
{
    int num = 0;
    while(num<SLEEPERSEC){
        std::this_thread::sleep_for (std::chrono::seconds(1));
        num++;
    }
    return NULL;
}

int main()
{
    int READERS=10;
    int WRITERS=3;

    for(int i=0;i<READERS;i++){
        pthread_t reader1;
        pthread_create(&reader1, NULL, reader, NULL);
    }
    for(int i=0;i<WRITERS;i++){
        pthread_t writer1;
        pthread_create(&writer1, NULL, writer, NULL);
    }

    pthread_t sleeper1;
    pthread_create(&sleeper1, NULL, sleeper, NULL);
    pthread_join(sleeper1, NULL);

    return 0;
}
