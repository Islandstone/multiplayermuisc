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

	void LowerVolume();
	void RaiseVolume();
private:
	QMap<int, QSound*> sounds;
	QSound* m_pCurrentSound;
	void LoadSounds();
	float m_flCurrentVolume;
};

#endif // SOUNDMANAGER_H
