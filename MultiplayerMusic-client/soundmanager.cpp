#include <QStringList>
#include <QDir>
#include <QFileInfoList>
#include <QDebug>
#include "soundmanager.h"

CSoundManager::CSoundManager()
{
	m_pCurrentSound = NULL;
	LoadSounds();
}

CSoundManager::~CSoundManager() {
	QList<QSound*> list = sounds.values();

	for (int i = 0; i < list.size(); i++) {
		QSound* sound = list.at(i);
		delete sound;
		list[i] = NULL;
	}
}

void CSoundManager::LoadSounds() {
	QStringList filter; filter << "sound*.*";
	QDir dir(QDir::currentPath());
	dir.setNameFilters(filter);
	dir.setFilter(QDir::Files);

	QRegExp exp("^sound(\\d+)\\..{3,4}$");
	QFileInfoList list = dir.entryInfoList();

	for (int i = 0; i < list.size(); i++) {
		QFileInfo file = list.at(i);
		if ( exp.exactMatch(file.fileName() )) {
			bool ok;
			int id = exp.cap(1).toInt(&ok);

			if (ok) {
				qDebug() << "Sound no." << exp.cap(1).toInt();

				QSound* sound = new QSound(file.fileName());
				sounds.insert(id, sound);
			} else {
				qDebug() << "Invalid number in file name:" << exp.cap(0);
				qDebug() << "Extracted from pattern:" << exp.cap(1);
			}
		}
	}
}

void CSoundManager::Stop() {
	if (m_pCurrentSound != NULL) {
		m_pCurrentSound->stop();
		m_pCurrentSound = NULL;
	}
}

void CSoundManager::PlaySound(int id) {
	if (sounds.contains(id)) {
		qDebug() << "Playing sound" << id;
		Stop();
		m_pCurrentSound = sounds.value(id);
		m_pCurrentSound->play();
	} else {
		qDebug() << "Invalid sound id:" << id;
	}
}
