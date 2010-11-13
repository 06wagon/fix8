//-----------------------------------------------------------------------------------------
#if 0

FIX8 is released under the New BSD License.

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

-------------------------------------------------------------------------------------------
$Id: f8cutils.cpp 540 2010-11-05 21:25:33Z davidd $
$Date: 2010-11-06 08:25:33 +1100 (Sat, 06 Nov 2010) $
$URL: svn://catfarm.electro.mine.nu/usr/local/repos/fix8/compiler/f8cutils.cpp $

#endif
//-----------------------------------------------------------------------------------------
#include <config.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <iterator>
#include <memory>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <bitset>

#ifdef HAS_TR1_UNORDERED_MAP
#include <tr1/unordered_map>
#endif

#include <strings.h>
#include <regex.h>
#include <config.h>

#include <f8exception.hpp>
#include <f8types.hpp>
#include <f8utils.hpp>
#include <traits.hpp>
#include <field.hpp>
#include <message.hpp>
#include <thread.hpp>
#include <session.hpp>
#include <persist.hpp>

//-------------------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std;

//-------------------------------------------------------------------------------------------------
RegExp FIXReader::_hdr("8=([^\x01]+)\x01{1}9=([^\x01]+)\x01");

//-------------------------------------------------------------------------------------------------
int FIXReader::operator()()
{
   unsigned processed(0), dropped(0), invalid(0);

   for (;;)
   {
		try
		{
			f8String msg;

			if (read(msg))	// will block
			{
				if (_msg_queue.try_push (msg))
				{
					cerr << "FIXReader: message queue is full" << endl;
					++dropped;
				}
				else
					++processed;
			}
			else
				++invalid;
		}
		catch (exception& e)
		{
			cerr << e.what() << endl;
			++invalid;
		}
   }

	cerr << "FIXReader: " << processed << " messages processed, " << dropped << " dropped, "
		<< invalid << " invalid" << endl;

	return 0;
}

//-------------------------------------------------------------------------------------------------
int FIXReader::callback_processor()
{
   for (;;)
   {
      f8String msg;

      _msg_queue.pop (msg); // will block
      if (msg.empty() || !(_session.*_callback)(msg)) // return false to exit
			break;
   }

	return 0;
}

//-------------------------------------------------------------------------------------------------
void FIXReader::set_preamble_sz()
{
	_bg_sz = 2 + _session.get_ctx()._beginStr.size() + 1 + 3;
}

//-------------------------------------------------------------------------------------------------
bool FIXReader::read(f8String& to)	// read a complete FIX message
{
	char msg_buf[_max_msg_len] = {};
	int result(_sock->receiveBytes(msg_buf, _bg_sz));

	if (result == static_cast<int>(_bg_sz))
	{
		char bt;
		size_t offs(_bg_sz);
		do	// get the last chrs of bodylength and ^A
		{
			if (_sock->receiveBytes(&bt, 1) != 1)
				return false;
			if (!isdigit(bt) || bt != default_field_separator)
				throw IllegalMessage(msg_buf);
			msg_buf[offs++] = bt;
		}
		while (bt != default_field_separator && offs < _max_msg_len);
		to.assign(msg_buf, offs);

		RegMatch match;
		if (_hdr.SearchString(match, to, 3, 0) == 3)
		{
			f8String bgstr, len;
			_hdr.SubExpr(match, to, bgstr, 0, 1);
			_hdr.SubExpr(match, to, len, 0, 2);

			if (bgstr != _session.get_ctx()._beginStr)	// invalid begin string
				throw InvalidVersion(bgstr);

			const unsigned mlen(GetValue<unsigned>(len));
			if (mlen == 0 || mlen > _max_msg_len - _bg_sz - _chksum_sz) // invalid msglen
				throw InvalidBodyLength(mlen);

			// read the body
			if ((result = _sock->receiveBytes(msg_buf, mlen) == static_cast<int>(mlen)))
				return false;

			// read the checksum
			if ((result = _sock->receiveBytes(msg_buf + mlen, _chksum_sz) == static_cast<int>(_chksum_sz)))
				return false;

			to += string(msg_buf, mlen + _chksum_sz);
			return true;
		}

		throw IllegalMessage(to);
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
int FIXWriter::operator()()
{
	int result(0);

   for (;;)
   {
      f8String msg;

      _msg_queue.pop (msg); // will block
      if (msg.empty() || (result = _sock->sendBytes(msg.data(), msg.size()) < static_cast<int>(msg.size())))
			break;
   }

	return result;
}

//-------------------------------------------------------------------------------------------------
bool FIXWriter::write(const f8String& from)
{
	return !_msg_queue.try_push (from);
}

//-------------------------------------------------------------------------------------------------
void Connection::start()
{
	_writer.start();
	_reader->start();
}

