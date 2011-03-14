#ifndef DICTONARY_H
#define DICTONARY_H

#include <QString>
#include <QFile>
#include <QVector>

#include <ctime>
#include <cstdlib>

class Dictonary
{
	int dictonarySize;
    QFile file;
    QString lastWord;
    QVector<QString> dictonary;
    void refreshDictonary();
public:
    Dictonary(QString);
    QString nextWord();
    QString getLastWord() const;
};

#endif // DICTONARY_H
