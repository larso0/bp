#ifndef BP_H
#define BP_H

namespace bp
{
	void init();

	void poll_events();
	void wait_events();
	void wait_events(double timeout);
}

#endif
