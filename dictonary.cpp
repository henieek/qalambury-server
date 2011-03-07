#include "dictonary.h"

Dictonary::Dictonary(QString filename)
{
	this->file.setFileName(filename);
	srand(time(NULL));
    this->refreshDictonary();
}

void Dictonary::refreshDictonary() {
	this->dictonarySize = 0;
    this->dictonary.clear();
    this->file.open(QIODevice::ReadOnly);
    while(!file.atEnd()) {
        dictonary.push_back(QString(file.readLine()).replace(QString("\n"),QString("")));
		dictonarySize++;
    }
	this->file.close();
}

QString Dictonary::nextWord() {
	QString word("");
	if(this->dictonarySize) {
		int random = rand()%(this->dictonarySize);
		word = dictonary.at(random);
		this->dictonary.remove(random);
		this->dictonarySize--;
		if(!dictonarySize) {
			this->refreshDictonary();
		}
		this->lastWord = word;
	}
	return word;
}
