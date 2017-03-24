#include "pdg.h"

bool Statement::isControlReachableUpwards(Statement* source, Statement* dest)
{
	bool ret = false;
	bool edgetype = source->getControlParent().second;
	// search in siblings
	for (auto& sibling : source->getControlParent().first->getControlChildren()) {
		if (edgetype == sibling.second &&
			sibling.first == dest) return true;
	}
	// search in parent's siblings, recursively
	if (source->getControlParent().first != nullptr)
		return isControlReachableUpwards(source->getControlParent().first, dest);
	else
		return false;
}

