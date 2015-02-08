#include <QCoreApplication>
#include <QDebug>
#include "pasafer.h"


#ifdef WIN
#include <windows.h>

void echo( bool on = true )
{
    DWORD  mode;
    HANDLE hConIn = GetStdHandle( STD_INPUT_HANDLE );
    GetConsoleMode( hConIn, &mode );
    mode = on
            ? (mode |   ENABLE_ECHO_INPUT )
            : (mode & ~(ENABLE_ECHO_INPUT));
    SetConsoleMode( hConIn, mode );
}
#else
#include <termios.h>
#include <unistd.h>

void echo( bool on = true )
{
    struct termios settings;
    tcgetattr( STDIN_FILENO, &settings );
    settings.c_lflag = on
            ? (settings.c_lflag |   ECHO )
            : (settings.c_lflag & ~(ECHO));
    tcsetattr( STDIN_FILENO, TCSANOW, &settings );
}
#endif

int usage(QString cmd)
{
    QTextStream out(stdout);

    out << "Usage: "<< cmd <<" [-g --file file_name --size file_size] [-r --file file_name --key key]\n";
    out << "  -g      generate sands file by with size(--size)\n";
    out << "  -r      read a password from sands file by providing a key, and Main Password\n";
    out << "  --size  size of sands file to generate\n";
    out << "  --key   key of a password to get from sands file\n";
    return 0;
}


QString get_main_password()
{
    QTextStream in(stdin);
    QTextStream out(stdout);

    out << "Main Password:";
    out.flush();
    echo( false );
    QString password = in.readLine();
    out << "\n";
    echo( true );
    return password;
}

int main(int argc, char *argv[])
{
    QString file_name="";
    qint32  file_size = 0;
    QString key="";
    QString password;
    Pasafer pasafer(QCryptographicHash::Sha512);

    bool is_gen_sands_file = false;
    bool is_get_password = false;


    for (int i = 1; i < argc; i++) {
        QString arg = argv[i];
        if (arg == "-g") {
            is_gen_sands_file = true;
            is_get_password = false;
        } else if (arg == "-r"){
            is_get_password = true;
            is_gen_sands_file = false;
        } else {
            if (i+1 >= argc) {
                usage(argv[0]);
                return 0;
            }
            if (arg == "--file") {
                file_name = argv[++i];
            } else if (arg == "--size") {
                file_size = QString(argv[++i]).toInt();
            } else if (arg == "--key") {
                key = argv[++i];
            }
        }
    }

    if(is_gen_sands_file) {
        if(file_name=="" || file_size == 0) {
            usage(argv[0]);
            return 0;
        }
        pasafer.set_sands_file(file_name);
        return pasafer.gen_sands_file(file_size);
    }
    if(is_get_password) {
        if (file_name=="" || key == "") {
            usage(argv[0]);
            return 0;
        }
        password = get_main_password();
        pasafer.set_sands_file(file_name);
        pasafer.set_main_password(password);
        pasafer.set_key(key);
        password = pasafer.get_password(8);
        QTextStream out(stdout);
        out << password <<"\n";
        return 0;
    }

    usage(argv[0]);
    return 0;
 }
