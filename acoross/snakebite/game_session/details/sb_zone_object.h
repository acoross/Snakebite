#ifndef ACOROSS_SB_ZONE_OBJECT_H_
#define ACOROSS_SB_ZONE_OBJECT_H_

#include <acoross/snakebite/zone_system/zone_system.h>
#include "collider_base.h"

namespace acoross {
namespace snakebite {

using SbGeoZone = GeoZone<SbColliderBase>;
//using SbGeoZone = GeoZone;
using SbGeoZoneGrid = ZoneGrid<SbGeoZone>;
using SbZoneObject = ZoneObjectEx<SbColliderBase>;
using SbZoneObjectWP = std::weak_ptr<SbZoneObject>;
using ZoneObjectClone = ZoneObjectCloneEx<SbColliderBase>;
}
}

#endif //ACOROSS_SB_ZONE_OBJECT_H_