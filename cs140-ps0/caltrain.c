#include "pintos_thread.h"

struct station {
	int waiting;
	int boarding;
	int seats;
	struct lock l;
	struct condition seats_avail;
	struct condition boarding_done;
};

void
station_init(struct station *s)
{
	s->waiting = 0;
	s->boarding = 0;
	s->seats = 0;
	lock_init((struct lock*)&s->l);
	cond_init((struct condition*)&s->seats_avail);
	cond_init((struct condition*)&s->boarding_done);
}

void
station_load_train(struct station *s, int count)
{
	lock_acquire((struct lock*)&s->l);
	s->seats = count;
	cond_broadcast((struct condition*)&s->seats_avail, (struct lock*)&s->l);
	while(s->boarding || (s->waiting && s->seats))
		cond_wait((struct condition*)&s->boarding_done, (struct lock*)&s->l);
	s->seats = 0;
	lock_release((struct lock*)&s->l);
}

void
station_wait_for_train(struct station *s)
{
	lock_acquire((struct lock*)&s->l);
	s->waiting++;
	while(!s->seats)
		cond_wait((struct condition*)&s->seats_avail, (struct lock*)&s->l);
	s->waiting--;
	s->seats--;
	s->boarding++;
	lock_release((struct lock*)&s->l);
}

void
station_on_board(struct station *s)
{
	lock_acquire((struct lock*)&s->l);
	s->boarding--;
	if(!s->boarding && (!s->waiting || !s->seats))
		cond_signal((struct condition*)&s->boarding_done, (struct lock*)&s->l);
	lock_release((struct lock*)&s->l);
}
