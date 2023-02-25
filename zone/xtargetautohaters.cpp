#include "xtargetautohaters.h"
#include "mob.h"
#include "client.h"
#include "raids.h"
#include "groups.h"

#include <algorithm>

void XTargetAutoHaters::increment_count(Mob *in) {}

void XTargetAutoHaters::decrement_count(Mob *in) {}

void XTargetAutoHaters::merge(XTargetAutoHaters &other) {}

// demerge this from other. other belongs to group/raid you just left
void XTargetAutoHaters::demerge(XTargetAutoHaters &other) {}

bool XTargetAutoHaters::contains_mob(int spawn_id) { return false; }
