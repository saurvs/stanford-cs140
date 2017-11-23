#include "pintos_thread.h"

// Forward declaration. This function is implemented in reaction-runner.c,
// but you needn't care what it does. Just be sure it's called when
// appropriate within reaction_o()/reaction_h().
void make_water();

struct reaction {
	int h_idle;
	int o_idle;
	int h_needed;
	int o_needed;
	struct lock l;
	struct condition h_reactable;
	struct condition o_reactable;
};

void
reaction_init(struct reaction *r)
{
	r->h_idle = 0;
	r->o_idle = 0;
	r->h_needed = 0;
	r->o_needed = 0;
	lock_init((struct lock*)&r->l);
	cond_init((struct condition*)&r->h_reactable);
	cond_init((struct condition*)&r->o_reactable);
}

void
reaction_h(struct reaction *r)
{
	lock_acquire((struct lock*)&r->l);
	r->h_idle++;
	while((r->h_idle < 2 || r->o_idle < 1) && !r->h_needed)
		cond_wait((struct condition*)&r->h_reactable, (struct lock*)&r->l);
	r->h_idle--;
	if(r->h_needed){
		r->h_needed--;
	} else {
		make_water();
		r->h_needed++;
		r->o_needed++;
		cond_broadcast((struct condition*)&r->h_reactable, (struct lock*)&r->l);
		cond_broadcast((struct condition*)&r->o_reactable, (struct lock*)&r->l);
	}
	lock_release((struct lock*)&r->l);
}

void
reaction_o(struct reaction *r)
{
	lock_acquire((struct lock*)&r->l);
	r->o_idle++;
	while(r->h_idle < 2 && !r->o_needed)
		cond_wait((struct condition*)&r->o_reactable, (struct lock*)&r->l);
	r->o_idle--;
	if(r->o_needed){
		r->o_needed--;
	} else {
		make_water();
		r->h_needed += 2;
		cond_broadcast((struct condition*)&r->h_reactable, (struct lock*)&r->l);
	}
	lock_release((struct lock*)&r->l);
}
