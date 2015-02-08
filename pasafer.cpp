#include <QDebug>
#include "pasafer.h"
#include "thread_random.h"

Pasafer::Pasafer(int hash_type=QCryptographicHash::Sha256)
{
    hash_func = NULL;
    set_hash_func(hash_type);
    sands_file_name="pasafer.sands";
}

void Pasafer::set_sands_file(QString file)
{
    sands_file_name = file;
}

void Pasafer::set_main_password(QString password)
{
    QByteArray qb;

    main_password = password;
    qb = password.toLatin1();

    hash_func->reset();
    hash_func->addData(qb.data(), qb.length());
#define SALT "*D.j@|}jd9D4+_(&>G"
    hash_func->addData(SALT, sizeof(SALT));
    hash_func->addData(qb.data(), qb.length());
    qb = hash_func->result();
    memcpy(main_password_char, qb.data(), qb.length());
}

void Pasafer::set_key(QString key)
{
    QByteArray qb;

    this->key = key;
    qb = key.toLatin1();

    hash_func->reset();
    hash_func->addData(qb.data(), qb.length());
#define SALT2 "LN#d>u!(9l:T31pQ"
    hash_func->addData(SALT2, sizeof(SALT2));
    hash_func->addData(qb.data(), qb.length());
    qb = hash_func->result();
    memcpy(key_char, qb.data(), qb.length());
}

void Pasafer::set_hash_func(int hash_type)
{
    switch(hash_type) {
        case QCryptographicHash::Md5:
            hash_len = 16;
            break;
        case QCryptographicHash::Sha1:
            hash_len = 20;
            break;
        case QCryptographicHash::Sha224:
            hash_len = 26;
            break;
        case QCryptographicHash::Sha256:
            hash_len = 32;
            break;
        case QCryptographicHash::Sha384:
            hash_len = 48;
            break;
        case QCryptographicHash::Sha512:
            hash_len = 64;
            break;
        case QCryptographicHash::Sha3_224:
            hash_len = 26;
            break;
        case QCryptographicHash::Sha3_256:
            hash_len = 32;
            break;
        case QCryptographicHash::Sha3_384:
            hash_len = 48;
            break;
        case QCryptographicHash::Sha3_512:
            hash_len = 64;
            break;
        default:
            hash_type = QCryptographicHash::Sha256;
            hash_len = 32;
            break;
    }
    if (hash_func) {
        delete hash_func;
    }
    hash_func = new QCryptographicHash((QCryptographicHash::Algorithm)hash_type);
}


int Pasafer::gen_sands_file(qint64 file_size)
{
    thread_random rand_gen;
    uchar buff[1024];
    QFile  f;

    file_size *= 1024;

    f.setFileName(sands_file_name);
    if(f.open(QIODevice::WriteOnly))
    {
        while(file_size)
        {
            rand_gen.gen_random_array(buff, 1024);
            f.write((const char*)buff, 1024);
            file_size--;
        }
        f.close();
        return 0;
    }
    else
    {
        qDebug() << "Cannot open file!";
        return -1;
    }
}

