//
// latency.h - mesure la reactivite du coeur 0
//
// Le Wi-Fi, la page web et l'animation se partagent le coeur 0 a tour de role.
// Cette petite tache dort 2 ms en boucle et note de combien elle se reveille
// en retard : ce retard est le temps pendant lequel une autre tache a garde
// le processeur sans rendre la main (et donc sans laisser passer le Wi-Fi).
//
#ifndef _latency_h
#define _latency_h

#include <circle/sched/task.h>
#include <circle/types.h>

class CLatencyProbe : public CTask
{
public:
	CLatencyProbe (void);

	void Run (void);

	// plus long retard en ms : sur les 20 dernieres secondes, et sur les 5 dernieres minutes
	static unsigned GetRecentMs (void);
	static unsigned GetMaxMs (void);

private:
	static const unsigned SleepMicros = 2000;
	static const unsigned WindowSeconds = 10;
	static const unsigned Windows = 30;		// 30 x 10 s = 5 minutes

	static unsigned s_nWindowMax[Windows];
	static unsigned s_nCurrent;
};

#endif
