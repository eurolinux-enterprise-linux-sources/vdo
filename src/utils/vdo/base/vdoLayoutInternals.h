/*
 * Copyright (c) 2018 Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA. 
 *
 * $Id: //eng/vdo-releases/magnesium-rhel7.6/src/c++/vdo/base/vdoLayoutInternals.h#1 $
 */

#ifndef VDO_LAYOUT_INTERNALS_H
#define VDO_LAYOUT_INTERNALS_H

#include "fixedLayout.h"
#include "types.h"

struct vdoLayout {
  // The current layout of the VDO
  FixedLayout         *layout;
  // The next layout of the VDO
  FixedLayout         *nextLayout;
  // The previous layout of the VDO
  FixedLayout         *previousLayout;
  // The first block in the layouts
  PhysicalBlockNumber  startingOffset;
};

#endif // VDO_LAYOUT_INTERNALS_H
