#include <QCoreApplication>
#include <QDebug>
#include "pasafer.h"

#define WIN

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

    out << "Usage: "<< cmd <<" [-g [--file file_name [--size file_size]]] [-k key [--file file_name]]\n";
    out << "  -g      generate pasafer sands file\n";
    out << "  -k      key of password to get\n";
    out << "  --size  size of sands file to generate\n";
    out << "  --file  pasafer sands file\n";
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
    QString file_name="pasafer.sands";
    qint32  file_size = 1;
    QString key="";
    QString password;
    Pasafer pasafer(QCryptographicHash::Sha512);

    bool is_gen_sands_file = false;
    bool is_get_password = true;


    for (int i = 1; i < argc; i++) {
        QString arg = argv[i];
        if (arg == "-g") {
            is_gen_sands_file = true;
            is_get_password = false;
        } else {
            if (i+1 >= argc) {
                usage(argv[0]);
                return 0;
            }
            if (arg == "--file") {
                file_name = argv[++i];
            } else if (arg == "--size") {
                file_size = QString(argv[++i]).toInt();
            } else if (arg == "-k") {
                key = argv[++i];
            }
        }
    }

    if(is_gen_sands_file) {
        pasafer.set_sands_file(file_name);
        return pasafer.gen_sands_file(file_size);
    } else if(is_get_password) {
        if (key == "") {
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
    } else {
        usage(argv[0]);
        return 0;
    }
 }
