//-------------------------------------------------------------------------------------------------
#if 0

Fix8 is released under the New BSD License.

Copyright (c) 2010, David L. Dight <fix@fix8.org>
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of
	 	conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list
	 	of conditions and the following disclaimer in the documentation and/or other
		materials provided with the distribution.
    * Neither the name of the author nor the names of its contributors may be used to
	 	endorse or promote products derived from this software without specific prior
		written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
OR  IMPLIED  WARRANTIES,  INCLUDING,  BUT  NOT  LIMITED  TO ,  THE  IMPLIED  WARRANTIES  OF
MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE ARE  DISCLAIMED. IN  NO EVENT  SHALL
THE  COPYRIGHT  OWNER OR  CONTRIBUTORS BE  LIABLE  FOR  ANY DIRECT,  INDIRECT,  INCIDENTAL,
SPECIAL,  EXEMPLARY, OR CONSEQUENTIAL  DAMAGES (INCLUDING,  BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF USE, DATA,  OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED  AND ON ANY THEORY OF LIABILITY, WHETHER  IN CONTRACT, STRICT  LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

---------------------------------------------------------------------------------------------------
$Id: logger.hpp 549 2010-11-14 11:09:12Z davidd $
$Date: 2010-11-14 22:09:12 +1100 (Sun, 14 Nov 2010) $
$URL: svn://catfarm.electro.mine.nu/usr/local/repos/fix8/include/logger.hpp $

#endif
//-------------------------------------------------------------------------------------------------
#ifndef _FIX8_INCLUDES_HPP_
#define _FIX8_INCLUDES_HPP_

#include <f8config.h>

#ifdef HAS_TR1_UNORDERED_MAP
#include <tr1/unordered_map>
#endif

#include <f8exception.hpp>
#include <memory.hpp>
#include <f8allocator.hpp>
#include <f8types.hpp>
#include <f8utils.hpp>
#include <xml.hpp>
#include <thread.hpp>
#include <gzstream.hpp>
#include <logger.hpp>
#include <traits.hpp>
#include <timer.hpp>
#include <field.hpp>
#include <message.hpp>
#include <session.hpp>
#include <connection.hpp>
#include <configuration.hpp>
#include <persist.hpp>

#endif // _FIX8_INCLUDES_HPP_
