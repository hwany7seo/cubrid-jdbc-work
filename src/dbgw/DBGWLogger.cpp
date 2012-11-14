/*
 * Copyright (C) 2008 Search Solution Corporation. All rights reserved by Search Solution.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 */
#include <sys/stat.h>
#include <fstream>
#include <stdarg.h>
#include "DBGWCommon.h"
#include "DBGWError.h"
#include "DBGWPorting.h"
#include "DBGWLogger.h"

namespace dbgw
{

  static const char *DBGW_LOG_PATH = "log/cci_dbgw.log";
  static const int LOG_BUFFER_SIZE = 1024 * 20;
  static system::_MutexSharedPtr g_pLogMutex = system::_MutexFactory::create();

  Logger _DBGWLogger::m_logger = NULL;
  string _DBGWLogger::m_logPath = DBGW_LOG_PATH;
  CCI_LOG_LEVEL _DBGWLogger::m_logLevel = CCI_LOG_LEVEL_ERROR;

  _DBGWLogger::_DBGWLogger()
  {
  }

  _DBGWLogger::_DBGWLogger(const string &sqlName) :
    m_sqlName(sqlName)
  {
  }

  _DBGWLogger::_DBGWLogger(const string &groupName, const string &sqlName) :
    m_groupName(groupName), m_sqlName(sqlName)
  {
  }

  _DBGWLogger::~_DBGWLogger()
  {
  }

  void _DBGWLogger::setGroupName(const string &groupName)
  {
    m_groupName = groupName;
  }

  void _DBGWLogger::setSqlName(const string &sqlName)
  {
    m_sqlName = sqlName;
  }

  const string _DBGWLogger::getLogMessage(const char *szMsg) const
  {
    string message;
    if (m_groupName == "")
      {
        message += m_sqlName;
        message += " : ";
        message += szMsg;
      }
    else if (m_sqlName == "")
      {
        message += m_groupName;
        message += " : ";
        message += szMsg;
      }
    else
      {
        message += m_groupName;
        message += ".";
        message += m_sqlName;
        message += " : ";
        message += szMsg;
      }
    return message;
  }

  void _DBGWLogger::initialize()
  {
    initialize(m_logLevel, DBGW_LOG_PATH);
  }

  void _DBGWLogger::initialize(CCI_LOG_LEVEL level, const char *szLogPath)
  {
    setLogPath(szLogPath);
    setLogLevel(level);
  }

  void _DBGWLogger::setLogPath(const char *szLogPath)
  {
    g_pLogMutex->lock();
    if (szLogPath != NULL)
      {
        if (m_logger != NULL)
          {
            cci_log_remove(m_logPath.c_str());
          }

        m_logPath = szLogPath;
        m_logger = cci_log_get(m_logPath.c_str());
      }
    g_pLogMutex->unlock();
  }

  void _DBGWLogger::setLogLevel(CCI_LOG_LEVEL level)
  {
    if (m_logger != NULL)
      {
        cci_log_set_level(m_logger, level);
      }
  }

  void _DBGWLogger::setForceFlush(bool bForceFlush)
  {
    if (m_logger != NULL)
      {
        cci_log_set_force_flush(m_logger, bForceFlush);
      }
  }

  void _DBGWLogger::finalize()
  {
    g_pLogMutex->lock();
    if (m_logger != NULL)
      {
        cci_log_remove(m_logPath.c_str());
        m_logger = NULL;
      }
    g_pLogMutex->unlock();
  }

  void _DBGWLogger::writeLogF(const char *szFile, int nLine, CCI_LOG_LEVEL level,
      const char *szFormat, ...)
  {
    if (m_logger != NULL)
      {
        const char *szBase = strrchr(szFile, '/');
        szBase = szBase ? (szBase + 1) : szFile;
        char fileLineBuf[32];
        snprintf(fileLineBuf, 32, "%s:%d", szBase, nLine);

        char prefixBuf[LOG_BUFFER_SIZE];
        snprintf(prefixBuf, LOG_BUFFER_SIZE, " %-31s %s", fileLineBuf, szFormat);

        char szLogFormat[LOG_BUFFER_SIZE];
        va_list vl;
        va_start(vl, szFormat);
        vsnprintf(szLogFormat, LOG_BUFFER_SIZE, prefixBuf, vl);
        va_end(vl);

        cci_log_writef(level, m_logger, szLogFormat);
      }
  }

  void _DBGWLogger::writeLog(const char *szFile, int nLine, CCI_LOG_LEVEL level,
      const char *szLog)
  {
    if (m_logger != NULL)
      {
        const char *szBase = strrchr(szFile, '/');
        szBase = szBase ? (szBase + 1) : szFile;
        char fileLineBuf[32];
        snprintf(fileLineBuf, 32, "%s:%d", szBase, nLine);

        char logBuf[LOG_BUFFER_SIZE];
        snprintf(logBuf, LOG_BUFFER_SIZE, " %-31s %s", fileLineBuf, szLog);

        cci_log_write(level, m_logger, logBuf);
      }
  }

  bool _DBGWLogger::isWritable(CCI_LOG_LEVEL level)
  {
    return cci_log_is_writable(m_logger, level);
  }

  const char *_DBGWLogger::getLogPath()
  {
    return m_logPath.c_str();
  }

  CCI_LOG_LEVEL _DBGWLogger::getLogLevel()
  {
    return m_logLevel;
  }

  _DBGWLogDecorator::_DBGWLogDecorator(const char *szHeader) :
    m_header(szHeader), m_iLogCount(0)
  {
    clear();
  }

  _DBGWLogDecorator::~_DBGWLogDecorator()
  {
  }

  void _DBGWLogDecorator::clear()
  {
    m_iLogCount = 0;
    m_buffer.seekp(ios_base::beg);

    m_buffer << m_header;
    m_buffer << " [";
  }

  void _DBGWLogDecorator::addLog(const string &log)
  {
    if (m_iLogCount++ > 0)
      {
        m_buffer << ", ";
      }
    m_buffer << log;
  }

  void _DBGWLogDecorator::addLogDesc(const string &desc)
  {
    m_buffer << "(" << desc << ")";
  }

  string _DBGWLogDecorator::getLog()
  {
    m_buffer << "]";
    return m_buffer.str();
  }

}
