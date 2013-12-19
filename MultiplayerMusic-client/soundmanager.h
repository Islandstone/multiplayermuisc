#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include <QMap>
//#include <QSound>

#include "irrKlang.h"

class CSoundManager
{
public:
	CSoundManager();
	virtual ~CSoundManager();

	void PlaySound(int id);

	void Stop();

	void LowerVolume();
    void RaiseVolume();
    void SetVolume(float vol);
private:
    irrklang::ISoundEngine *m_pEngine;
    /*
	QMap<int, QSound*> sounds;
    QSound* m_pCurrentSound;
    */
    QMap<int, irrklang::ISoundSource*> sounds;
    irrklang::ISoundSource* m_pCurrentSound;
	void LoadSounds();
	float m_flCurrentVolume;
};

#endif // SOUNDMANAGER_H
