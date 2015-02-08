#ifndef PASAFER_H
#define PASAFER_H

#include <QCryptographicHash>
#include <QString>
#include <QByteArray>


#define MAX_STATES_NUM  256
#define MAX_STATE_LEN   100

class Pasafer
{
public:
    Pasafer(int hash_type);
    int     gen_sands_file(qint64 file_size);
    QString get_password(int len);

    void set_sands_file(QString file);
    void set_key(QString key);
    void set_main_password(QString passowrd);
    void set_hash_func(int hash_type);

private:
    QCryptographicHash *hash_func;
    int hash_len;

    QString sands_file_name;
    QString main_password;
    QString key;

    uchar main_password_char[MAX_STATE_LEN];
    uchar key_char[MAX_STATE_LEN];

    uchar r_key[MAX_STATE_LEN];
    uchar states[MAX_STATES_NUM][MAX_STATE_LEN];

    void buff_hash(const uchar *buff, int len, uchar *result);
    void swape_trans(uchar target[], const uchar trans[],int inverse);
    void self_swape_trans(uchar *target, const uchar *trans, int inverse);
    void XOR_trans(uchar target[], const uchar trans[], int inverse);
    void self_circular_shift(uchar *target, const uchar *trans, int inverse);
    void circular_shift(uchar target[], const uchar trans[], int inverse);

    void transform(uchar target[], const uchar trans[],int depth, int inverse);
    void __transform(uchar target[], const uchar trans[], int depth);

};

#endif // PASAFER_H