QString Pasafer::get_password(int len)
{
    QFile file;
    QByteArray qb;
    int per_byte = hash_len/len;
    uchar data_nums[MAX_STATE_LEN];
    uchar password[MAX_STATE_LEN];
    int state_index = MAX_STATES_NUM - 1;
    int i,j;

    memset(password, 0, sizeof(password));

    file.setFileName(sands_file_name);
    if(!file.open(QIODevice::ReadOnly)) {
        return QString("Sands file open error");
    }

    hash_func->reset();
    hash_func->addData((const char*)main_password_char, hash_len);
#define SALT3 "sf4d#1dhpj>,m"
    hash_func->addData((const char*)key_char, hash_len);
    hash_func->addData(SALT3, sizeof(SALT3));
    qb = hash_func->result();
    memcpy(r_key, qb.data(), qb.length());

    if (file.size() <= 10 * hash_len * MAX_STATES_NUM) {
        return QString("Sands file too small");
    }

    transform(this->r_key, this->main_password_char, 3,  0);
    for(i = 0; i < MAX_STATES_NUM; i++)
    {
        file.read((char *)states[i], hash_len);
        transform(this->r_key, states[i], 4, 0);
        transform(states[i], this->r_key, 4, 1);
        transform(states[i], this->main_password_char, 1, 1);
    }

    while(true)
    {
        transform(r_key,states[state_index],2, 1);
        transform(states[state_index],r_key,1,0);
        transform(states[state_index], this->main_password_char, 1, 1);

        i = file.read((char *)data_nums, hash_len);
        if(i != hash_len)
        {
            break;
        }
        transform(states[state_index], data_nums, 1,0);
        transform(data_nums, states[state_index], 1,0);
        transform(this->r_key, data_nums, 1,1);

        state_index += data_nums[0];
        state_index %= MAX_STATES_NUM;
    }
    file.close();

    for (i = 0; i < len;  i++) {
        for (j = 0; j < per_byte && i*per_byte+j < hash_len; j++) {
            password[i] ^= r_key[i*per_byte+j];
        }
    }
    QByteArray base64 = (QByteArray((const char *)password)).toBase64();
    base64.remove(len, base64.length()-len);

    return QString(base64);

}

void Pasafer::buff_hash(const uchar *buff, int len, uchar *result)
{
    QByteArray hvalue;

    hash_func->reset();
    hash_func->addData((const char *)this->main_password_char, hash_len);
    hash_func->addData((const char *)buff, len);
    hash_func->addData((const char *)this->key_char, hash_len);
    hvalue = hash_func->result();
    memcpy(result, hvalue.data(), hash_len);
    return;
}


void Pasafer::swape_trans(uchar target[], const uchar trans[],int inverse)
{
    int i, k;
    if(inverse)
    {
        for(i=hash_len -1; i>=0; i--)
        {
            k = (trans[i]^main_password_char[hash_len-1 -i])%hash_len;
            if(i == k) {
                target[i] ^= key_char[k];
            } else {
                //swap(a, b) (a) ^= (b), (b) ^= (a), (a) ^= (b)
                target[i] ^= target[k];
                target[k] ^=  target[i];
                target[i] ^= target[k];
            }
        }
    }
    else
    {
        for(i=0; i< hash_len; i++)
        {
            k = (trans[i]^main_password_char[hash_len-1 -i])%hash_len;
            if(i == k) {
                target[i] ^= key_char[k];
            } else {
                //swap(a, b) (a) ^= (b), (b) ^= (a), (a) ^= (b)
                target[i] ^= target[k];
                target[k] ^=  target[i];
                target[i] ^= target[k];
            }
        }
    }
}

void Pasafer::self_swape_trans(uchar *target, const uchar *trans, int inverse)
{
    int i, m, n;

    if(inverse)
    {
        for(i = hash_len*8-2; i >= 1; i--)
        {
            m = target[i/8] & (1<<(i%8));
            n = target[(i+1)/8] &(1<<((i+1)%8));

            if(trans[i/8] & (1<<(i%8)))
            {
                //swape two bit.
                if(n)
                {
                    target[i/8]  |= 1<<(i%8);
                }
                else
                {
                    target[i/8] &= (~(1<<(i%8)));
                }

                if(m)
                {
                    target[(i+1)/8] |= 1<<((i+1)%8);
                }
                else
                {
                    target[(i+1)/8] &= (~(1<<((i+1)%8)));
                }
            }
        }
    }
    else
    {
        for(i = 1; i <= hash_len*8-2; i++)
        {
            m = target[i/8] & (1<<(i%8));
            n = target[(i+1)/8] & (1<<((i+1)%8));

            if(trans[i/8] & (1<<(i%8)))
            {
                //swape two bit.
                if(n)
                {
                    target[i/8]  |= 1<<(i%8);
                }
                else
                {
                    target[i/8] &= (~(1<<(i%8)));
                }

                if(m)
                {
                    target[(i+1)/8] |= 1<<((i+1)%8);
                }
                else
                {
                    target[(i+1)/8] &= (~(1<<((i+1)%8)));
                }
            }
        }
    }
}

