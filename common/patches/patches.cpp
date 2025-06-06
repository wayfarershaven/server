/*	EQEMu: Everquest Server Emulator

	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "../global_define.h"
#include "patches.h"

#include "titanium.h"
#include "uf.h"
#include "sof.h"
#include "sod.h"
#include "rof.h"
#include "rof2.h"


void RegisterAllPatches(EQStreamIdentifier &into)
{
//	Titanium::Register(into);
//	SoF::Register(into);
//	SoD::Register(into);
//	UF::Register(into);
//	RoF::Register(into);
	RoF2::Register(into);
}

void ReloadAllPatches()
{
//	Titanium::Reload();
//	SoF::Reload();
//	SoD::Reload();
//	UF::Reload();
//	RoF::Reload();
	RoF2::Reload();
}
