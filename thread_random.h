#ifndef THREAD_RANDOM_H
#define THREAD_RANDOM_H

#include <QThread>
#include <QObject>
#include <QFile>


#define PHI 0x9e3779b9

class thread_random : public QThread
{
    Q_OBJECT

public:
    explicit thread_random(QObject *parent = 0);
    ~thread_random();

    void run();

    unsigned int gen_random_num();
    bool _gen_random_num(unsigned int *value);
    int gen_random_array(uchar *array, int len);

    void stop_processing();

signals:

public slots:
    void terminate();
    void randomness_from_file(bool set, QString file);

private:

    volatile bool is_running;
    volatile bool req_stat[2];
    volatile unsigned int random_num;
    QFile randomness_file;
    bool using_randomness_file;

    unsigned long int  Q[4096], c,c1;
    unsigned long int rand_cmwc(void)
    {
        unsigned long long int  t, a = 18782LL;
        static unsigned long int  i = 4095;
        unsigned long int  x,x1, r = 0xfffffffe;
        i = (i + 1) & 4095;
        t = a * Q[i] + c;
        c = (t >> 32);
        x = t + c;
        x1=x;
        if (x < c) {
            x++;
            c++;
        }
        else
        {
            x1++;
            c1++;
        }
        c1=x1;
        return (Q[i] = r - x);
    }
    void init_rand(unsigned long int  x)
    {
        int i;

        Q[0] = x;
        Q[1] = x + PHI;
        Q[2] = x + PHI + PHI;

        for (i = 3; i < 4096; i++)
            Q[i] = Q[i - 3] ^ Q[i - 2] ^ PHI ^ i;
    }
};

#endif // THREAD_RANDOM_H