void Pasafer::XOR_trans(uchar target[], const uchar trans[], int inverse)
{
    int i,k;

    if(inverse)
    {
        for(i=hash_len-1; i>=0; i--)
        {
            k = (trans[i]^main_password_char[i])%hash_len;
            if(i == k) {
                target[i] = ~target[i];
            } else {
                target[i] ^=target[k];
            }
        }
    }
    else
    {
        for(i=0; i< hash_len; i++)
        {
            k = (trans[i]^main_password_char[i])%hash_len;
            if(i == k) {
                target[i] = ~target[i];
            } else {
                target[i] ^=target[k];
            }
        }
    }

}

void Pasafer::self_circular_shift(uchar *target, const uchar *trans, int inverse)
{
    int i,k;

    if(inverse)
    {
        for(i=0; i<hash_len; i++)
        {
            k = trans[i]%8;
            target[i] = target[i] >>k | target[i] <<(8-k);
        }
    }
    else
    {
        for(i=0; i<hash_len; i++)
        {
            k = trans[i]%8;
            target[i] = target[i] << k | target[i] >>(8-k);
        }
    }

}

void Pasafer::circular_shift(uchar target[], const uchar trans[], int inverse)
{
    int i, k;
    uchar t;

    if(inverse)
    {
        for(i=hash_len -1; i>=0; i--)
        {
            k = (trans[i]^key_char[hash_len-1 - i])%hash_len;
            if(i == k) {
                target[i] = target[i] << (i%8) | target[i] >>(8-(i%8));
            } else {
                t = target[i];
                target[i] = (target[i] >> (trans[i]%8)) | (target[k] << (8 - trans[i]%8)) ;
                target[k] = (target[k] >> (trans[i]%8)) | (t << (8 - trans[i]%8))  ;
            }
        }
    }
    else
    {
        for(i=0; i<hash_len; i++)
        {
            k = (trans[i]^key_char[hash_len-1 -i])%hash_len;
            if(i == k) {
                target[i] = target[i] >> (i%8) | target[i] <<(8-(i%8));
            } else {
                t = target[i];
                target[i] = (target[i] << (trans[i]%8)) | (target[k] >> (8 - trans[i]%8)) ;
                target[k] = (target[k] << (trans[i]%8)) | (t >> (8 - trans[i]%8))  ;
            }
        }
    }
}

void Pasafer::__transform(uchar target[], const uchar trans[], int depth)
{
    uchar tmd5[MAX_STATE_LEN];

    if(depth == 0)
    {
        buff_hash(trans, hash_len, tmd5);
        circular_shift ((uchar *)target, (uchar *)tmd5, 1);
        return;
    }
    else
    {
        buff_hash(trans, hash_len, tmd5);

        __transform(target, tmd5, --depth);

        swape_trans    (target, tmd5, 1);
        self_swape_trans(target, tmd5, 1);
        circular_shift (target, tmd5, 1);
        self_circular_shift(target, tmd5, 1);
        XOR_trans      (target, tmd5, 1);
        return;
    }
}

void Pasafer::transform(uchar target[], const uchar trans[],int depth, int inverse)
{
    int i;
    uchar tmd5[MAX_STATE_LEN];

    if(inverse)//inverse transform
    {
        buff_hash(trans, hash_len, tmd5);

        __transform(target,tmd5,depth);

        for(i = 0; i<hash_len; i++)
        {
            target[i] = tmd5[i]^target[i];
        }
        return;
    }
    else
    {
        buff_hash(trans, hash_len, tmd5);
        for(i = 0; i<hash_len; i++)
        {
            target[i] = tmd5[i]^target[i];
        }

        while(depth--)
        {
            buff_hash(tmd5, hash_len, tmd5);

            XOR_trans(target,tmd5, 0);
            self_circular_shift(target, tmd5, 0);
            circular_shift(target,tmd5, 0);
            self_swape_trans(target, tmd5, 0);
            swape_trans(target,tmd5, 0);
        }
        buff_hash(tmd5, hash_len, tmd5);
        circular_shift(target,tmd5, 0);
        return;
    }
}
