/*  KIARA - Middleware for efficient and QoS/Security-aware invocation of services and exchange of messages
 *
 *  Copyright (C) 2013  German Research Center for Artificial Intelligence (DFKI)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/*
 * version.c
 *
 *  Created on: 24.05.2013
 *      Author: Dmitri Rubinstein
 */
#include <KIARA/kiara.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    printf("KIARA Version: %i.%i.%i [Rev: %s] %i-bit\n",
            kiaraGetVersionMajor(), kiaraGetVersionMinor(), kiaraGetVersionPatch(),
            kiaraGetRepositoryRevision(), (int)sizeof(void*)*8);
    return 0;
}
