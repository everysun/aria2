/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2006 Tatsuhiro Tsujikawa
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */
/* copyright --> */
#include "MessageDigestHelper.h"
#include "messageDigest.h"
#include "DlAbortEx.h"
#include "message.h"
#include "DefaultDiskWriter.h"
#include "Util.h"
#include <errno.h>

string MessageDigestHelper::digest(const string& algo, DiskWriterHandle diskWriter, int64_t offset, int64_t length)
{
  MessageDigestContext ctx;
  ctx.trySetAlgo(algo);
  ctx.digestInit();

  int32_t BUFSIZE = 4096;
  char BUF[BUFSIZE];
  int64_t iteration = length/BUFSIZE;
  int32_t tail = length%BUFSIZE;
  for(int64_t i = 0; i < iteration; ++i) {
    int32_t readLength = diskWriter->readData(BUF, BUFSIZE, offset);
    if(readLength != BUFSIZE) {
      throw new DlAbortEx(EX_FILE_READ, "n/a", strerror(errno));
    }
    ctx.digestUpdate(BUF, readLength);
    offset += readLength;
  }
  if(tail) {
    int32_t readLength = diskWriter->readData(BUF, tail, offset);
    if(readLength != tail) {
      throw new DlAbortEx(EX_FILE_READ, "n/a", strerror(errno));
    }
    ctx.digestUpdate(BUF, readLength);
  }
  string rawMD = ctx.digestFinal();
  return Util::toHex((const unsigned char*)rawMD.c_str(), rawMD.size());
}

string MessageDigestHelper::digest(const string& algo, const string& filename)
{
  DiskWriterHandle writer = new DefaultDiskWriter();
  writer->openExistingFile(filename);
  return digest(algo, writer);
}

string MessageDigestHelper::digest(const string& algo, const void* data, int32_t length)
{
  MessageDigestContext ctx;
  ctx.trySetAlgo(algo);
  ctx.digestInit();
  ctx.digestUpdate(data, length);
  string rawMD = ctx.digestFinal();
  return Util::toHex((const unsigned char*)rawMD.c_str(), rawMD.size());
}

void MessageDigestHelper::digest(unsigned char* md, int32_t mdLength,
				 const string& algo, const void* data, int32_t length)
{
  if(mdLength < MessageDigestContext::digestLength(algo)) {
    throw new DlAbortEx("Insufficient space for storing message digest: %d required, but only %d is allocated", MessageDigestContext::digestLength(algo), mdLength);
  }
  MessageDigestContext ctx;
  ctx.trySetAlgo(algo);
  ctx.digestInit();
  ctx.digestUpdate(data, length);
  ctx.digestFinal(md);
}

