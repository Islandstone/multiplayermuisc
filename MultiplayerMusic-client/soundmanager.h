#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include <QMap>
#include <QSound>

class CSoundManager
{
public:
	CSoundManager();
	virtual ~CSoundManager();

	void PlaySound(int id);

	void Stop();

private:
	QMap<int, QSound*> sounds;
	QSound* m_pCurrentSound;
	void LoadSounds();
};

#endif // SOUNDMANAGER_H
