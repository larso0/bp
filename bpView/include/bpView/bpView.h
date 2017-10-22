#ifndef BP_VIEW_BPVIEW_H
#define BP_VIEW_BPVIEW_H

#include <vector>
#include <string>
#include <bp/Event.h>

namespace bpView
{

extern const std::vector<std::string>& requiredInstanceExtensions;
extern bp::Event<const std::string&> errorEvent;

void init();

void pollEvents();
void waitEvents();

}

#endif