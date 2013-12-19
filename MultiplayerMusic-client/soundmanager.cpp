#include <QStringList>
#include <QDir>
#include <QFileInfoList>
#include <QDebug>
//#include <QAudioDeviceInfo>
#include "soundmanager.h"

#include "console.h"

CSoundManager::CSoundManager()
{
    m_pEngine = irrklang::createIrrKlangDevice();

    if (m_pEngine == NULL) {
        qDebug() << "Failed to load sound engine!";
        exit(0);
    }

	m_flCurrentVolume = 1.0f;
	LoadSounds();
}

CSoundManager::~CSoundManager() {
    m_pEngine->drop();
}

void CSoundManager::LoadSounds() {
	QStringList filter; filter << "sound*.*";
    Msg("Current directory: " + QDir::currentPath());
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
                irrklang::ISoundSource* sound = m_pEngine->addSoundSourceFromFile(file.fileName().toStdString().c_str());
				sounds.insert(id, sound);
                Msg("Loaded sound: " + file.fileName());
            }
        }
    }
}

void CSoundManager::SetVolume(float vol) {
    m_pEngine->setSoundVolume(vol);
}

void CSoundManager::LowerVolume() {
	m_flCurrentVolume = qMax(m_flCurrentVolume - 0.1f, 0.0f);
}

void CSoundManager::RaiseVolume() {
	m_flCurrentVolume = qMin(m_flCurrentVolume + 0.1f, 1.0f);
}

void CSoundManager::Stop() {
    m_pEngine->stopAllSounds();
}

void CSoundManager::PlaySound(int id) {
	if (sounds.contains(id)) {
        Msg(QString("Playing sound ") + QString::number(id));
		Stop();

        m_pEngine->play2D(sounds.value(id));
	} else {
        Msg("Could not play sound, no file loaded for id " + QString::number(id));
	}
}
