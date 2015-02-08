#include "thread_random.h"
#include "stdio.h"
#include "time.h"

thread_random::thread_random(QObject *parent) :
    QThread(parent),c(362436)
{
    is_running = true;
    using_randomness_file = false;
    //init_rand(time(NULL));
    init_rand(0);
    this->start();
    QThread::sleep(1); // for randomness
}

thread_random::~thread_random()
{
    is_running = false;

    if(randomness_file.isOpen()) {
        randomness_file.close();
    }

    wait(1000);
    this->quit();
}

void thread_random::randomness_from_file(bool set, QString file)
{
    if(set)
    {
        //close old opened file
        if(randomness_file.isOpen()) {
            randomness_file.close();
        }

        randomness_file.setFileName(file);
        if(!randomness_file.open(QIODevice::ReadOnly))
        {
            using_randomness_file = false;
        } else {
            using_randomness_file = true;
        }
    } else {
        using_randomness_file = false;
        if(randomness_file.isOpen()) {
            randomness_file.close();
        }
    }
}

void thread_random::stop_processing()
{
    is_running = false;
}

void thread_random::terminate()
{
    stop_processing();
}

void thread_random::run()
{
    unsigned int val;
    while(is_running)
    {
       val = rand_cmwc();
       if(req_stat[0])
       {
           random_num = val;
           req_stat[1] = true;
           req_stat[0] = false;
       }
    }
}

bool thread_random::_gen_random_num(unsigned int *value)
{
    if(using_randomness_file &&
            !randomness_file.atEnd())
    {
        randomness_file.read((char*)value, sizeof(unsigned int));
        return true;
    } else {
        if(using_randomness_file) using_randomness_file = false;
        if(req_stat[0] == true)
        {
            return false;
        }
        if(req_stat[1] == false)
        {
            req_stat[0] = true;
            return false;
        }
        *value = random_num;
        req_stat[1] = false;
        return true;
    }
}

unsigned int thread_random::gen_random_num()
{
   unsigned int ret;
   while(!_gen_random_num(&ret));
   return ret;
}

int thread_random::gen_random_array(uchar *array, int len)
{
    int i;

    for(i=0; i< len; i++)
    {
        *(array+i) = (gen_random_num()%256);
    }
    return 0;
}
